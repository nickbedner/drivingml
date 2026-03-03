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

# Create checkpoint directory to store trained models
os.makedirs("checkpoints", exist_ok=True)

def gaussian_log_prob(u, mean, log_std):
    # u, mean, log_std: [..., act_dim]
    std = log_std.exp()
    base = D.Normal(mean, std)
    return base.log_prob(u).sum(-1)

def tanh_correction(a, eps=1e-6):
    # a is tanh(u) in [-1,1]
    return torch.log(1 - a * a + eps).sum(-1)

# Model replaced with an actor-critic network
class ActorCritic(nn.Module):
    def __init__(self):
        super().__init__()
        self.shared = nn.Sequential(nn.Linear(5, 64), nn.ReLU())
        self.actor = nn.Linear(64, 2) 
        self.critic = nn.Linear(64, 1)
    def forward(self, x):
        h = self.shared(x)
        return self.actor(h), self.critic(h).squeeze(-1)
model = ActorCritic()

# This is the nueral network
# nn.Sequential means to stack layers sequentially
# In our case it's 4 modules stacked in order: Linear, ReLU, Linear, Tanh
# There are two trainable layers(Linear) and two non-trainable layers(ReLU and Tanh)
# However generally we only count the layers with trainable parameters as "layers" in a model, so we say this is a 2-layer model
# A linear layer has weights + bias
# In PyTorch, the weight matrix shape is(output_size, input_size)
# Total number of weights = input_size * output_size
# Bias size = output_siz
# nn.ReLU() stands for Rectified Linear Unit and sets negative values to zero and keeps positive values unchanged, it has no trainable parameters
# It introduces non-linearity so the network can model complex, non-linear relationships such as curves and collisions
# nn.Tanh() stands for Hyperbolic Tangent and squashes values to the range [-1, 1], it also has no trainable parameters
#model = nn.Sequential(
#    nn.Linear(5, 64), # Weight is 5x64 and bias is 64, so total parameters are 5*64 + 64 = 384
#    nn.ReLU(),
#    nn.Linear(64, 2) # Weight is 64x2 and bias is 2, so total parameters are 64*2 + 2 = 130
#    # nn.Tanh()
#)

# Hyperparameters / small sane defaults
entropy_coef = 0.001
value_loss_coef = 0.5
grad_clip_norm = 0.5
lr = 3e-4   # more stable than 1e-3 for actor-critic
gamma = 0.99
rollout_length = 256
update_count = 0

# log_std = nn.Parameter(torch.ones(2) * -1.0)  # std ~= 0.37
# We will reduce exploration just a bit
log_std = nn.Parameter(torch.ones(2) * -1.5)  # std ~= 0.22

# This is an Adam optimizer which stands for Adaptive Moment Estimation
# model.parameters() are the weights to update
# lr=1e-3 is how big each update should be, it's a common default learning rate for Adam because it works well in many cases
# Adam is used for driving AI because it handles noisy, unstable, and non-stationary gradients very well
# optimizer = optim.Adam(model.parameters(), lr=1e-3)
optimizer = optim.Adam(list(model.parameters()) + [log_std], lr=lr)

# We use a large gamma close to 1 to prioritize long term rewards and avoid immediate rewards so it doesn't become greedy
gamma = 0.99
# Buffers
rewards = []
values = []
episode = 0
states = []
old_log_probs = []
us = []

# Resume training from the latest checkpoint if it exists
checkpoint_path = "checkpoints/latest_model.pt"

if os.path.exists(checkpoint_path):
    checkpoint = torch.load(checkpoint_path)
    model.load_state_dict(checkpoint["model_state_dict"])
    log_std.data.copy_(checkpoint.get("log_std", torch.zeros_like(log_std)))
    optimizer.load_state_dict(checkpoint["optimizer_state_dict"])
    episode = checkpoint["episode"]
    print(f"Resumed training from episode {episode}")
else:
    print("Starting fresh training.")

server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.bind((HOST, PORT))
server.listen(1)

print("Waiting for connection...")
conn, addr = server.accept()
print("Connected:", addr)

def recv_exact(sock, size):
    data = b''
    while len(data) < size:
        packet = sock.recv(size - len(data))
        if not packet:
            return None
        data += packet
    return data

while True:
    data = recv_exact(conn, 24)
    if data is None:
        break

    dx, dy, speed, azimuth,reward, done = struct.unpack("<5fi", data)
    speed = np.tanh(speed / 50.0)
    azimuth = np.sin(azimuth), np.cos(azimuth)

    state = torch.tensor([dx, dy, speed, azimuth[0], azimuth[1]], dtype=torch.float32)

    mean, value = model(state)
    log_std_exp = log_std.expand_as(mean)
    std = log_std_exp.exp()

    # ----- Sample pre-tanh -----
    u = mean + std * torch.randn_like(mean)
    a = torch.tanh(u)  # true squashed action in (-1, 1)

    # log prob uses the true tanh transform
    log_prob = gaussian_log_prob(u, mean, log_std_exp) - tanh_correction(a)

    # clamp ONLY for env safety / numeric reasons
    eps = 1e-6
    a_env = a.clamp(-1 + eps, 1 - eps)

    states.append(state.detach())
    us.append(u.detach())
    old_log_probs.append(log_prob.detach())
    values.append(value.detach())
    rewards.append(reward)

    # send clipped action to game
    action_np = a_env.detach().cpu().numpy().astype("float32")
    conn.send(struct.pack("<2f", *action_np))

    # If episode finished the we train and save train
    # Trigger training on rollout or episode end
    if done == 1 or len(rewards) >= rollout_length:
        if done == 1:
            episode += 1

        # Bootstrap value if not done
        if done == 0:
            with torch.no_grad():
                _, next_value = model(state)
            bootstrap_value = next_value.item()
        else:
            bootstrap_value = 0.0
        
        # Convert rollout to tensors
        states_tensor = torch.stack(states)
        old_log_probs_tensor = torch.stack(old_log_probs)
        values_tensor = torch.stack(values).squeeze(-1)
        us_tensor = torch.stack(us)

        # Compute returns once
        returns = []
        G = bootstrap_value
        for r in reversed(rewards):
            G = r + gamma * G
            returns.insert(0, G)
        returns = torch.tensor(returns, dtype=torch.float32, device=values_tensor.device)

        # Compute advantages
        advantages = (returns - values_tensor).detach()
        advantages = (advantages - advantages.mean()) / (advantages.std().clamp_min(1e-8))

        # PPO hyperparameters
        clip_eps = 0.2
        ppo_epochs = 4

        for _ in range(ppo_epochs):
            mean, new_values = model(states_tensor)

            log_std_exp = log_std.expand_as(mean)
            std = log_std_exp.exp()

            u_fixed = us_tensor  # fixed rollout sample
            a_fixed = torch.tanh(u_fixed)

            new_log_probs = gaussian_log_prob(u_fixed, mean, log_std_exp) - tanh_correction(a_fixed)

            ratio = torch.exp(new_log_probs - old_log_probs_tensor)
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

        with torch.no_grad():
            # Reducing exploration just a bit
            # log_std.clamp_(min=-2.5, max=1.0)
            log_std.clamp_(min=-2.0, max=0.5)

        # ----- Debug (only print at episode end) -----
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

            torch.save(checkpoint, "checkpoints/latest_model.pt")
            torch.save(checkpoint, f"checkpoints/model_ep_{episode}.pt")
            print(f"Checkpoint saved at episode {episode}")

        # ----- Clear rollout buffers -----
        states.clear()
        old_log_probs.clear()
        values.clear()
        rewards.clear()
        us.clear()

conn.close()