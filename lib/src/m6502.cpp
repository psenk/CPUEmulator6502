#include "m6502.h"

namespace m6502
{

    enum CPU::RegisterType
    {
        A_REGISTER,
        X_REGISTER,
        Y_REGISTER
    };

    /**
     * ADDRESSING MODES
     */

    s32 CPU::fetchAddressZeroPage(s32 &cycles, Mem &memory)
    {
        Word zeroPageAddress = fetchNextByte(cycles, memory);
        return zeroPageAddress;
    }

    s32 CPU::fetchAddressZeroPagePlusRegister(s32 &cycles, RegisterType reg, Mem &memory)
    {
        Byte zeroPageAddress = fetchNextByte(cycles, memory);
        Byte registerValue;
        if (reg == X_REGISTER)
        {
            registerValue = fetchByteFromXRegister();
        }
        else
        {
            registerValue = fetchByteFromYRegister();
        }
        Byte effectiveAddress = zeroPageAddress + registerValue; // cycle is taken for addition here
        cycles--;
        return effectiveAddress;
    }

    s32 CPU::fetchAddressAbsolute(s32 &cycles, Mem &memory)
    {
        Word absoluteAddress = fetchNextWord(cycles, memory);
        return absoluteAddress;
    }

    s32 CPU::fetchAddressAbsolutePlusRegister(s32 &cycles, RegisterType reg, Mem &memory)
    {
        Word absoluteAddress = fetchNextWord(cycles, memory);
        Byte registerValue;
        if (reg == X_REGISTER)
        {
            registerValue = fetchByteFromXRegister();
        }
        else
        {
            registerValue = fetchByteFromYRegister();
        }
        Word newAddress = absoluteAddress + registerValue;

        bool pageCrossed = (absoluteAddress & 0xFF00) != (newAddress & 0xFF00);
        if (pageCrossed)
        {
            cycles--;
        }
        return newAddress;
    }

    /**
     * SET FLAGS
     */

    void CPU::setFlagStatusLoad(Byte reg)
    {
        Z = (reg == 0);
        N = (reg & 0b10000000) > 0;
    }

    /**
     * CPU EXECUTE FUNCTION
     */

    s32 CPU::execute(s32 cycles, Mem &memory)
    {
        const s32 cyclesRequested = cycles;
        while (cycles > 0)
        {
            Byte instruction = fetchNextByte(cycles, memory);
            switch (instruction)
            {

            /**
             * LOAD ACCUMULATOR
             */

            // load accumulator immediate
            case INS_LDA_IMM: // testing complete
            {
                A = fetchNextByte(cycles, memory);
                setFlagStatusLoad(A);
                break;
            }

            // load accumulator from zero page address
            case INS_LDA_ZPG: // testing complete
            {
                Word address = fetchAddressZeroPage(cycles, memory);
                A = fetchByteFromAddress(cycles, address, memory);
                setFlagStatusLoad(A);
                break;
            }

            // load accumulator from zero page address + x register
            case INS_LDA_ZPX: // testing complete
            {
                Word effectiveAddress = fetchAddressZeroPagePlusRegister(cycles, X_REGISTER, memory);
                A = fetchByteFromAddress(cycles, effectiveAddress, memory);
                setFlagStatusLoad(A);
                break;
            }

            // load accumulator from absolute address
            case INS_LDA_ABS: // testing complete
            {
                Word effectiveAddress = fetchAddressAbsolute(cycles, memory);
                A = fetchByteFromAddress(cycles, effectiveAddress, memory);
                setFlagStatusLoad(A);
                break;
            }

            // load accumulator from absolute address + x register
            case INS_LDA_ABX: // testing complete
            {
                Word effectiveAddress = fetchAddressAbsolutePlusRegister(cycles, X_REGISTER, memory);
                A = fetchByteFromAddress(cycles, effectiveAddress, memory);
                setFlagStatusLoad(A);
                break;
            }

            // load accumulator from absolute address + y register
            case INS_LDA_ABY: // testing complete
            {
                Word effectiveAddress = fetchAddressAbsolutePlusRegister(cycles, Y_REGISTER, memory);
                A = fetchByteFromAddress(cycles, effectiveAddress, memory);
                setFlagStatusLoad(A);
                break;
            }

            // load accumulator with indexed indirect addressing + x register
            case INS_LDA_INX: // testing complete
            {
                Byte zeroPageAddress = fetchNextByte(cycles, memory);
                Byte xRegister = fetchByteFromXRegister();
                Word addedAddress = (zeroPageAddress + xRegister) & 0xFF; // cycle is taken for addition here
                cycles--;
                Word newAddress = fetchWordFromAddress(cycles, addedAddress, memory);
                A = fetchByteFromAddress(cycles, newAddress, memory);
                setFlagStatusLoad(A);
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
                setFlagStatusLoad(A);
                break;
            }

            /**
             * LOAD X REGISTER
             */

            // load x register immediate
            case INS_LDX_IMM: // testing complete
            {
                X = fetchNextByte(cycles, memory);
                setFlagStatusLoad(X);
                break;
            }

            // load x register from zero page address
            case INS_LDX_ZPG: // testing complete
            {
                Word address = fetchAddressZeroPage(cycles, memory);
                X = fetchByteFromAddress(cycles, address, memory);
                setFlagStatusLoad(X);
                break;
            }

            // load x register from zero page address + y register
            case INS_LDX_ZPY: // testing complete
            {
                Word effectiveAddress = fetchAddressZeroPagePlusRegister(cycles, Y_REGISTER, memory);
                X = fetchByteFromAddress(cycles, effectiveAddress, memory);
                setFlagStatusLoad(X);
                break;
            }

            // load x register from absolute address
            case INS_LDX_ABS: // testing complete
            {
                Word effectiveAddress = fetchAddressAbsolute(cycles, memory);
                X = fetchByteFromAddress(cycles, effectiveAddress, memory);
                setFlagStatusLoad(X);
                break;
            }

            // load x register from absolute address + y register
            case INS_LDX_ABY: // testing complete
            {
                Word effectiveAddress = fetchAddressAbsolutePlusRegister(cycles, Y_REGISTER, memory);
                X = fetchByteFromAddress(cycles, effectiveAddress, memory);
                setFlagStatusLoad(X);
                break;
            }

            /**
             * LOAD Y REGISTER
             */

            // load Y register immediate
            case INS_LDY_IMM: // testing complete
            {
                Y = fetchNextByte(cycles, memory);
                setFlagStatusLoad(Y);
                break;
            }

            // load y register from zero page address
            case INS_LDY_ZPG: // testing complete
            {
                Word address = fetchAddressZeroPage(cycles, memory);
                Y = fetchByteFromAddress(cycles, address, memory);
                setFlagStatusLoad(Y);
                break;
            }

            // load y register from zero page address + x register
            case INS_LDY_ZPX: // testing complete
            {
                Word effectiveAddress = fetchAddressZeroPagePlusRegister(cycles, X_REGISTER, memory);
                Y = fetchByteFromAddress(cycles, effectiveAddress, memory);
                setFlagStatusLoad(Y);
                break;
            }

            // load y register from absolute address
            case INS_LDY_ABS: // testing complete
            {
                Word effectiveAddress = fetchAddressAbsolute(cycles, memory);
                Y = fetchByteFromAddress(cycles, effectiveAddress, memory);
                setFlagStatusLoad(Y);
                break;
            }

            // load y register from absolute address + x register
            case INS_LDY_ABX: // testing complete
            {
                Word effectiveAddress = fetchAddressAbsolutePlusRegister(cycles, X_REGISTER, memory);
                Y = fetchByteFromAddress(cycles, effectiveAddress, memory);
                setFlagStatusLoad(Y);
                break;
            }

            // error
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