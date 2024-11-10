#include "server.hpp"
#include <yaml-cpp/yaml.h>
#include <iostream>
#include <boost/interprocess/mapped_region.hpp>

QuicServer::QuicServer(const std::string& config_path)
    : send_buffer_(1024 * 1024)  // 1MB pre-allocated buffer
    , receive_buffer_(1024 * 1024) {
    
    try {
        // Load configuration
        YAML::Node config = YAML::LoadFile(config_path);
        
        // Initialize shared memory
        size_t shm_size = config["server"]["shared_memory_size"].as<size_t>();
        shared_memory_ = std::make_unique<boost::interprocess::managed_shared_memory>(
            boost::interprocess::create_only,
            "simulation_shared_memory",
            shm_size
        );
        
        // Setup connections from config
        for (const auto& client : config["clients"]) {
            Connection conn;
            conn.is_local = (client["type"].as<std::string>() == "local");
            conn.client_id = client["id"].as<uint32_t>();
            connections_.push_back(conn);
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error initializing server: " << e.what() << std::endl;
    }
}

bool QuicServer::init() {
    try {
        quic_connection_ = std::make_unique<QuicConnection>(true);
        if (!quic_connection_->listen(8080)) {
            throw std::runtime_error("Failed to start QUIC server");
        }
        
        quic_connection_->set_message_handler(
            [this](const uint8_t* data, size_t len) {
                // Handle new client connections
            });
            
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error in server init: " << e.what() << std::endl;
        return false;
    }
}

bool QuicServer::step(uint64_t timestep_us) {
    flatbuffers::FlatBufferBuilder builder;
    
    // Create step request
    auto request = SimProtocol::CreateStepRequest(
        builder,
        timestep_us,
        builder.CreateVector(std::vector<flatbuffers::Offset<SimProtocol::Variable>>())  // Empty inputs for now
    );
    
    auto message = SimProtocol::CreateMessage(
        builder,
        SimProtocol::MessageType_StepRequest,
        request.Union());
    
    builder.Finish(message);
    
    // Send to all clients
    for (const auto& conn : connections_) {
        if (conn.is_local) {
            // Use shared memory
            // TODO: Copy to shared memory region
        } else {
            // Send via QUIC
            // TODO: Implement QUIC send
        }
    }
    
    return true;
}

void QuicServer::prepare_simulation() {
    // Pre-allocate resources
    // TODO: Pre-allocate connection buffers
}

void QuicServer::handle_client_message(uint32_t client_id, const uint8_t* data, size_t len) {
    auto msg = flatbuffers::GetRoot<SimProtocol::Message>(data);
    
    if (msg->message_type() == SimProtocol::MessageType_StepResponse) {
        auto response = msg->message_as_StepResponse();
        // Process response and update connected clients
        // TODO: Implement response handling
    }
} 