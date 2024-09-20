#ifndef CPU_H
#define CPU_H

#include <cstdint>
#include "Memory.h"

struct CPU
{

    // registers
    uint8_t PC; // program counter
    uint8_t SP; // stack pointer

    uint8_t A; // accumulator
    uint8_t X; // x register
    uint8_t Y; // y register

    // processor status registers
    uint8_t C; // carry flag
    uint8_t Z; // zero flag
    uint8_t I; // interrupt disable
    uint8_t D; // decimal mode
    uint8_t B; // break command
    uint8_t V; // overflow command
    uint8_t N; // negative number

    Memory memory;

    int cycles_taken;

    // functions
    void reset();
    uint8_t fetchByte();
    int fetchBytes(int);
    int fetchBytesFromLocation(int, uint8_t);

    void execute(int);
    uint8_t consumeCycles(uint8_t, uint8_t);
};

#endif // CPU

/** ADDRESSING MODES
 * Immediate addressing - 2 cycles - LDA #10
 * 1. fetch opcode
 * 2. load value
 *
 * Zero page addressing - 3 cycles - LDA $10
 * 1. fetch opcode
 * 2. fetch address (zero page)
 * 3. load from address
 *
 * Zero page + X addressing - 4 cycles, LDA $10,X
 * 1. fetch opcode
 * 2. fetch address (zero page)
 * 3. add X to address
 * 5. load from address
 *
 * Absolute addressing - 4 cycles - LDA $00FF
 * 1. fetch opcode
 * 2-3. fetch address
 * 4. load from address
 *
 * Absolute + X/Y addressing - 4+1 cycles - LDA $2580,X
 * 1. fetch opcode
 * 2-3. fetch address
 * 4. add X/Y to address, load from address
 * --5. if crossed pg
 *
 * Indexed indirect addressing - 6 cycles - LDA ($20, X)
 * 1. fetch opcode
 * 2. fetch address of address
 * 3. add X to address of address
 * 4-5. fetch address
 * 6. load from address
 *
 * Indirect indexed addressing - 5+1 cycles - LDA ($20), Y
 * 1. fetch opcode
 * 2. fetch address of address
 * 3-4. fetch address
 * 5. add Y to address, load from address
 * --6. if crossed pg
 */
