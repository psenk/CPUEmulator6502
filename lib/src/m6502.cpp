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

    s32 CPU::fetchAddressZeroPage(s32 &cycles, const Mem &memory)
    {
        Word zeroPageAddress = readNextByte(cycles, memory);
        return zeroPageAddress;
    }

    s32 CPU::fetchAddressZeroPagePlusRegister(s32 &cycles, RegisterType reg, const Mem &memory)
    {
        Byte zeroPageAddress = readNextByte(cycles, memory);
        Byte registerValue;
        if (reg == X_REGISTER)
        {
            registerValue = readByteFromXRegister();
        }
        else
        {
            registerValue = readByteFromYRegister();
        }
        Byte effectiveAddress = zeroPageAddress + registerValue; // cycle is taken for addition here
        cycles--;
        return effectiveAddress;
    }

    s32 CPU::fetchAddressAbsolute(s32 &cycles, const Mem &memory)
    {
        Word absoluteAddress = readNextWord(cycles, memory);
        return absoluteAddress;
    }

    s32 CPU::fetchAddressAbsolutePlusRegister(s32 &cycles, RegisterType reg, const Mem &memory)
    {
        Word absoluteAddress = readNextWord(cycles, memory);
        Byte registerValue;
        if (reg == X_REGISTER)
        {
            registerValue = readByteFromXRegister();
        }
        else
        {
            registerValue = readByteFromYRegister();
        }
        Word newAddress = absoluteAddress + registerValue;

        bool pageCrossed = (absoluteAddress & 0xFF00) != (newAddress & 0xFF00);
        if (pageCrossed)
        {
            cycles--;
        }
        return newAddress;
    }

    s32 CPU::fetchAddressIndexedIndirect(s32 &cycles, const Mem &memory)
    {
        Byte zeroPageAddress = readNextByte(cycles, memory);
        Byte xRegister = readByteFromXRegister();
        Word indexedAddress = (zeroPageAddress + xRegister) & 0xFF; // cycle is taken for addition here
        cycles--;
        Word effectiveAddress = readWordFromAddress(cycles, indexedAddress, memory);
        return effectiveAddress;
    }

    s32 CPU::fetchAddressIndirectIndexed(s32 &cycles, const Mem &memory, bool extraCycle)
    {
        Byte zeroPageAddress = readNextByte(cycles, memory);
        Byte yRegister = readByteFromYRegister();
        Word address = readWordFromAddress(cycles, zeroPageAddress, memory);
        Word effectiveAddress = address + yRegister; // no cycles taken adding here

        if (extraCycle)
        {
            bool pageCrossed = (address & 0xFF00) != (effectiveAddress & 0xFF00);
            if (pageCrossed)
            {
                cycles--;
            }
            return effectiveAddress;
        }
        cycles--;
        return effectiveAddress;
    }

    /**
     * SET FLAGS
     */

    void CPU::setFlagStatusLoad(Byte value)
    {
        P.bits.Z = (value == 0) ? 1 : 0;
        P.bits.N = (value & 0x80) ? 1 : 0;
    }

    /**
     * CPU EXECUTE FUNCTION
     */

    s32 CPU::execute(s32 cycles, Mem &memory)
    {

        // load register from a memory address
        auto loadRegister = [&cycles, &memory, this](Word address, Byte &reg)
        {
            reg = readByteFromAddress(cycles, address, memory);
            setFlagStatusLoad(reg);
        };

        const s32 cyclesRequested = cycles;
        while (cycles > 0)
        {
            Byte instruction = readNextByte(cycles, memory);
            switch (instruction)
            {

            /**
             * LOAD INSTRUCTIONS
             */

            /**
             * ACCUMULATOR
             */

            // load accumulator immediate
            case INS_LDA_IMM: // testing complete
            {
                A = readNextByte(cycles, memory);
                setFlagStatusLoad(A);
                break;
            }

            // load accumulator from zero page address
            case INS_LDA_ZPG: // testing complete
            {
                Word address = fetchAddressZeroPage(cycles, memory);
                loadRegister(address, A);
                break;
            }

            // load accumulator from zero page address + x register
            case INS_LDA_ZPX: // testing complete
            {
                Word address = fetchAddressZeroPagePlusRegister(cycles, X_REGISTER, memory);
                loadRegister(address, A);
                break;
            }

            // load accumulator from absolute address
            case INS_LDA_ABS: // testing complete
            {
                Word address = fetchAddressAbsolute(cycles, memory);
                loadRegister(address, A);
                break;
            }

            // load accumulator from absolute address + x register
            case INS_LDA_ABX: // testing complete
            {
                Word address = fetchAddressAbsolutePlusRegister(cycles, X_REGISTER, memory);
                loadRegister(address, A);
                break;
            }

            // load accumulator from absolute address + y register
            case INS_LDA_ABY: // testing complete
            {
                Word address = fetchAddressAbsolutePlusRegister(cycles, Y_REGISTER, memory);
                loadRegister(address, A);
                break;
            }

            // load accumulator with indexed indirect addressing + x register
            case INS_LDA_INX: // testing complete
            {
                Word address = fetchAddressIndexedIndirect(cycles, memory);
                loadRegister(address, A);
                break;
            }

            // load accumulator with indirect indexed addressing + y register
            case INS_LDA_INY: // testing complete
            {
                Word address = fetchAddressIndirectIndexed(cycles, memory, true);
                loadRegister(address, A);
                break;
            }

            /**
             * X REGISTER
             */

            // load x register immediate
            case INS_LDX_IMM: // testing complete
            {
                X = readNextByte(cycles, memory);
                setFlagStatusLoad(X);
                break;
            }

            // load x register from zero page address
            case INS_LDX_ZPG: // testing complete
            {
                Word address = fetchAddressZeroPage(cycles, memory);
                loadRegister(address, X);
                break;
            }

            // load x register from zero page + y register address
            case INS_LDX_ZPY: // testing complete
            {
                Word address = fetchAddressZeroPagePlusRegister(cycles, Y_REGISTER, memory);
                loadRegister(address, X);
                break;
            }

            // load x register from absolute address
            case INS_LDX_ABS: // testing complete
            {
                Word address = fetchAddressAbsolute(cycles, memory);
                loadRegister(address, X);
                break;
            }

            // load x register from absolute + y register address
            case INS_LDX_ABY: // testing complete
            {
                Word address = fetchAddressAbsolutePlusRegister(cycles, Y_REGISTER, memory);
                loadRegister(address, X);
                break;
            }

            /**
             * Y REGISTER
             */

            // load Y register immediate
            case INS_LDY_IMM: // testing complete
            {
                Y = readNextByte(cycles, memory);
                setFlagStatusLoad(Y);
                break;
            }

            // load y register from zero page address
            case INS_LDY_ZPG: // testing complete
            {
                Word address = fetchAddressZeroPage(cycles, memory);
                loadRegister(address, Y);
                break;
            }

            // load y register from zero page + x register address
            case INS_LDY_ZPX: // testing complete
            {
                Word address = fetchAddressZeroPagePlusRegister(cycles, X_REGISTER, memory);
                loadRegister(address, Y);
                break;
            }

            // load y register from absolute address
            case INS_LDY_ABS: // testing complete
            {
                Word address = fetchAddressAbsolute(cycles, memory);
                loadRegister(address, Y);
                break;
            }

            // load y register from absolute + x register address
            case INS_LDY_ABX: // testing complete
            {
                Word address = fetchAddressAbsolutePlusRegister(cycles, X_REGISTER, memory);
                loadRegister(address, Y);
                break;
            }

            /**
             * STORE INSTRUCTIONS
             */

            /**
             * ACCUMULATOR
             */

            // store accumulator value to memory from zero page address
            case INS_STA_ZPG: // testing complete
            {
                Word address = fetchAddressZeroPage(cycles, memory);
                writeByte(cycles, A, address, memory);
                break;
            }

            // store accumulator value to memory from zero page + x register address
            case INS_STA_ZPX: // testing complete
            {
                Word address = fetchAddressZeroPagePlusRegister(cycles, X_REGISTER, memory);
                writeByte(cycles, A, address, memory);
                break;
            }

            // store accumulator value to memory from absolute address
            case INS_STA_ABS: // testing complete
            {
                Word address = fetchAddressAbsolute(cycles, memory);
                writeByte(cycles, A, address, memory);
                break;
            }

            // store accumulator to memory from absolute + x register address
            case INS_STA_ABX: // testing complete
            {
                Word address = fetchAddressAbsolutePlusRegister(cycles, X_REGISTER, memory);
                cycles--; // for arithmetic
                writeByte(cycles, A, address, memory);
                break;
            }

            // store accumulator to memory from absolute + y register address
            case INS_STA_ABY: // testing complete
            {
                Word address = fetchAddressAbsolutePlusRegister(cycles, Y_REGISTER, memory);
                cycles--; // for arithmetic
                writeByte(cycles, A, address, memory);
                break;
            }

            // store accumulator to memory from indexed indirect + x register address
            case INS_STA_INX: // testing complete
            {
                Word address = fetchAddressIndexedIndirect(cycles, memory);
                writeByte(cycles, A, address, memory);
                break;
            }

            // store accumulator to memeory from indirect indexed + y register address
            case INS_STA_INY: // testing complete
            {
                Word address = fetchAddressIndirectIndexed(cycles, memory, false);
                writeByte(cycles, A, address, memory);
                break;
            }

            /**
             * X REGISTER
             */

            // store x register value to memory from zero page address
            case INS_STX_ZPG: // testing complete
            {
                Word address = fetchAddressZeroPage(cycles, memory);
                writeByte(cycles, X, address, memory);
                break;
            }

            // store x register value to memory from zero page + x register address
            case INS_STX_ZPY: // testing complete
            {
                Word address = fetchAddressZeroPagePlusRegister(cycles, Y_REGISTER, memory);
                writeByte(cycles, X, address, memory);
                break;
            }

            // store x register value to memory from absolute address
            case INS_STX_ABS: // testing complete
            {
                Word address = fetchAddressAbsolute(cycles, memory);
                writeByte(cycles, X, address, memory);
                break;
            }

            /**
             * Y REGISTER
             */

            // store y register to memory from zero page address
            case INS_STY_ZPG: // testing complete
            {
                Word address = fetchAddressZeroPage(cycles, memory);
                writeByte(cycles, Y, address, memory);
                break;
            }

            // store y register to memory from zero page + x register address
            case INS_STY_ZPX: // testing complete
            {
                Word address = fetchAddressZeroPagePlusRegister(cycles, X_REGISTER, memory);
                writeByte(cycles, Y, address, memory);
                break;
            }

            // store y register value to memory from absolute address
            case INS_STY_ABS: // testing complete
            {
                Word address = fetchAddressAbsolute(cycles, memory);
                writeByte(cycles, Y, address, memory);
                break;
            }

            /**
             * JUMP AND CALL INSTRUCTIONS
             */

            // jump to location from absolute address
            case INS_JMP_ABS:
            {
                Word address = readNextWord(cycles, memory);
                PC = address;
                break;
            }

            // jump to location from indirect address
            case INS_JMP_IND:
            {
                Word address = readNextWord(cycles, memory);
                Word effectiveAddress = readWordFromAddress(cycles, address, memory);
                PC = effectiveAddress;
                break;
            }

            // jump to subroutine from absolute address
            case INS_JSR_ABS:
            {
                Word address = readNextWord(cycles, memory);
                pushWordToStack(cycles, PC - 1, memory);
                PC = address;
                cycles--; // for arithmetic
                break;
            }

            // return from subroutine
            case INS_RTS:
            {
                Word address = popWordFromStack(cycles, memory);
                PC = address + 1;
                cycles--; // for arithmetic
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