#include "client.hpp"
#include <iostream>
#include <algorithm>
#include <flatbuffers/flatbuffers.h>

QuicClient::QuicClient(const std::string& fmu_path) 
    : fmu_instance_(nullptr)
    , send_buffer_(1024 * 1024)  // 1MB pre-allocated buffer
    , receive_buffer_(1024 * 1024) {
    
    // Load FMU here
    // TODO: Implement FMU loading using fmi2Functions
}

bool QuicClient::init() {
    if (!fmu_instance_) {
        std::cerr << "FMU not loaded" << std::endl;
        return false;
    }

    quic_connection_ = std::make_unique<QuicConnection>(false);
    if (!quic_connection_->connect("localhost", 8080)) {
        std::cerr << "Failed to connect to server" << std::endl;
        return false;
    }

    quic_connection_->set_message_handler(
        [this](const uint8_t* data, size_t len) {
            auto msg = flatbuffers::GetRoot<SimProtocol::Message>(data);
            if (msg->message_type() == SimProtocol::MessageType_StepRequest) {
                handle_step_request(msg->message_as_StepRequest());
            }
        });

    // Pre-cache all variable references
    // This is just an example - you'll need to get these from the FMU
    variable_cache_.clear();
    
    // Example: Cache some variables
    VariableCache cache;
    cache.ref = 0;  // Get from FMU
    cache.value_ptr = new double(0.0);  // Pre-allocate memory
    cache.value_size = sizeof(double);
    cache.is_output = true;
    variable_cache_.push_back(cache);
    
    return true;
}

bool QuicClient::handle_step_request(const SimProtocol::StepRequest* request) {
    if (!request) return false;

    // Update input values
    for (const auto* input : *request->inputs()) {
        auto it = std::find_if(variable_cache_.begin(), variable_cache_.end(),
            [input](const VariableCache& cache) {
                return cache.ref == input->valueReference();
            });
            
        if (it != variable_cache_.end()) {
            // Set value in FMU
            switch (input->value_type()) {
                case SimProtocol::ValueType_Real:
                    fmi2SetReal(fmu_instance_, &it->ref, 1, &input->value_as_Real());
                    break;
                // Handle other types...
            }
        }
    }

    // Do the actual FMU step
    fmi2Status status = fmi2DoStep(fmu_instance_, 
                                 0.0,  // current_time 
                                 request->timestep_us() / 1e6,  // convert to seconds
                                 true); // noSetFMUStatePriorToCurrentPoint

    if (status != fmi2OK) {
        return false;
    }

    // Build response with updated outputs
    flatbuffers::FlatBufferBuilder builder;
    std::vector<flatbuffers::Offset<SimProtocol::Variable>> outputs;
    
    // Get updated outputs
    for (const auto& cache : variable_cache_) {
        if (cache.is_output) {
            double value;
            fmi2GetReal(fmu_instance_, &cache.ref, 1, &value);
            
            auto var = SimProtocol::CreateVariable(
                builder,
                builder.CreateString(""),  // name
                SimProtocol::ValueType_Real,
                cache.ref,
                value);
            outputs.push_back(var);
        }
    }

    auto response = SimProtocol::CreateStepResponse(
        builder,
        builder.CreateVector(outputs));

    auto message = SimProtocol::CreateMessage(
        builder,
        SimProtocol::MessageType_StepResponse,
        response.Union());

    builder.Finish(message);

    // Copy to send buffer
    memcpy(send_buffer_.data(), builder.GetBufferPointer(), builder.GetSize());
    
    return true;
} 