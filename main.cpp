#include <iostream>
#include <cstdint>
#include "CPU.h"

int main()
{
    CPU cpu;

    cpu.memory.data[0x42] = 0xFF;
    // std::cout << "Data at memory address 0x42: " << static_cast<int>(cpu.memory.data[0x42]) << std::endl;
    std::cout << "CPU Online.\n";
    return 0;
}