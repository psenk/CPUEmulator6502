#include <iostream>
#include <stdlib.h>

// http://6502.org/users/obelisk/

// register = small area of memory for computer to access
// - 16 bit addresses

namespace m6502
{
    // custom types
    using Byte = unsigned char;  // 8-bit
    using Word = unsigned short; // 16-bit

    using u32 = unsigned int;
    using s32 = signed int;

    struct Mem;
    struct CPU;
}

/**
 * MEMORY
 */
struct m6502::Mem
{
    static constexpr u32 MAX_MEM = 1024 * 64;
    Byte data[MAX_MEM];

    /* initialize memory */
    void initialize()
    {
        for (u32 i = 0; i < MAX_MEM; i++)
        {
            data[i] = 0;
        }
    }

    /* read one byte from memory
       defining custom [] operator */
    Byte operator[](u32 address) const
    {
        return data[address];
    }

    /* write one byte to memory
       defining custom [] operator */
    Byte &operator[](u32 address)
    {
        return data[address];
    }

    /* write one word/two bytes to memory */
    void writeWord(s32 &cycles, Word input, u32 address)
    {
        data[address] = input & 0xFF;
        data[address + 1] = input >> 8;
        cycles -= 2;
    }
};

/**
 * CPU
 */
struct m6502::CPU
{
    /*
    Zero Page: $0000 - $00FF - 256 bytes
    Stack: $0100 - $01FF - 256 bytes
    Non-maskable Interrupt Handler: $FFFA/B
    Power on Reset: $FFFC/D
    BRK/Interrupt Request Handler: $FFFE/F
    */

    Word PC; // program counter
    Word SP; // stack pointer

    Byte A, X, Y; // registers

    enum RegisterType;

    Byte C : 1; // carry flag
    Byte Z : 1; // zero flag
    Byte I : 1; // interrupt disable
    Byte D : 1; // decimal mode
    Byte B : 1; // break command
    Byte V : 1; // overflow flag
    Byte N : 1; // negative flag

    /**
     * RESET COMMAND
     */

    void reset(Mem &memory)
    {
        PC = 0xFFFC;
        SP = 0x0100;
        A = X = Y = 0;
        C = Z = I = D = B = V = N = 0;
        memory.initialize();
    }

    /**
     * FETCH FUNCTIONS
     */

    /* fetch next byte from memory */
    Byte fetchNextByte(s32 &cycles, Mem &memory)
    {
        Byte data = memory[PC];
        PC++;
        cycles--;
        return data;
    }

    /* fetch next word from memory
       little endian */
    Word fetchNextWord(s32 &cycles, Mem &memory)
    {
        Word data = memory[PC];
        PC++;

        data |= (memory[PC] << 8);
        PC++;

        // handle endianness?

        cycles -= 2;
        return data;
    }

    /* fetch byte from memory */
    Byte fetchByteFromAddress(s32 &cycles, Word address, Mem &memory)
    {
        Byte data = memory[address];
        cycles--;
        return data;
    }

    /* fetch one word from address
       little endian */
    Word fetchWordFromAddress(s32 &cycles, Word address, Mem &memory)
    {

        Word data = memory[address];
        address++;

        data |= (memory[address] << 8);

        // handle endianness?

        cycles -= 2;
        return data;
    }

    /* fetch byte from the A register */
    Byte fetchByteFromARegister()
    {
        Byte data = A;
        return data;
    }

    /* fetch byte from the X register */
    Byte fetchByteFromXRegister()
    {
        Byte data = X;
        return data;
    }

    /* fetch byte from the Y register */
    Byte fetchByteFromYRegister()
    {
        Byte data = Y;
        return data;
    }

    /**
     * INSTRUCTION OPCODES
     */

    // LDA instructions
    static constexpr Byte INS_LDA_IMM = 0xA9,
                          INS_LDA_ZPG = 0xA5,
                          INS_LDA_ZPX = 0xB5,
                          INS_LDA_ABS = 0xAD,
                          INS_LDA_ABX = 0xBD,
                          INS_LDA_ABY = 0xB9,
                          INS_LDA_INX = 0xA1,
                          INS_LDA_INY = 0xB1;

    // LDX instructions
    static constexpr Byte INS_LDX_IMM = 0xA2,
                          INS_LDX_ZPG = 0xA6,
                          INS_LDX_ZPY = 0xB6,
                          INS_LDX_ABS = 0xAE,
                          INS_LDX_ABY = 0xBE;

    // LDY instructions
    static constexpr Byte INS_LDY_IMM = 0xA0,
                          INS_LDY_ZPG = 0xA4,
                          INS_LDY_ZPX = 0xB4,
                          INS_LDY_ABS = 0xAC,
                          INS_LDY_ABX = 0xBC;

    // STA instructions
    static constexpr Byte INS_STA_ZPG = 0x85,
                          INS_STA_ZPX = 0x95,
                          INS_STA_ABS = 0x8D,
                          INS_STA_ABX = 0x9D,
                          INS_STA_ABY = 0x99,
                          INS_STA_INX = 0x81,
                          INS_STA_INY = 0x91;

    // STX instructions
    static constexpr Byte INS_STA_ZPG = 0x86,
                          INS_STA_ZPY = 0x96,
                          INS_STA_ABS = 0x8E;

    // STY instructions
    static constexpr Byte INS_STA_ZPG = 0x84,
                          INS_STA_ZPX = 0x94,
                          INS_STA_ABS = 0x8C;

    /**
     * ADDRESSING MODES
     */

    s32 fetchAddressZeroPage(s32 &cycles, Mem &memory);
    s32 fetchAddressZeroPagePlusRegister(s32 &cycles, RegisterType reg, Mem &memory);
    s32 fetchAddressAbsolute(s32 &cycles, Mem &memory);
    s32 fetchAddressAbsolutePlusRegister(s32 &cycles, RegisterType reg, Mem &memory);

    /**
     * SET FLAGS
     */
    void setFlagStatusLoad(Byte reg);

    /**
     * CPU EXECUTE FUNCTION
     */
    s32 execute(s32 cycles, Mem &memory);
};