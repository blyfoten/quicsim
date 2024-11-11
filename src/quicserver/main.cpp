#include "server.hpp"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <config_path>" << std::endl;
        return 1;
    }

    std::string config_path = argv[1];
    
    try {
        QuicServer server(config_path);
        
        if (!server.init()) {
            std::cerr << "Failed to initialize server" << std::endl;
            return 1;
        }

        // Simple simulation loop
        uint64_t current_time_us = 0;
        const uint64_t step_size_us = 1000;  // 1ms steps
        
        while (true) {
            if (!server.step(step_size_us)) {
                std::cerr << "Simulation step failed" << std::endl;
                break;
            }
            current_time_us += step_size_us;
            std::this_thread::sleep_for(std::chrono::microseconds(step_size_us));
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
} 