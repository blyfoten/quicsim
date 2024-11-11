# QUIC-based Distributed Simulation Framework

A high-performance distributed simulation framework using QUIC protocol for network communication and FMI 2.0 for simulation components. The framework supports both local (shared memory) and distributed execution modes.

## Features

- High-performance QUIC-based communication using MSQUIC
- Support for FMI 2.0 Co-Simulation
- Local execution using Boost.Interprocess shared memory
- Distributed execution using QUIC protocol
- Cross-platform support (Windows/Linux)
- Pre-allocated buffers for minimal runtime allocations
- Flatbuffers for efficient serialization
- YAML-based configuration

## Requirements

- C++17 compiler
- CMake 3.15 or higher
- Conan package manager
- MSQUIC library (see installation instructions below)

### Dependencies

Most dependencies are managed through Conan:
- Boost 1.81.0 (system, filesystem)
- yaml-cpp 0.7.0
- flatbuffers 2.0.8
- OpenSSL 3.1.0
- cppfmu 1.0 (for FMU integration)

MSQUIC is handled as an external dependency.

### Installing MSQUIC

#### Linux
1. Clone MSQUIC repository:
   ```bash
   cd external
   git clone --recursive https://github.com/microsoft/msquic.git
   cd msquic
   ```

2. Install prerequisites:
   ```bash
   sudo apt-get install -y \
       build-essential \
       cmake \
       libssl-dev \
       ninja-build \
       lttng-tools \
       liblttng-ust-dev
   ```

3. Build and install:
   ```bash
   mkdir build && cd build
   cmake -G 'Ninja' ..
   ninja
   sudo ninja install
   ```

#### Windows
1. Clone MSQUIC repository:
   ```powershell
   cd external
   git clone --recursive https://github.com/microsoft/msquic.git
   cd msquic
   ```

2. Prerequisites:
   - Visual Studio 2019 or newer with C++ workload
   - [CMake](https://cmake.org/download/)
   - [Ninja](https://github.com/ninja-build/ninja/releases)

3. Build:
   ```powershell
   mkdir build
   cd build
   cmake -G 'Visual Studio 16 2019' -A x64 ..
   cmake --build . --config Release
   ```

## Building the Project

### Option 1: Using Dev Container (Recommended for Linux builds)

1. Prerequisites:
   - Install [Docker](https://docs.docker.com/get-docker/)
   - Install [Visual Studio Code](https://code.visualstudio.com/)
   - Install VS Code extension: [Dev Containers](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers)

2. Clone and open the project:
   ```bash
   git clone https://github.com/yourusername/quic-simulation.git
   cd quic-simulation
   code .
   ```

3. When VS Code opens, click "Reopen in Container"
   - The container will automatically install all dependencies including MSQUIC
   - First build might take a few minutes as it builds MSQUIC from source

4. Build the project:
   ```bash
   cd /workspace
   mkdir build && cd build
   conan install .. --build=missing
   cmake ..
   cmake --build .
   ```

### Option 2: Manual Build

1. Install dependencies:
   ```bash
   # Install MSQUIC following the instructions above
   
   # Install Conan
   pip install conan==1.62.0
   
   # Setup Conan profile (first time only)
   conan profile new default --detect
   # On Linux:
   conan profile update settings.compiler.libcxx=libstdc++11 default
   # On Windows:
   conan profile update settings.compiler.runtime=MD default
   ```

2. Build:
   ```bash
   mkdir build && cd build
   conan install .. --build=missing
   cmake ..
   cmake --build .
   ```

### Prerequisites

1. Add required Conan remote for SINTEF packages:
   ```bash
   conan remote add sintef-public https://artifactory.smd.sintef.no/artifactory/api/conan/conan-local
   ```
   This remote provides access to the cppfmu package required for FMU integration.

   Note: If you encounter authentication issues with the SINTEF remote, you may need to:
   - Request access from SINTEF
   - Use appropriate credentials if provided
   - Check your network/proxy settings

2. Install dependencies:
   ```bash
   # Install Conan
   pip install conan==1.62.0
   
   # Setup Conan profile (first time only)
   conan profile new default --detect
   # On Linux:
   conan profile update settings.compiler.libcxx=libstdc++11 default
   # On Windows:
   conan profile update settings.compiler.runtime=MD default
   ```

3. Install Conan 2.0:
   ```bash
   pip install conan==2.0.0
   ```

## FMU Integration

The project uses cppfmu for FMU integration, which provides:
- Clean C++ interface for FMI
- Exception-based error handling
- RAII-based resource management
- Built-in logging support

### Loading FMUs

FMUs can be loaded using the configuration file:
```yaml
clients:
  - id: 1
    type: "local"
    fmu_path: "/path/to/your.fmu"
```

The client will automatically:
1. Load the FMU using cppfmu
2. Initialize the simulation
3. Handle variable mapping
4. Manage FMU lifecycle

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

### Third-Party Licenses

This software includes the following third-party libraries:

#### Boost
- License: Boost Software License 1.0
- Copyright (c) Boost Contributors
- https://www.boost.org/LICENSE_1_0.txt

#### yaml-cpp
- License: MIT License
- Copyright (c) 2008-2015 Jesse Beder
- https://github.com/jbeder/yaml-cpp/blob/master/LICENSE

#### FlatBuffers
- License: Apache License 2.0
- Copyright (c) 2014 Google Inc.
- https://github.com/google/flatbuffers/blob/master/LICENSE.txt

#### OpenSSL
- License: OpenSSL License / SSLeay License
- Copyright (c) 1998-2023 The OpenSSL Project
- Copyright (c) 1995-1998 Eric Young
- https://www.openssl.org/source/license.html

#### quiche
- License: MIT License
- Copyright (c) Microsoft Corporation
- https://github.com/microsoft/quiche/blob/main/LICENSE

### License Compliance Requirements

When using this software, you must:

1. Include the above copyright notices
2. Include a copy of the MIT License
3. Include copies of all third-party licenses
4. For modifications to any of the libraries, comply with their respective licenses

The MIT License is compatible with all included libraries, and you can safely use this software in both open-source and commercial projects, provided you maintain the license notices and attributions as described above.

## Development

### Using Dev Container

The dev container provides a consistent development environment with:
- All required dependencies pre-installed
- Proper compiler and build tools
- Pre-configured VS Code extensions
- Debug configuration ready to use

#### Available Tasks
- `Create Build Directory`: Creates the build directory
- `Conan Install`: Installs dependencies using Conan
- `CMake Configure`: Configures the CMake project
- `CMake Build`: Builds the project

#### Debugging
Two debug configurations are available:
1. `Debug Server`: Launches the server with default configuration
2. `Debug Client`: Launches a client instance

#### Container Features
- Ubuntu 22.04 base image
- GCC/G++ compiler suite
- CMake and Ninja build system
- Conan package manager
- GDB debugger
- Git
- Python 3
- OpenSSL development libraries

#### Customizing the Dev Container
You can customize the development environment by:
1. Modifying `.devcontainer/Dockerfile`
2. Adding extensions in `.devcontainer/devcontainer.json`
3. Updating VS Code settings in `.devcontainer/devcontainer.json`

#### Troubleshooting Dev Container
1. If the container fails to build:
   ```bash
   docker system prune -a
   ```
   Then try rebuilding the container

2. If dependencies fail to install:
   ```bash
   sudo rm -rf build/
   conan remove -f "*"
   ```
   Then rebuild using `Ctrl+Shift+B`

3. For permission issues:
   - The container runs as non-root user 'vscode'
   - Use `sudo` for operations requiring root access

[Rest of the README remains the same...]