#include "client.hpp"
#include <iostream>
#include <algorithm>
#include <flatbuffers/flatbuffers.h>
#include <cosim/fmi/importer.hpp>
#include <cosim/algorithm.hpp>
#include <cosim/time.hpp>

QuicClient::QuicClient(const std::string& fmu_path) 
    : send_buffer_(1024 * 1024)  // 1MB pre-allocated buffer
    , receive_buffer_(1024 * 1024) {
    
    try {
        // Create FMI importer and load FMU
        auto importer = cosim::fmi::importer::create();
        fmu_ = importer->import(cosim::filesystem::path(fmu_path));
        
        // Create slave instance
        slave_ = fmu_->instantiate_slave("instance1");

        // Setup slave with start time and optional stop time
        slave_->setup(
            cosim::to_time_point(0.0),  // start time
            cosim::to_time_point(1.0),  // stop time (optional)
            std::nullopt              // relative tolerance (optional)
        );

        // Cache variable information from model description
        auto model_desc = fmu_->model_description();
        for (const auto& var : model_desc->variables) {
            if (var.causality == cosim::variable_causality::input ||
                var.causality == cosim::variable_causality::output) {
                VariableCache cache{
                    static_cast<uint32_t>(var.reference),
                    var.causality == cosim::variable_causality::output
                };
                variable_cache_.push_back(cache);
            }
        }

    } catch (const std::exception& e) {
        std::cerr << "Failed to load FMU: " << e.what() << std::endl;
    }
}

bool QuicClient::init() {
    if (!slave_) {
        std::cerr << "FMU not loaded" << std::endl;
        return false;
    }

    try {
        quic_connection_ = std::make_unique<QuicConnection>(false);
        if (!quic_connection_->connect("localhost", 8080)) {
            std::cerr << "Failed to connect to server" << std::endl;
            return false;
        }

        quic_connection_->set_message_handler(
            [this](const uint8_t* data, size_t len) {
                auto msg = flatbuffers::GetRoot<SimProtocol::Message>(data);
                if (msg->message_type_type() == SimProtocol::MessageType_StepRequest) {
                    handle_step_request(msg->message_type_as_StepRequest());
                }
            });

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error during initialization: " << e.what() << std::endl;
        return false;
    }
}

bool QuicClient::handle_step_request(const SimProtocol::StepRequest* request) {
    if (!request) return false;

    try {
        // Update input values using get_real_variables/set_real_variables
        std::vector<uint32_t> input_refs;
        std::vector<double> input_values;
        
        for (const auto* input : *request->inputs()) {
            if (input->value_type() == SimProtocol::ValueType_Real) {
                input_refs.push_back(input->value_reference());
                input_values.push_back(input->real_value());
            }
        }

        if (!input_refs.empty()) {
            slave_->set_real_variables(
                gsl::make_span(input_refs),
                gsl::make_span(input_values)
            );
        }

        // Do the FMU step
        const double stepSize = request->timestep_us() / 1e6;  // Convert to seconds
        slave_->do_step(
            cosim::to_time_point(0.0),
            cosim::to_duration(stepSize)   // step size
        );

        // Build response with updated outputs
        flatbuffers::FlatBufferBuilder builder;
        std::vector<flatbuffers::Offset<SimProtocol::Variable>> outputs;
        
        // Get updated outputs using get_real_variables
        std::vector<uint32_t> output_refs;
        std::vector<double> output_values;
        
        for (const auto& cache : variable_cache_) {
            if (cache.is_output) {
                output_refs.push_back(cache.reference);
            }
        }

        if (!output_refs.empty()) {
            output_values.resize(output_refs.size());
            slave_->get_real_variables(
                gsl::make_span(output_refs),
                gsl::make_span(output_values)
            );

            for (size_t i = 0; i < output_refs.size(); ++i) {
                auto var = SimProtocol::CreateVariable(
                    builder,
                    builder.CreateString(""),  // name
                    SimProtocol::ValueType_Real,  // value_type
                    output_refs[i],             // value_reference (now uint32_t)
                    output_values[i]            // real_value
                );
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

        // Send response
        return quic_connection_->send(
            builder.GetBufferPointer(), 
            builder.GetSize()
        );

    } catch (const std::exception& e) {
        std::cerr << "Error during step: " << e.what() << std::endl;
        return false;
    }
} 