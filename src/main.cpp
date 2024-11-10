#include "quicserver/server.hpp"
#include "quicclient/client.hpp"
#include <iostream>
#include <thread>
#include <chrono>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " [server|client] [config_path]" << std::endl;
        return 1;
    }
    
    std::string mode = argv[1];
    std::string config_path = argv[2];
    
    if (mode == "server") {
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
        
    } else if (mode == "client") {
        QuicClient client("/path/to/fmu.fmu");  // Get from config
        if (!client.init()) {
            std::cerr << "Failed to initialize client" << std::endl;
            return 1;
        }
        
        // Client main loop
        while (true) {
            // Wait for and handle requests
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    
    return 0;
} 