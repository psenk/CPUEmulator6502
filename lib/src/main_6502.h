#include <iostream>
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

    // opcodes
    static constexpr Byte INS_JSR = 0x20;
    // LDA instructions
    static constexpr Byte INS_LDA_IMM = 0xA9,
                          INS_LDA_ZPG = 0xA5,
                          INS_LDA_ZPX = 0xB5,
                          INS_LDA_ABS = 0xAD,
                          INS_LDA_ABX = 0xBD,
                          INS_LDA_ABY = 0xB9,
                          INS_LDA_INX = 0xA1,
                          INS_LDA_INY = 0xB1;

    /* set common flags for LDA instructions */
    void LDA_setStatus()
    {
        Z = (A == 0);
        N = (A & 0b10000000) > 0;
    }

    /* execute instructions */
    s32 execute(s32 cycles, Mem &memory)
    {
        const s32 cyclesRequested = cycles;
        while (cycles > 0)
        {
            Byte instruction = fetchNextByte(cycles, memory);
            switch (instruction)
            {
            // jump to subroutine
            case INS_JSR:
            {
                Word subroutineAddress = fetchNextWord(cycles, memory);
                memory.writeWord(cycles, PC - 1, SP);
                SP += 2;
                PC = subroutineAddress;
                cycles--;
                break;
            }
            // load accumulator immediate
            case INS_LDA_IMM: // testing complete
            {
                Byte value = fetchNextByte(cycles, memory);
                A = value;
                LDA_setStatus();
                break;
            }
            // load accumulator from zero page address
            case INS_LDA_ZPG: // testing complete
            {
                Byte zeroPageAddress = fetchNextByte(cycles, memory);
                A = fetchByteFromAddress(cycles, zeroPageAddress, memory);
                LDA_setStatus();
                break;
            }
            // load accumulator from zero page address + x register
            case INS_LDA_ZPX: // testing complete
            {
                Byte zeroPageAddress = fetchNextByte(cycles, memory);
                Byte xRegister = fetchByteFromXRegister();
                Byte newAddress = zeroPageAddress + xRegister; // cycle is taken for addition here
                cycles--;
                A = fetchByteFromAddress(cycles, newAddress, memory);
                LDA_setStatus();
                break;
            }
            // load accumulator from absolute address
            case INS_LDA_ABS: // testing complete
            {
                Word absoluteAddress = fetchNextWord(cycles, memory);
                A = fetchByteFromAddress(cycles, absoluteAddress, memory);
                LDA_setStatus();
                break;
            }
            // load accumulator from absolute address + x register
            case INS_LDA_ABX: // testing complete
            {
                Word absoluteAddress = fetchNextWord(cycles, memory);
                Byte xRegister = fetchByteFromXRegister();
                Word newAddress = absoluteAddress + xRegister;

                bool pageCrossed = (absoluteAddress & 0xFF00) != (newAddress & 0xFF00);
                if (pageCrossed)
                {
                    cycles--;
                }
                A = fetchByteFromAddress(cycles, newAddress, memory);
                LDA_setStatus();
                break;
            }
            // load accumulator from absolute address + y register
            case INS_LDA_ABY: // testing complete
            {
                Word absoluteAddress = fetchNextWord(cycles, memory);
                Byte yRegister = fetchByteFromYRegister();
                Word newAddress = absoluteAddress + yRegister;

                bool pageCrossed = (absoluteAddress & 0xFF00) != (newAddress & 0xFF00);
                if (pageCrossed)
                {
                    cycles--;
                }
                A = fetchByteFromAddress(cycles, newAddress, memory);
                LDA_setStatus();
                break;
            }
            // load accumulator with indexed indirect addressing with x register
            case INS_LDA_INX: // testing complete
            {
                Byte zeroPageAddress = fetchNextByte(cycles, memory);
                Byte xRegister = fetchByteFromXRegister();
                Word addedAddress = (zeroPageAddress + xRegister) & 0xFF; // cycle is taken for addition here
                cycles--;
                Word newAddress = fetchWordFromAddress(cycles, addedAddress, memory);
                A = fetchByteFromAddress(cycles, newAddress, memory);
                LDA_setStatus();
                break;
            }
            // load accumulator with indirect indexed addressing with y register
            case INS_LDA_INY: // testing complete
            {
                Byte zeroPageAddress = fetchNextByte(cycles, memory);
                Word newAddress = fetchWordFromAddress(cycles, zeroPageAddress, memory);
                Byte yRegister = fetchByteFromYRegister();
                Word addedAddress = newAddress + yRegister; // no cycles taken adding here

                bool pageCrossed = (newAddress & 0xFF00) != (addedAddress & 0xFF00);
                if (pageCrossed)
                {
                    cycles--;
                }
                A = fetchByteFromAddress(cycles, addedAddress, memory);
                LDA_setStatus();
                break;
            }
            default:
            {
                std::cout << "Instruction not handled: " << instruction << std::endl;
                PC--;
                return -1;
            }
            }
        }
        const s32 cyclesUsed = cyclesRequested - cycles;
        return cyclesUsed;
    }
};