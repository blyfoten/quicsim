cmake_minimum_required(VERSION 3.15)
project(quic_simulation CXX)

# Conan 2.0 setup - use the toolchain file

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find dependencies (using Conan-generated config files)
find_package(Boost REQUIRED COMPONENTS system filesystem)
find_package(yaml-cpp REQUIRED)
find_package(flatbuffers REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(libcosim REQUIRED)

# MSQUIC setup
if(NOT DEFINED MSQUIC_PATH)
    set(MSQUIC_PATH "/usr/local" CACHE PATH "Path to MSQUIC installation")
endif()

if(NOT EXISTS "${MSQUIC_PATH}")
    message(FATAL_ERROR "MSQUIC_PATH not found. Please set MSQUIC_PATH to your MSQUIC installation directory")
endif()

add_library(msquic INTERFACE)
target_include_directories(msquic INTERFACE "${MSQUIC_PATH}/include")
if(WIN32)
    target_link_libraries(msquic INTERFACE "${MSQUIC_PATH}/lib/msquic.lib")
else()
    target_link_libraries(msquic INTERFACE "${MSQUIC_PATH}/lib/libmsquic.so")
endif()

# Generate Flatbuffers code
flatbuffers_generate_headers(
    TARGET generate_flatbuffers
    SCHEMAS "${CMAKE_CURRENT_SOURCE_DIR}/schemas/simulation_protocol.fbs"
    FLAGS --gen-mutable --gen-object-api
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
        msquic
        OpenSSL::SSL
        OpenSSL::Crypto
        generate_flatbuffers
        flatbuffers::flatbuffers
)

# Server executable
add_executable(quicserver
    src/quicserver/server.cpp
    src/quicserver/server.hpp
    src/quicserver/main.cpp
)
target_link_libraries(quicserver 
    PRIVATE
        simulation_common
        Boost::system
        Boost::filesystem
        yaml-cpp
        generate_flatbuffers
        flatbuffers::flatbuffers
)

# Client executable
add_executable(quicclient
    src/quicclient/client.cpp
    src/quicclient/client.hpp
    src/quicclient/main.cpp
)
target_link_libraries(quicclient 
    PRIVATE
        simulation_common
        generate_flatbuffers
        flatbuffers::flatbuffers
        libcosim::cosim
        fmilib::shared
)

# Add dependencies after targets are defined
add_dependencies(simulation_common generate_flatbuffers)
add_dependencies(quicserver generate_flatbuffers)
add_dependencies(quicclient generate_flatbuffers)

# Add include directories after targets are defined
target_include_directories(simulation_common PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(quicserver PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(quicclient PUBLIC ${CMAKE_CURRENT_BINARY_DIR})