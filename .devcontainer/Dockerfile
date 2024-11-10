FROM ubuntu:22.04

# Prevent tzdata questions
ENV DEBIAN_FRONTEND=noninteractive 

# Install basic development tools
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    python3 \
    python3-pip \
    ninja-build \
    pkg-config \
    libssl-dev \
    curl \
    gdb \
    && rm -rf /var/lib/apt/lists/*

# Install Conan
RUN pip3 install conan==1.62.0

# Create non-root user
ARG USERNAME=vscode
ARG USER_UID=1000
ARG USER_GID=$USER_UID

RUN groupadd --gid $USER_GID $USERNAME \
    && useradd --uid $USER_UID --gid $USER_GID -m $USERNAME \
    && apt-get update \
    && apt-get install -y sudo \
    && echo $USERNAME ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/$USERNAME \
    && chmod 0440 /etc/sudoers.d/$USERNAME

# Setup Conan for the user
USER $USERNAME
RUN conan profile new default --detect \
    && conan profile update settings.compiler.libcxx=libstdc++11 default \
    && conan remote add conancenter https://center.conan.io

# Create workspace directory
WORKDIR /workspace

# Switch back to root for final setup
USER root

# Cleanup
RUN apt-get clean && rm -rf /var/lib/apt/lists/* 