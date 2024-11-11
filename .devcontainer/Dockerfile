FROM ubuntu:22.04

# Prevent tzdata questions
ENV DEBIAN_FRONTEND=noninteractive 

# Install basic development tools and MSQUIC dependencies
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
    # MSQUIC specific dependencies
    lttng-tools \
    liblttng-ust-dev \
    && rm -rf /var/lib/apt/lists/*

# Install Conan
RUN pip3 install conan>=2.0.0

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
    && conan remote add sintef-public https://artifactory.smd.sintef.no/artifactory/api/conan/conan-local

# Create workspace directory
WORKDIR /workspace

# Create external directory for MSQUIC
RUN mkdir -p /workspace/external

# Clone and build MSQUIC
RUN cd /workspace/external \
    && git clone --recursive https://github.com/microsoft/msquic.git \
    && cd msquic \
    && mkdir build \
    && cd build \
    && cmake -G 'Ninja' .. \
    && ninja \
    && sudo ninja install

# Switch back to root for final setup
USER root

# Cleanup
RUN apt-get clean && rm -rf /var/lib/apt/lists/* 