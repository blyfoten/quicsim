#pragma once
#include <cstdint>
#include <memory>
#include <vector>
#include <cosim/execution.hpp>
#include <cosim/algorithm.hpp>
#include <cosim/algorithm/fixed_step_algorithm.hpp>
#include <cosim/fmi/fmu.hpp>
#include <cosim/fmi/importer.hpp>
#include "simulation_protocol_generated.h"
#include "common/network.hpp"

class QuicClient {
private:
    // Pre-allocated buffers
    std::vector<uint8_t> send_buffer_;
    std::vector<uint8_t> receive_buffer_;
    
    // Cosim FMU instance
    std::shared_ptr<cosim::fmi::fmu> fmu_;
    std::shared_ptr<cosim::fmi::slave_instance> slave_;
    std::shared_ptr<cosim::fixed_step_algorithm> algorithm_;
    std::unique_ptr<cosim::execution> execution_;
    
    // Network connection
    std::unique_ptr<QuicConnection> quic_connection_;

    // Cache for variable references and values
    struct VariableCache {
        uint32_t reference;
        bool is_output;
    };
    std::vector<VariableCache> variable_cache_;

public:
    QuicClient(const std::string& fmu_path);
    
    // Initialize FMU and prepare caches
    bool init();
    
    // Handle incoming step request
    bool handle_step_request(const SimProtocol::StepRequest* request);
    
    // Pre-allocate all needed resources
    void prepare_simulation();
}; 