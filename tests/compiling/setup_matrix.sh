#!/bin/bash
# This script file was generated using Google Gemini

set -e

# --- Color Definitions ---
BLUE='\033[0;34m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

echo -e "${BLUE}🛠️ Creating Dockerfile for the test environment...${NC}"

cat << 'EOF' > Dockerfile.matrix
FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    software-properties-common \
    wget \
    gnupg \
    lsb-release \
    cmake \
    ninja-build \
    diffutils \
    g++-aarch64-linux-gnu \
    qemu-user

RUN add-apt-repository ppa:ubuntu-toolchain-r/test -y && \
    apt-get update && \
    apt-get install -y gcc-13 g++-13 gcc-14 g++-14 gcc-15 g++-15

RUN wget https://apt.llvm.org/llvm.sh && chmod +x llvm.sh && \
    ./llvm.sh 18 && \
    ./llvm.sh 19 && \
    ./llvm.sh 20 || true && \
    ./llvm.sh 21 || true

RUN apt-get clean && rm -rf /var/lib/apt/lists/*
EOF

echo -e "${BLUE}🐳 Building Docker image 'vkg_gen_test_env'...${NC}"
docker build -t vkg_gen_test_env -f Dockerfile.matrix .
rm Dockerfile.matrix

echo -e "${GREEN}✅ Setup complete!${NC}"
