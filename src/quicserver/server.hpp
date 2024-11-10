#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <boost/interprocess/managed_shared_memory.hpp>
#include "simulation_protocol_generated.h"
#include "common/network.hpp"
#include <map>

class QuicServer {
private:
    // Pre-allocated buffers for QUIC messages
    std::vector<uint8_t> send_buffer_;
    std::vector<uint8_t> receive_buffer_;
    
    // Shared memory for local connections
    std::unique_ptr<boost::interprocess::managed_shared_memory> shared_memory_;
    
    // Connection mapping
    struct Connection {
        bool is_local;  // true = shared memory, false = QUIC
        uint32_t client_id;
        // Additional connection info
    };
    std::vector<Connection> connections_;

    std::unique_ptr<QuicConnection> quic_connection_;
    std::map<uint32_t, std::unique_ptr<QuicConnection>> client_connections_;

    void handle_client_message(uint32_t client_id, const uint8_t* data, size_t len);

public:
    QuicServer(const std::string& config_path);
    
    // Initialize server and load configuration
    bool init();
    
    // Single step of simulation
    bool step(uint64_t timestep_us);
    
    // Pre-allocate buffers and resources
    void prepare_simulation();
}; 