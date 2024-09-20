#ifndef MEMORY_H
#define MEMORY_H

#include <cstdint>

struct Memory {
    static constexpr int MAX_MEMORY = 1024 * 64;
    uint8_t data[MAX_MEMORY];

    // functions
    void zeroize();
};

#endif // MEMORY_H