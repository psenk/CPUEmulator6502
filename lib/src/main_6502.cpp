#include "main_6502.h"

int main()
{
    Mem mem;
    CPU cpu;
    cpu.reset(mem);

    // hardcoding machine code
    mem[0xFFFC] = CPU::INS_LDA_ZPG;
    mem[0xFFFD] = 0x42;
    mem[0x0042] = 0x84;

    cpu.execute(3, mem);
    return 0;
}