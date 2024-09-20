#include <cstdint>
#include <stdio.h>
#include "CPU.h"
#include "OpCodes.h"

void CPU::reset()
{

    PC = 0xFFFC;
    SP = 0x0100;

    A = X = Y = 0;

    C = Z = I = D = B = V = N = 0;

    cycles_taken = 0;
}

uint8_t CPU::fetchByte()
{
    uint8_t val = memory.data[PC];
    PC++;
    return val;
}

int CPU::fetchBytes(int len)
{
    int val = 0;
    for (int i = 0; i < len; i++)
    {
        val |= memory.data[PC] << (8 * i);
        PC++;
    }
    return val;
}

int CPU::fetchBytesFromLocation(int len, uint8_t addr)
{
    int val = 0;
    for (int i = 0; i < len; i++)
    {
        val |= memory.data[addr] << (8 * i);
        addr++;
    }
    return val;
}

void CPU::execute(int cycles)
{
    while (cycles > 0)
    {
        // load instruction
        uint8_t ins = fetchByte();
        consumeCycles(cycles, 1);

        // act on instruction
        switch (static_cast<OpCode>(ins))
        {
        case OpCode::LDA_IMM:

        default:
            printf("Unknown op code/instruction.") break;
        }
    }
}

uint8_t CPU::consumeCycles(uint8_t cycles, uint8_t num)
{
    cycles_taken++;
    return cycles - num;
}