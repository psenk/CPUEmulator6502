#include "m6502.h"

int main()
{
    using namespace m6502;
    Mem mem;
    CPU cpu;
    cpu.reset(mem);

    return 0;
}