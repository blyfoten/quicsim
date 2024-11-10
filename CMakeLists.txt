cmake_minimum_required(VERSION 3.15)
project(quic_simulation CXX)

# Conan setup
include(${CMAKE_BINARY_DIR}/conan_paths.cmake OPTIONAL)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find dependencies (Conan-provided)
find_package(Boost REQUIRED COMPONENTS system filesystem)
find_package(yaml-cpp REQUIRED)
find_package(Flatbuffers REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(msquic REQUIRED)  # Using msquic instead of quiche for better platform support

# Generate Flatbuffers code
flatbuffers_generate_headers(
    TARGET simulation_protocol
    SCHEMAS ${CMAKE_SOURCE_DIR}/schemas/simulation_protocol.fbs
)

# Common library
add_library(simulation_common
    src/common/shared_memory.hpp
    src/common/network.cpp
    src/common/network.hpp
)
target_include_directories(simulation_common 
    PUBLIC 
        src
        ${FLATBUFFERS_INCLUDE_DIRS}
)
target_link_libraries(simulation_common
    PUBLIC
        msquic::msquic
        OpenSSL::SSL
        OpenSSL::Crypto
)

# Server executable
add_executable(quicserver
    src/quicserver/server.cpp
    src/quicserver/server.hpp
)
target_link_libraries(quicserver 
    PRIVATE
        simulation_common
        Boost::system
        Boost::filesystem
        yaml-cpp
        flatbuffers
)

# Client executable
add_executable(quicclient
    src/quicclient/client.cpp
    src/quicclient/client.hpp
)
target_link_libraries(quicclient 
    PRIVATE
        simulation_common
        flatbuffers
) 