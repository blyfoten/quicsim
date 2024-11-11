#include "client.hpp"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <path_to_fmu>" << std::endl;
        return 1;
    }

    std::string fmu_path = argv[1];
    
    try {
        QuicClient client(fmu_path);
        
        if (!client.init()) {
            std::cerr << "Failed to initialize client" << std::endl;
            return 1;
        }

        // Main client loop
        while (true) {
            // Process messages and handle simulation steps
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
} 