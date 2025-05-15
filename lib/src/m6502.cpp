#include "m6502.h"

/* execute instructions */
namespace m6502
{
    s32 CPU::execute(s32 cycles, Mem &memory)
    {
        const s32 cyclesRequested = cycles;
        while (cycles > 0)
        {
            Byte instruction = fetchNextByte(cycles, memory);
            switch (instruction)
            {

            // lda instructions
            // load accumulator immediate
            case INS_LDA_IMM: // testing complete
            {
                Byte value = fetchNextByte(cycles, memory);
                A = value;
                load_setStatus(A);
                break;
            }
            // load accumulator from zero page address
            case INS_LDA_ZPG: // testing complete
            {
                Byte zeroPageAddress = fetchNextByte(cycles, memory);
                A = fetchByteFromAddress(cycles, zeroPageAddress, memory);
                load_setStatus(A);
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
                load_setStatus(A);
                break;
            }
            // load accumulator from absolute address
            case INS_LDA_ABS: // testing complete
            {
                Word absoluteAddress = fetchNextWord(cycles, memory);
                A = fetchByteFromAddress(cycles, absoluteAddress, memory);
                load_setStatus(A);
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
                load_setStatus(A);
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
                load_setStatus(A);
                break;
            }
            // load accumulator with indexed indirect addressing+ x register
            case INS_LDA_INX: // testing complete
            {
                Byte zeroPageAddress = fetchNextByte(cycles, memory);
                Byte xRegister = fetchByteFromXRegister();
                Word addedAddress = (zeroPageAddress + xRegister) & 0xFF; // cycle is taken for addition here
                cycles--;
                Word newAddress = fetchWordFromAddress(cycles, addedAddress, memory);
                A = fetchByteFromAddress(cycles, newAddress, memory);
                load_setStatus(A);
                break;
            }
            // load accumulator with indirect indexed addressing + y register
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
                load_setStatus(A);
                break;
            }

            // ldx instructions
            // load x register immediate
            case INS_LDX_IMM: // testing complete
            {
                Byte value = fetchNextByte(cycles, memory);
                X = value;
                load_setStatus(X);
                break;
            }
            // load x register from zero page address
            case INS_LDX_ZPG: // testing complete
            {
                Byte zeroPageAddress = fetchNextByte(cycles, memory);
                X = fetchByteFromAddress(cycles, zeroPageAddress, memory);
                load_setStatus(X);
                break;
            }
            // load x register from zero page address + y register
            case INS_LDX_ZPY: // testing complete
            {
                Byte zeroPageAddress = fetchNextByte(cycles, memory);
                Byte yRegister = fetchByteFromYRegister();
                Byte newAddress = zeroPageAddress + yRegister; // cycle is taken for addition here
                cycles--;
                X = fetchByteFromAddress(cycles, newAddress, memory);
                load_setStatus(X);
                break;
            }
            // load x register from absolute address
            case INS_LDX_ABS: // testing complete
            {
                Word absoluteAddress = fetchNextWord(cycles, memory);
                X = fetchByteFromAddress(cycles, absoluteAddress, memory);
                load_setStatus(X);
                break;
            }
            // load x register from absolute address + y register
            case INS_LDX_ABY: // testing complete
            {
                Word absoluteAddress = fetchNextWord(cycles, memory);
                Byte yRegister = fetchByteFromYRegister();
                Word newAddress = absoluteAddress + yRegister;

                bool pageCrossed = (absoluteAddress & 0xFF00) != (newAddress & 0xFF00);
                if (pageCrossed)
                {
                    cycles--;
                }
                X = fetchByteFromAddress(cycles, newAddress, memory);
                load_setStatus(X);
                break;
            }

            // ldy instructions
            // load Y register immediate
            case INS_LDY_IMM: // testing complete
            {
                Byte value = fetchNextByte(cycles, memory);
                Y = value;
                load_setStatus(Y);
                break;
            }
            // load y register from zero page address
            case INS_LDY_ZPG: // testing complete
            {
                Byte zeroPageAddress = fetchNextByte(cycles, memory);
                Y = fetchByteFromAddress(cycles, zeroPageAddress, memory);
                load_setStatus(Y);
                break;
            }
            // load y register from zero page address + x register
            case INS_LDY_ZPX: // testing complete
            {
                Byte zeroPageAddress = fetchNextByte(cycles, memory);
                Byte xRegister = fetchByteFromXRegister();
                Byte newAddress = zeroPageAddress + xRegister; // cycle is taken for addition here
                cycles--;
                Y = fetchByteFromAddress(cycles, newAddress, memory);
                load_setStatus(Y);
                break;
            }
            // load y register from absolute address
            case INS_LDY_ABS: // testing complete
            {
                Word absoluteAddress = fetchNextWord(cycles, memory);
                Y = fetchByteFromAddress(cycles, absoluteAddress, memory);
                load_setStatus(Y);
                break;
            }
            // load y register from absolute address + x register
            case INS_LDY_ABX: // testing complete
            {
                Word absoluteAddress = fetchNextWord(cycles, memory);
                Byte xRegister = fetchByteFromXRegister();
                Word newAddress = absoluteAddress + xRegister;

                bool pageCrossed = (absoluteAddress & 0xFF00) != (newAddress & 0xFF00);
                if (pageCrossed)
                {
                    cycles--;
                }
                Y = fetchByteFromAddress(cycles, newAddress, memory);
                load_setStatus(Y);
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
}