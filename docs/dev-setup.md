# Dev Environment Setup (Amazon Linux 2023 + Qt 6.9.3)

This document describes how to set up a Phoenix development machine and validate the toolchain.

## Host OS
- **Amazon Linux 2023**  
  - Kernel 6.1.x  
  - gcc 11.5.0, CMake 3.22.2, Ninja 1.10+

## Prerequisites
Install base build tools + OpenGL dev headers (already on dev-01; list kept for reproducibility):
```bash
sudo dnf update -y
sudo dnf install -y gcc gcc-c++ ninja-build cmake \
  mesa-libGL-devel libglvnd-devel \
  xz unzip tar git
