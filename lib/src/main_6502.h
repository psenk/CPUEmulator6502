#include <stdio.h>
#include <stdlib.h>

// http://6502.org/users/obelisk/

// register = small area of memory for computer to access
// - 16 bit addresses

// custom types
using Byte = unsigned char;  // 8-bit
using Word = unsigned short; // 16-bit

using u32 = unsigned int;
using s32 = signed int;

// memory
struct Mem
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
       defining custom [] operator
       return by reference & */
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

// CPU
struct CPU
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

    Byte C : 1; // carry flag
    Byte Z : 1; // zero flag
    Byte I : 1; // interrupt disable
    Byte D : 1; // decimal mode
    Byte B : 1; // break command
    Byte V : 1; // overflow flag
    Byte N : 1; // negative flag

    /* reset CPU */
    void reset(Mem &memory)
    {
        PC = 0xFFFC;
        SP = 0x0100;
        A = X = Y = 0;
        C = Z = I = D = B = V = N = 0;
        memory.initialize();
    }

    /* fetch one byte from memory
       cycles and memory by reference */
    Byte fetchByte(s32 &cycles, Mem &memory)
    {
        Byte data = memory[PC];
        PC++;
        cycles--;
        return data;
    }

    /* read byte from memory
       cycles and memory by reference
       no PC increment (no code being executed) */
    Byte readByteFromMemory(s32 &cycles, Byte address, Mem &memory)
    {
        Byte data = memory[address];
        cycles--;
        return data;
    }

    Byte readByteFromARegister(s32 &cycles)
    {
        Byte data = A;
        cycles--;
        return data;
    }

    Byte readByteFromXRegister(s32 &cycles)
    {
        Byte data = X;
        cycles--;
        return data;
    }

    Byte readByteFromYRegister(s32 &cycles)
    {
        Byte data = Y;
        cycles--;
        return data;
    }

    /* fetch one word from memory
       cycles and memory by reference
       little endian */
    Word fetchWord(s32 &cycles, Mem &memory)
    {
        Word data = memory[PC];
        PC++;

        data |= (memory[PC] << 8);
        PC++;

        // handle endianness?

        cycles -= 2;
        return data;
    }

    static constexpr Byte INS_JSR = 0x20;
    static constexpr Byte INS_LDA_IMM = 0xA9;
    static constexpr Byte INS_LDA_ZPG = 0xA5;
    static constexpr Byte INS_LDA_ZPX = 0xB5;

    void LDA_setStatus()
    {
        Z = (A == 0);
        N = (A & 0b10000000) > 0;
    }

    /* execute code */
    s32 execute(s32 cycles, Mem &memory)
    {
        const u32 cyclesRequested = cycles;
        while (cycles > 0)
        {
            Byte instruction = fetchByte(cycles, memory);
            switch (instruction)
            {
            // jump to subroutine
            case INS_JSR:
            {
                Word subroutineAddress = fetchWord(cycles, memory);
                memory.writeWord(cycles, PC - 1, SP);
                SP += 2;
                PC = subroutineAddress;
                cycles--;
                break;
            }
            // load accumulator immediate
            case INS_LDA_IMM: // test complete
            {
                Byte value = fetchByte(cycles, memory);
                A = value;
                LDA_setStatus();
                break;
            }
            // load accumulator from zero page address
            case INS_LDA_ZPG: // test complete
            {
                Byte zeroPageAddress = fetchByte(cycles, memory);
                A = readByteFromMemory(cycles, zeroPageAddress, memory);
                LDA_setStatus();
                break;
            }

            case INS_LDA_ZPX: // test complete
            {
                Byte zeroPageAddress = fetchByte(cycles, memory);
                Byte xRegister = readByteFromXRegister(cycles);
                Byte newAddress = zeroPageAddress + xRegister;
                A = readByteFromMemory(cycles, newAddress, memory);
                LDA_setStatus();
                break;
            }
            default:
            {
                printf("Instruction not handled: %d", instruction);
                break;
            }
            }
        }
        const s32 cyclesUsed = cyclesRequested - cycles;
        return cyclesUsed;
    }
};