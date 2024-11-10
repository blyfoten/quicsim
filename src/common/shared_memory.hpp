#pragma once
#include <cstdint>

#pragma pack(push, 1)
struct SharedMemoryLayout {
    struct Variable {
        uint32_t value_reference;
        uint8_t type;  // 0=Real, 1=Integer, 2=Boolean, 3=String
        union Value {
            double real;
            int32_t integer;
            uint8_t boolean;
            uint32_t string_offset;  // Offset into string pool
        } value;
    };

    uint64_t current_time_us;
    uint32_t variable_count;
    Variable variables[1];  // Flexible array member
};
#pragma pack(pop) 