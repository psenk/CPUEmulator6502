#include "main_6502.h"

int main()
{
    Mem mem;
    CPU cpu;
    cpu.reset(mem);

    return 0;
}