#ifndef OPCODES_H
#define OPCODES_H
#include <cstdint>

enum class OpCode : uint8_t
{
    // LDA
    LDA_IMM = 0xA9,
    LDA_ZPG = 0xA5,
    LDA_ZPX = 0xB5,
    LDA_ABS = 0xAD,
    LDA_ABX = 0xBD,
    LDA_ABY = 0xB9,
    LDA_INX = 0xA1,
    LDA_INY = 0xB1,

    // LDX
    LDX_IMM = 0xA2,
    LDX_ZPG = 0xA6,
    LDX_ZPY = 0xB6,
    LDX_ABS = 0xAE,
    LDX_ABY = 0xBE,

    // LDY
    LDY_IMM = 0xA0,
    LDY_ZPG = 0xA4,
    LDY_ZPX = 0xB4,
    LDY_ABS = 0xAC,
    LDY_ABX = 0xBC,

    // STA
    STA_ZPG = 0x85,
    STA_ZPX = 0x95,
    STA_ABS = 0x8D,
    STA_ABX = 0x9D,
    STA_ABY = 0x99,
    STA_INX = 0x81,
    STA_INY = 0x91,

    // STX
    STX_ZPG = 0x86,
    STX_ZPY = 0x96,
    STX_ABS = 0x8E,

    // STY
    STY_ZPG = 0x84,
    STY_ZPX = 0x94,
    STY_ABS = 0x8C
};

#endif // OPCODES_H