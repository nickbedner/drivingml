#!/usr/bin/env python3

import socket
import struct
import torch
import torch.nn as nn
import torch.optim as optim
import torch.distributions as D
import numpy as np
import os

HOST = "127.0.0.1"
PORT = 5000

# Set to False to resume training
EVAL_MODE = False
# If True: overwrite single checkpoint
SAVE_ONLY_LATEST = True
# Export .bin weights for C engine
EXPORT_WEIGHTS = True
EXPORT_PATH = "build/checkpoints"

# Create checkpoint directory to store trained models
os.makedirs(EXPORT_PATH, exist_ok=True)

# Checks how normal an action is u
def gaussian_log_prob(u, mean, log_std):
    # How random it should be
    std = log_std.exp()
    # Bell curve with mean as the usual action and std as to how much randomness is allowed
    base = D.Normal(mean, std)
    # Checks how close u is to the usual action, we sum because we can have multiple actions at the same time
    return base.log_prob(u).sum(-1)

# Squashes range to -1 to 1, needs to be in log form and because we have multiple actions we add them together too
def tanh_correction(a, eps=1e-6):
    # a is tanh(u) in [-1,1]
    return torch.log(1 - a * a + eps).sum(-1)

# Writes the weights to a bin file which is loaded in sum and actions are generated via custom forward pass
def export_weights_bin(model, log_std, path):
    """
    Export model weights to a simple float32 binary format
    readable from pure C with fread().
    """
    sd = model.state_dict()

    def write_tensor(f, t):
        t = t.detach().cpu().contiguous().view(-1).to(torch.float32)
        f.write(struct.pack("<%sf" % t.numel(), *t.numpy().tolist()))

    with open(path, "wb") as f:
        # Magic number so we know version, aka Actor-Critic v.1
        f.write(b"ACv1")
        # Dimensions so C knows what are model is
        f.write(struct.pack("<5i", 5, 128, 128, 2, 1))

        # Shared layers
        write_tensor(f, sd["shared.0.weight"])
        write_tensor(f, sd["shared.0.bias"])
        write_tensor(f, sd["shared.2.weight"])
        write_tensor(f, sd["shared.2.bias"])

        # Actor
        write_tensor(f, sd["actor.weight"])
        write_tensor(f, sd["actor.bias"])

        # Critic
        write_tensor(f, sd["critic.weight"])
        write_tensor(f, sd["critic.bias"])

        # Log_std
        write_tensor(f, log_std)

    print(f"[EXPORT] Weights exported to {path}")

# We define small neural network for learning, 4 sequential stacked layers
# 128 neurons in our Linear layers
# Actor-Critic model
# Actor decides what action to take and outputs an action
# Critic judges how good the situation is and gives a score
# Critic it like the coach, and actor is like the player
# Linear is a linear function
# ReLU is Rectified Linear Unit which helps nueral network learn complex pattersn
# 4 shared layers linear -> relu -> linear -> relu
# Neurons = 5(input) + 128(first hidden) + 128(second hidden) + 2(actor output) + 1(critic output) = 264
# Parameters = (5×128+128)(first hidden)+(128×128+128)(second hidden)+(128×2+2)(actor)+(128×1+1)(critic) = 17,667
class ActorCritic(nn.Module):
    def __init__(self):
        super().__init__()
        self.shared = nn.Sequential(
            nn.Linear(5, 128),
            nn.ReLU(),
            nn.Linear(128, 128),
            nn.ReLU()
        )
        self.actor = nn.Linear(128, 2)
        self.critic = nn.Linear(128, 1)

    def forward(self, x):
        h = self.shared(x)
        return self.actor(h), self.critic(h).squeeze(-1)
model = ActorCritic()

# Hyperparameters 
# How random the AI is encouraged to be, higher for more exploration
entropy_coef = 0.005
# Changes how big of an importance is put on critic, increase for critic, decrease for actor
value_loss_coef = 0.5
# Max size of an update
grad_clip_norm = 0.5
# (Learning rate)Size of each training step, too big becaomes unstable and too small learns slow
lr = 1e-4
# Discount factor, higher places bigger importance on future rewards vs immediate, 1 is very high, 0 is very low
gamma = 0.99
# How many state steps to experience before updating the nueral network
rollout_length = 512
# Counter to track how many updates we've done
update_count = 0
# Coutner to track episode num
episode = 0
# Buffers
# Past reward values
rewards = []
# Past critic values
values = []
# Past nputs from simulation
states = []
# Past log probability of each action when it was taken, aka how confident the AI was about action
old_log_probs = []
# Past actual actions taken
us = []

# Sets how much randonmess for our two actions
log_std = nn.Parameter(torch.ones(2) * -1.5)

# Adam optimizer which updates the model's numbers during training, makes the AI learn
# Adam is a popular learning algorithm, similar to gradient descent but adaptive step size, stores past in memory, smoother updates
optimizer = optim.Adam(list(model.parameters()) + [log_std], lr=lr)

# Resume training from the latest checkpoint if it exists
checkpoint_path = EXPORT_PATH + "/latest_model.pt"
if os.path.exists(checkpoint_path):
    checkpoint = torch.load(checkpoint_path)
    model.load_state_dict(checkpoint["model_state_dict"])
    log_std.data.copy_(checkpoint.get("log_std", torch.zeros_like(log_std)))
    optimizer.load_state_dict(checkpoint["optimizer_state_dict"])
    episode = checkpoint["episode"]
    print(f"Resumed training from episode {episode}")
else:
    print("Starting fresh training.")

# Wait for connection from C app
server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.bind((HOST, PORT))
server.listen(1)
print("Waiting for connection...")
conn, addr = server.accept()
print("Connected:", addr)
# Recieve packet from C app containing state
def recv_exact(sock, size):
    data = b''
    while len(data) < size:
        packet = sock.recv(size - len(data))
        if not packet:
            return None
        data += packet
    return data

while True:
    # Make sure we actually recieved something and it's correct packet size
    data = recv_exact(conn, 24)
    if data is None:
        break

    # Break apart packet into variable
    dx, dy, speed, azimuth,reward, done = struct.unpack("<5fi", data)
    # Normalize speed to help with training
    speed = np.tanh(speed / 50.0)
    # Simplifies and normalizes azimuth into sin and cos angles for model
    # For example in azimuth 0 and 360 are the same but can look different for the model
    azimuth = np.sin(azimuth), np.cos(azimuth)

    # Creates and input tensor for the model
    state = torch.tensor([dx, dy, speed, azimuth[0], azimuth[1]], dtype=torch.float32)

    # Mean is the actor's output(the action)
    # Value it the critic's estimate how good the state is, is it expecting good future rewards
    mean, value = model(state)
    # Makes log_std the same shape as mean, matching sizes by copying same randomness values. Make the randomness match the action size
    log_std_exp = log_std.expand_as(mean)
    # Turn log randomness into real randomness
    std = log_std_exp.exp()

    # If evaluating(testing) the model then we just use the mean direction, otherwise add random for training
    if EVAL_MODE:
        # Deterministic action
        u = mean
    else:
        # Random action, randn means random numbers from a normal distribution
        u = mean + std * torch.randn_like(mean)

    # Squeezes action into range of -1 to 1
    a = torch.tanh(u)

    # Log prob uses the true tanh transform, aka fancy math to calculate correct probablity of final action
    log_prob = gaussian_log_prob(u, mean, log_std_exp) - tanh_correction(a)

    # Clamp for env safety and numeric reasons
    # Eps value to remove edge cases and prevent breaking
    eps = 1e-6
    a_env = a.clamp(-1 + eps, 1 - eps)

    # If training then store past into buffers
    if not EVAL_MODE:
        states.append(state.detach())
        us.append(u.detach())
        old_log_probs.append(log_prob.detach())
        values.append(value.detach())
        rewards.append(reward)

    # Send clipped action to game
    action_np = a_env.detach().cpu().numpy().astype("float32")
    conn.send(struct.pack("<2f", *action_np))

    # If episode finished then we train and save training
    # Trigger training on rollout or episode end
    if not EVAL_MODE and (done == 1 or len(rewards) >= rollout_length):
        if done == 1:
            episode += 1

        # Bootstrap value if not done, uses critic to estimate future reward
        if done == 0:
            with torch.no_grad():
                _, next_value = model(state)
            bootstrap_value = next_value.item()
        else:
            bootstrap_value = 0.0
        
        # Convert rollout to tensors for model to train on
        states_tensor = torch.stack(states)
        old_log_probs_tensor = torch.stack(old_log_probs)
        values_tensor = torch.stack(values).squeeze(-1)
        us_tensor = torch.stack(us)

        # Check if we even have enough date to train on
        if len(rewards) < 2:
            states.clear()
            old_log_probs.clear()
            values.clear()
            rewards.clear()
            us.clear()
            continue

        # Compute returns once, which are the total future rewards from each step
        returns = []
        G = bootstrap_value
        for r in reversed(rewards):
            G = r + gamma * G
            returns.insert(0, G)
        returns = torch.tensor(returns, dtype=torch.float32, device=values_tensor.device)

        # Compute advantages, aka did we do better or worse than critic
        advantages = (returns - values_tensor).detach()

        # Simples advantagers so easier for model to learn from
        if advantages.numel() > 1:
            adv_std = advantages.std(unbiased=False).clamp_min(1e-8)
            advantages = (advantages - advantages.mean()) / adv_std
        else:
            # If only 1 step in rollout, skip normalization
            advantages = advantages * 0.0
        
        # PPO stands for Proximal Policy Optimization, reinforcement learning algorithm used to train policies(actor)
        # PPO is used because it makes learning stable and safe
        # It prevents the policy from changing too much at once, which can break training
        # PPO hyperparameters
        # Limits how much the policy is allowed to change using clipping
        clip_eps = 0.2
        # Repeated 4 times over the same collected data
        ppo_epochs = 4
        # Checks how much the new policy changed from the old one and limits the change to keep learning stable
        # Then it updates the actor and critic
        for _ in range(ppo_epochs):
            # TODO: Doc this
            mean, new_values = model(states_tensor)

            log_std_exp = log_std.expand_as(mean)
            std = log_std_exp.exp()

            u_fixed = us_tensor  # fixed rollout sample
            a_fixed = torch.tanh(u_fixed)

            new_log_probs = gaussian_log_prob(u_fixed, mean, log_std_exp) - tanh_correction(a_fixed)

            ratio = torch.exp(new_log_probs - old_log_probs_tensor)
            ratio = torch.clamp(ratio, 0.0, 5.0)
            clipped_ratio = torch.clamp(ratio, 1.0 - clip_eps, 1.0 + clip_eps)

            policy_loss = -torch.min(ratio * advantages, clipped_ratio * advantages).mean()
            value_loss = nn.SmoothL1Loss()(new_values.squeeze(-1), returns)

            base_entropy = D.Normal(mean, std).entropy().sum(-1).mean()

            loss = policy_loss + value_loss_coef * value_loss - entropy_coef * base_entropy

            optimizer.zero_grad()
            loss.backward()
            torch.nn.utils.clip_grad_norm_(list(model.parameters()) + [log_std], grad_clip_norm)
            optimizer.step()

        update_count += 1

        # This keeps log_std within a safe range, stops the randomness from getting too big or too small
        with torch.no_grad():
            log_std.clamp_(min=-2.0, max=0.5)

        # Debug (only print at episode end)
        if done == 1:
            print(
                f"Episode {episode} | "
                f"returns_abs_mean: {returns.abs().mean().item():.4f} | "
                f"value_loss: {value_loss.item():.4f}"
            )

            print(
                f"Episode {episode} trained. "
                f"Loss: {loss.item():.4f}, "
                f"policy: {policy_loss.item():.4f}, "
                f"value: {value_loss.item():.4f}, "
                f"entropy: {base_entropy.item():.4f}, "
                f"mean_std: {log_std.exp().mean().item():.4f}"
            )

            checkpoint = {
                "model_state_dict": model.state_dict(),
                "log_std": log_std.detach().cpu(),
                "optimizer_state_dict": optimizer.state_dict(),
                "episode": episode
            }

            # Always update latest checkpoint
            torch.save(checkpoint, EXPORT_PATH + "/latest_model.pt")

            # Optionally also save per-episode snapshots
            if not SAVE_ONLY_LATEST:
                torch.save(checkpoint, EXPORT_PATH + f"/model_ep_{episode}.pt")

            print(f"Checkpoint updated at episode {episode}")

            # Export weights for C engine
            if EXPORT_WEIGHTS:
                export_weights_bin(model, log_std.detach().cpu(), EXPORT_PATH + "/ac_weights.bin")

        # Clear rollout buffers
        states.clear()
        old_log_probs.clear()
        values.clear()
        rewards.clear()
        us.clear()

conn.close()