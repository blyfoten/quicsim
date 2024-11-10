#pragma once
#include <cstdint>
#include <memory>
#include <vector>
#include "fmi2Functions.h"
#include "simulation_protocol_generated.h"
#include "common/network.hpp"

class QuicClient {
private:
    // FMU instance
    fmi2Component fmu_instance_;
    
    // Pre-allocated buffers
    std::vector<uint8_t> send_buffer_;
    std::vector<uint8_t> receive_buffer_;
    
    // Cache for variable references and values
    struct VariableCache {
        fmi2ValueReference ref;
        void* value_ptr;        // Points to pre-allocated memory
        size_t value_size;
        bool is_output;
    };
    std::vector<VariableCache> variable_cache_;

    std::unique_ptr<QuicConnection> quic_connection_;

public:
    QuicClient(const std::string& fmu_path);
    
    // Initialize FMU and prepare caches
    bool init();
    
    // Handle incoming step request
    bool handle_step_request(const SimProtocol::StepRequest* request);
    
    // Pre-allocate all needed resources
    void prepare_simulation();
}; 