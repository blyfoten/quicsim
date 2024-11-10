# QUIC-based Distributed Simulation Framework

A high-performance distributed simulation framework using QUIC protocol for network communication and FMI 2.0 for simulation components. The framework supports both local (shared memory) and distributed execution modes.

## Features

- High-performance QUIC-based communication
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

### Dependencies

All dependencies are managed through Conan:
- Boost 1.81.0 (system, filesystem)
- yaml-cpp 0.7.0
- flatbuffers 2.0.8
- OpenSSL 3.1.0
- msquic 2.1.8 

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

#### MsQuic
- License: MIT License
- Copyright (c) Microsoft Corporation
- https://github.com/microsoft/msquic/blob/main/LICENSE

### License Compliance Requirements

When using this software, you must:

1. Include the above copyright notices
2. Include a copy of the MIT License
3. Include copies of all third-party licenses
4. For modifications to any of the libraries, comply with their respective licenses

The MIT License is compatible with all included libraries, and you can safely use this software in both open-source and commercial projects, provided you maintain the license notices and attributions as described above.

## Building

### Prerequisites

#### Windows
1. Install Visual Studio 2019 or newer
2. Install Python 3.x
3. Install Conan package manager: