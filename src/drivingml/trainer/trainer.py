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

# replace model with an actor-critic network
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
entropy_coef = 0.02
value_loss_coef = 0.5
grad_clip_norm = 0.5
lr = 3e-4   # more stable than 1e-3 for actor-critic
gamma = 0.99
rollout_length = 256
update_count = 0

log_std = nn.Parameter(torch.ones(2) * -1.0)  # std ~= 0.37

# This is an Adam optimizer which stands for Adaptive Moment Estimation
# model.parameters() are the weights to update
# lr=1e-3 is how big each update should be, it's a common default learning rate for Adam because it works well in many cases
# Adam is used for driving AI because it handles noisy, unstable, and non-stationary gradients very well
# optimizer = optim.Adam(model.parameters(), lr=1e-3)
optimizer = optim.Adam(list(model.parameters()) + [log_std], lr=lr)

# We use a large gamma close to 1 to prioritize long term rewards and avoid immediate rewards so it doesn't become greedy
gamma = 0.99
log_probs = []
rewards = []
values = []
entropies = []
episode = 0
mean_raws = []      # NEW
actions_taken = []  # NEW

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

    x, y, speed, azimuth,reward, done = struct.unpack("<5fi", data)
    speed = np.tanh(speed / 50.0)
    azimuth = np.sin(azimuth), np.cos(azimuth)

    state = torch.tensor([x, y, speed, azimuth[0], azimuth[1]], dtype=torch.float32)

    mean_raw, value = model(state)
    mean = 2.0 * torch.tanh(mean_raw / 2.0)

    std = log_std.exp().expand_as(mean)
    base_dist = D.Normal(mean, std)
    dist = D.TransformedDistribution(base_dist, [D.transforms.TanhTransform(cache_size=1)])

    action = dist.rsample()
    log_prob = dist.log_prob(action).sum(-1)

    # Store debug info
    mean_raws.append(mean_raw.detach())
    actions_taken.append(action.detach())
    log_probs.append(log_prob)
    values.append(value)
    rewards.append(reward)
    entropies.append(base_dist.entropy().sum(-1))

    # Send action back
    with torch.no_grad():
        action_np = action.cpu().numpy().astype("float32")

    conn.send(struct.pack("<2f", *action_np))

    # If episode finished the we train and save train
    # Trigger training on rollout or episode end
    if done == 1 or len(rewards) >= rollout_length:
        if done == 1:
            episode += 1

        # ----- Bootstrap value if not done -----
        if done == 0:
            with torch.no_grad():
                _, next_value = model(state)
            bootstrap_value = next_value.item()
        else:
            bootstrap_value = 0.0

        # ----- Compute bootstrapped returns -----
        returns = []
        G = bootstrap_value
        for r in reversed(rewards):
            G = r + gamma * G
            returns.insert(0, G)

        returns = torch.tensor(returns, dtype=torch.float32)

        # Stack tensors
        log_probs_tensor = torch.stack(log_probs)
        values_tensor = torch.stack(values).squeeze()
        entropies_tensor = torch.stack(entropies)

        # ----- Normalize value targets -----
        ret_mean = returns.mean()
        ret_std = returns.std().clamp_min(1e-8)
        returns_tgt = (returns - ret_mean) / ret_std

        # ----- Advantages -----
        advantages = returns_tgt - values_tensor.detach()

        adv_mean = advantages.mean()
        adv_std = advantages.std()

        if adv_std > 1e-8:
            advantages = (advantages - adv_mean) / adv_std
        else:
            advantages = advantages - adv_mean
        # ----- Losses -----
        policy_loss = -(log_probs_tensor * advantages).mean()
        value_loss = nn.SmoothL1Loss()(values_tensor, returns_tgt)
        entropy_term = entropies_tensor.mean()

        loss = policy_loss + value_loss_coef * value_loss - entropy_coef * entropy_term

        optimizer.zero_grad()
        loss.backward()
        torch.nn.utils.clip_grad_norm_(list(model.parameters()) + [log_std], grad_clip_norm)
        optimizer.step()
        update_count += 1
        print(f"UPDATE {update_count} | steps={len(returns)} | done={done} | ep={episode}")

        with torch.no_grad():
            log_std.clamp_(min=-2.5, max=1.0)

        # ----- Debug (only print at episode end) -----
        if done == 1:
            mean_raw_tensor = torch.stack(mean_raws)
            action_tensor = torch.stack(actions_taken)

            print(
                f"Episode {episode} | "
                f"mean_raw_abs_mean: {mean_raw_tensor.abs().mean().item():.4f} | "
                f"action_abs_mean: {action_tensor.abs().mean().item():.4f} | "
                f"pct_action_sat: {(action_tensor.abs() > 0.95).float().mean().item():.4f} | "
                f"returns_abs_mean: {returns.abs().mean().item():.4f} | "
                f"value_loss: {value_loss.item():.4f}"
            )

            print(
                f"Episode {episode} trained. "
                f"Loss: {loss.item():.4f}, "
                f"policy: {policy_loss.item():.4f}, "
                f"value: {value_loss.item():.4f}, "
                f"entropy: {entropy_term.item():.4f}, "
                f"mean_std: {log_std.exp().mean().item():.4f}"
            )

            if episode % 10 == 0:
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
        log_probs.clear()
        rewards.clear()
        entropies.clear()
        values.clear()
        mean_raws.clear()
        actions_taken.clear()

conn.close()