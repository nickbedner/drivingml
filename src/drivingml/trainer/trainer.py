#!/usr/bin/env python3

import socket
import struct
import torch
import torch.nn as nn
import torch.optim as optim
import numpy as np
import os

HOST = "127.0.0.1"
PORT = 5000

# Create checkpoint directory
os.makedirs("checkpoints", exist_ok=True)

model = nn.Sequential(
    nn.Linear(2, 64),
    nn.ReLU(),
    nn.Linear(64, 2),
    nn.Tanh()
)

optimizer = optim.Adam(model.parameters(), lr=1e-3)
gamma = 0.99

log_probs = []
rewards = []

episode = 0

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
    data = recv_exact(conn, 16)
    if data is None:
        break

    x, y, reward, done = struct.unpack("<3fi", data)

    # Normalize state world coordinates to [-1, 1]
    x = x / 50
    y = y / 200

    state = torch.tensor([x, y], dtype=torch.float32)

    # Forward pass
    mean = model(state)

    # Add exploration noise
    dist = torch.distributions.Normal(mean, 0.2)
    action = dist.sample()
    action = torch.clamp(action, -1.0, 1.0)

    log_prob = dist.log_prob(action).sum()

    # Store memory
    log_probs.append(log_prob)
    rewards.append(reward)

    # Send action back
    action_np = action.detach().numpy().astype("float32")
    conn.send(struct.pack("<2f", *action_np))

    # If episode finished the we train and save train
    if done == 1:

        episode += 1

        # Compute discounted returns
        returns = []
        G = 0
        for r in reversed(rewards):
            G = r + gamma * G
            returns.insert(0, G)

        returns = torch.tensor(returns)
        returns = (returns - returns.mean()) / (returns.std() + 1e-8)

        # Policy gradient loss
        loss = 0
        for log_prob, G in zip(log_probs, returns):
            loss -= log_prob * G

        optimizer.zero_grad()
        loss.backward()
        optimizer.step()

        print(f"Episode {episode} trained. Loss: {loss.item()}")

        # Save the model
        torch.save(model.state_dict(), "checkpoints/latest_model.pt")
        torch.save(model.state_dict(), f"checkpoints/model_ep_{episode}.pt")

        print("Model saved.")

        log_probs.clear()
        rewards.clear()

conn.close()