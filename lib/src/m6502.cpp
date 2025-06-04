#include "m6502.h"

namespace m6502
{

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
            registerValue = X;
        else
            registerValue = Y;
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
            registerValue = X;
        else
            registerValue = Y;
        Word newAddress = absoluteAddress + registerValue;

        bool pageCrossed = (absoluteAddress & 0xFF00) != (newAddress & 0xFF00);
        if (pageCrossed)
            cycles--;
        return newAddress;
    }

    s32 CPU::fetchAddressIndexedIndirect(s32 &cycles, const Mem &memory)
    {
        Byte zeroPageAddress = readNextByte(cycles, memory);
        Byte xRegister = X;
        Word indexedAddress = (zeroPageAddress + xRegister) & 0xFF; // cycle is taken for addition here
        cycles--;
        Word effectiveAddress = readWordFromAddress(cycles, indexedAddress, memory);
        return effectiveAddress;
    }

    s32 CPU::fetchAddressIndirectIndexed(s32 &cycles, const Mem &memory, bool notStoreInstruction)
    {
        Byte zeroPageAddress = readNextByte(cycles, memory);
        Byte yRegister = Y;
        Word address = readWordFromAddress(cycles, zeroPageAddress, memory);
        Word effectiveAddress = address + yRegister; // no cycles taken adding here

        if (notStoreInstruction)
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

    // TODO: GET/SET flag helper methods

    void CPU::setFlagStatus_NZ(Byte value)
    {
        P.bits.Z = (value == 0) ? 1 : 0;
        P.bits.N = (value & 0x80) ? 1 : 0;
    }

    void CPU::setFlagStatus_BIT(Byte value)
    {
        Byte andResult = A & value;
        P.bits.Z = ((andResult & ZERO_FLAG_BIT) == 0) ? 1 : 0;
        P.bits.V = (value & OVERFLOW_FLAG_BIT) ? 1 : 0;
        P.bits.N = (value & NEGATIVE_FLAG_BIT) ? 1 : 0;
    }

    void CPU::setOverflowFlag(Byte regCopy, Byte operand, Byte result)
    {
    }

    void CPU::setFlagStatus_ADC(Word value,
                                Byte regCopy,
                                Byte operand)
    {
        Byte lowByte = value & 0xFF;

        setFlagStatus_NZ(lowByte); // n and z flags
        P.bits.C = value > 0xFF;   // c flag
        Byte aXORResult = regCopy ^ lowByte;
        Byte operXORResult = operand ^ lowByte;
        Byte signBit = aXORResult & operXORResult & 0x80;
        P.bits.V = (signBit != 0); // v flag
        if (P.bits.V)
            P.bits.C = P.bits.V;
    }

    void CPU::setFlagStatus_SBC(Word value,
                                Byte regCopy,
                                Byte operand)
    {
        Byte lowByte = value & 0xFF;

        setFlagStatus_NZ(lowByte); // n and z flags

        Word fullValue = operand + (P.bits.C ? 0 : 1);
        P.bits.C = (regCopy >= fullValue); // c flag
        Byte aXORResult = regCopy ^ operand;
        Byte operXORResult = regCopy ^ lowByte;
        Byte signBit = aXORResult & operXORResult & 0x80;
        P.bits.V = (signBit != 0); // v flag
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
            setFlagStatus_NZ(reg);
        };

        // and byte with a register
        auto andOperation = [&cycles, &memory, this](Word byteAddress, Byte &reg)
        {
            reg &= readByteFromAddress(cycles, byteAddress, memory);
            setFlagStatus_NZ(reg);
        };

        // xor bite with a register
        auto xorOperation = [&cycles, &memory, this](Word byteAddress, Byte &reg)
        {
            reg ^= readByteFromAddress(cycles, byteAddress, memory);
            setFlagStatus_NZ(reg);
        };

        // or bite with a register
        auto orOperation = [&cycles, &memory, this](Word byteAddress, Byte &reg)
        {
            reg |= readByteFromAddress(cycles, byteAddress, memory);
            setFlagStatus_NZ(reg);
        };

        // increment
        auto increment = [&cycles, &memory, this](Word address)
        {
            Byte value = readByteFromAddress(cycles, address, memory);
            value++;
            cycles--; // for incrementing
            writeByte(cycles, value, address, memory);
        };

        // decrement
        auto decrement = [&cycles, &memory, this](Word address)
        {
            Byte value = readByteFromAddress(cycles, address, memory);
            value--;
            cycles--; // for decrementing
            writeByte(cycles, value, address, memory);
        };

        // branch logic
        auto branch = [&cycles, &memory, this](bool flag, bool condition)
        {
            Byte operand = readNextByte(cycles, memory);
            s32 offset = (s8)operand;
            if (flag == condition)
            {
                Word pcCopy = PC;
                PC += offset;
                cycles--; // for PC arithmetic
                bool pageCrossed = (pcCopy & 0xFF00) != (PC & 0xFF00);
                if (pageCrossed)
                    cycles--;
            }
        };

        // add with carry
        auto addWithCarry = [&cycles, &memory, this](Word address)
        {
            Byte value = readByteFromAddress(cycles, address, memory);
            Byte aCopy = A;
            Word result = A + value + (P.bits.C ? 1 : 0);
            A = result & 0xFF; // only storing low byte of result
            setFlagStatus_ADC(result, aCopy, value);
        };

        auto subWithCarry = [&cycles, &memory, this](Word address)
        {
            Byte value = readByteFromAddress(cycles, address, memory);
            Byte aCopy = A;
            Word result = A - value - (P.bits.C ? 0 : 1);
            A = result & 0xFF; // only storing low byte of result
            setFlagStatus_SBC(result, aCopy, value);
        };

        const s32 cyclesRequested = cycles;
        while (cycles > 0)
        {
            Byte instruction = readNextByte(cycles, memory);
            std::cout << "Instruction: 0x" << std::hex << (int)instruction << std::endl;
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
                setFlagStatus_NZ(A);
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
                setFlagStatus_NZ(X);
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
                setFlagStatus_NZ(Y);
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
            case INS_JMP_ABS: // testing complete
            {
                Word address = readNextWord(cycles, memory);
                PC = address;
                break;
            }

            // jump to location from indirect address
            case INS_JMP_IND: // testing complete
            {
                Word address = readNextWord(cycles, memory);
                Word effectiveAddress = readWordFromAddress(cycles, address, memory);
                PC = effectiveAddress;
                break;
            }

            // jump to subroutine from absolute address
            case INS_JSR_ABS: // testing complete
            {
                Word address = readNextWord(cycles, memory);
                pushWordToStack(cycles, PC - 1, memory);
                PC = address;
                cycles--; // for arithmetic
                break;
            }

            // return from subroutine
            case INS_RTS: // testing complete
            {
                Word address = popWordFromStack(cycles, memory);
                PC = address + 1;
                cycles--; // for arithmetic
                break;
            }

            /**
             * STACK OPERATION INSTRUCTIONS
             */

            // transfer stack pointer to x register
            case INS_TSX: // testing complete
            {
                X = SP;
                cycles--; // for transfer of data
                setFlagStatus_NZ(X);
                break;
            }

            // transfer x register to stack pointer
            case INS_TXS: // testing complete
            {
                SP = X;
                cycles--; // for transfer of data
                break;
            }

            // push accumulator to stack
            case INS_PHA: // testing complete
            {
                pushByteToStack(cycles, A, memory);
                cycles--; // extra cycle
                break;
            }

            // push processor status to stack
            case INS_PHP: // testing complete
            {
                pushByteToStack(cycles, P.value, memory);
                cycles--; // extra cycle
                break;
            }

            // pull from stack to accumulator
            case INS_PLA: // testing complete
            {
                Byte value = popByteFromStack(cycles, memory);
                A = value;
                cycles--; // extra cycle
                setFlagStatus_NZ(A);
                break;
            }

            // pull processor from stack
            case INS_PLP: // testing complete
            {
                Byte value = popByteFromStack(cycles, memory);
                P.value = value;
                cycles--; // extra cycle
                break;
            }

            /**
             * LOGICAL INSTRUCTIONS
             */

            /**
             * AND OPERATIONS
             */

            // and immediate address
            case INS_AND_IMM: // testing complete
            {
                A &= readNextByte(cycles, memory);
                setFlagStatus_NZ(A);
                break;
            }

            // and zero page address
            case INS_AND_ZPG: // testing complete
            {
                Word address = fetchAddressZeroPage(cycles, memory);
                andOperation(address, A);
                break;
            }

            // and zero page + x register address
            case INS_AND_ZPX: // testing complete
            {
                Word address = fetchAddressZeroPagePlusRegister(cycles, X_REGISTER, memory);
                andOperation(address, A);
                break;
            }

            // and zero page absolute address
            case INS_AND_ABS: // testing complete
            {
                Word address = fetchAddressAbsolute(cycles, memory);
                andOperation(address, A);
                break;
            }

            // and zero page absolute + x register address
            case INS_AND_ABX: // testing complete
            {
                Word address = fetchAddressAbsolutePlusRegister(cycles, X_REGISTER, memory);
                andOperation(address, A);
                break;
            }

            // and zero page absolute + y register address
            case INS_AND_ABY: // testing complete
            {
                Word address = fetchAddressAbsolutePlusRegister(cycles, Y_REGISTER, memory);
                andOperation(address, A);
                break;
            }

            // and indexed indirect addressing
            case INS_AND_INX: // testing complete
            {
                Word address = fetchAddressIndexedIndirect(cycles, memory);
                andOperation(address, A);
                break;
            }

            // and indirect indexed addressing
            case INS_AND_INY: // testing complete
            {
                Word address = fetchAddressIndirectIndexed(cycles, memory, true);
                andOperation(address, A);
                break;
            }

            /**
             * EXCLUSIVE OR OPERATIONS
             */

            // or immediate address
            case INS_EOR_IMM: // testing complete
            {
                A ^= readNextByte(cycles, memory);
                setFlagStatus_NZ(A);
                break;
            }

            // or zero page address
            case INS_EOR_ZPG: // testing complete
            {
                Word address = fetchAddressZeroPage(cycles, memory);
                xorOperation(address, A);
                break;
            }

            // or zero page + x register address
            case INS_EOR_ZPX: // testing complete
            {
                Word address = fetchAddressZeroPagePlusRegister(cycles, X_REGISTER, memory);
                xorOperation(address, A);
                break;
            }

            // or zero page absolute address
            case INS_EOR_ABS: // testing complete
            {
                Word address = fetchAddressAbsolute(cycles, memory);
                xorOperation(address, A);
                break;
            }

            // or zero page absolute + x register address
            case INS_EOR_ABX: // testing complete
            {
                Word address = fetchAddressAbsolutePlusRegister(cycles, X_REGISTER, memory);
                xorOperation(address, A);
                break;
            }

            // or zero page absolute + y register address
            case INS_EOR_ABY: // testing complete
            {
                Word address = fetchAddressAbsolutePlusRegister(cycles, Y_REGISTER, memory);
                xorOperation(address, A);
                break;
            }

            // or indexed indirect addressing
            case INS_EOR_INX: // testing complete
            {
                Word address = fetchAddressIndexedIndirect(cycles, memory);
                xorOperation(address, A);
                break;
            }

            // or indirect indexed addressing
            case INS_EOR_INY: // testing complete
            {
                Word address = fetchAddressIndirectIndexed(cycles, memory, true);
                xorOperation(address, A);
                break;
            }

            /**
             * INCLUSIVE OR OPERATIONS
             */

            // or immediate address
            case INS_ORA_IMM: // testing complete
            {
                A |= readNextByte(cycles, memory);
                setFlagStatus_NZ(A);
                break;
            }

            // or zero page address
            case INS_ORA_ZPG: // testing complete
            {
                Word address = fetchAddressZeroPage(cycles, memory);
                orOperation(address, A);
                break;
            }

            // or zero page + x register address
            case INS_ORA_ZPX: // testing complete
            {
                Word address = fetchAddressZeroPagePlusRegister(cycles, X_REGISTER, memory);
                orOperation(address, A);
                break;
            }

            // or zero page absolute address
            case INS_ORA_ABS: // testing complete
            {
                Word address = fetchAddressAbsolute(cycles, memory);
                orOperation(address, A);
                break;
            }

            // or zero page absolute + x register address
            case INS_ORA_ABX: // testing complete
            {
                Word address = fetchAddressAbsolutePlusRegister(cycles, X_REGISTER, memory);
                orOperation(address, A);
                break;
            }

            // or zero page absolute + y register address
            case INS_ORA_ABY: // testing complete
            {
                Word address = fetchAddressAbsolutePlusRegister(cycles, Y_REGISTER, memory);
                orOperation(address, A);
                break;
            }

            // or indexed indirect addressing
            case INS_ORA_INX: // testing complete
            {
                Word address = fetchAddressIndexedIndirect(cycles, memory);
                orOperation(address, A);
                break;
            }

            // or indirect indexed addressing
            case INS_ORA_INY: // testing complete
            {
                Word address = fetchAddressIndirectIndexed(cycles, memory, true);
                orOperation(address, A);
                break;
            }

            /**
             * BIT TEST OPERATIONS
             */

            // bit test zero page addressing
            case INS_BIT_ZPG: // testing complete
            {
                Word address = fetchAddressZeroPage(cycles, memory);
                Byte value = readByteFromAddress(cycles, address, memory);
                setFlagStatus_BIT(value);
                break;
            }

            // bit test absolute addressing
            case INS_BIT_ABS: // testing complete
            {
                Word address = fetchAddressAbsolute(cycles, memory);
                Byte value = readByteFromAddress(cycles, address, memory);
                setFlagStatus_BIT(value);
                break;
            }

            /**
             * REGISTER TRANSFER INSTRUCTIONS
             */

            // transfer accumulator to x register
            case INS_TAX: // testing complete
            {
                X = A;
                cycles--;
                setFlagStatus_NZ(X);
                break;
            }

            // transfer accumulator to y register
            case INS_TAY: // testing complete
            {
                Y = A;
                cycles--;
                setFlagStatus_NZ(Y);
                break;
            }

            // transfer x register to accumulator
            case INS_TXA: // testing complete
            {
                A = X;
                cycles--;
                setFlagStatus_NZ(A);
                break;
            }

            // transfer y register to accumulator
            case INS_TYA: // testing complete
            {
                A = Y;
                cycles--;
                setFlagStatus_NZ(A);
                break;
            }

            /**
             *  INCREMENT AND DECREMENT INSTRUCTIONS
             */

            /**
             * INCREMENT INSTRUCTIONS
             */

            // increment memory at zero page address
            case INS_INC_ZPG: // testing complete
            {
                Byte address = fetchAddressZeroPage(cycles, memory);
                increment(address);
                break;
            }

            // increment memory at zero page + x register address
            case INS_INC_ZPX: // testing complete
            {
                Byte address = fetchAddressZeroPagePlusRegister(cycles, X_REGISTER, memory);
                increment(address);
                break;
            }

            // increment memory at absolute address
            case INS_INC_ABS: // testing complete
            {
                Word address = fetchAddressAbsolute(cycles, memory);
                increment(address);
                break;
            }

            // increment memory at absolute + x register address
            case INS_INC_ABX:
            {
                Word address = fetchAddressAbsolutePlusRegister(cycles, X_REGISTER, memory);
                cycles--; // for arithmetic
                increment(address);
                break;
            }

            /**
             * INCREMENT REGISTERS
             */

            // increment x register
            case INS_INX: // testing complete
            {
                X++;
                cycles--;
                setFlagStatus_NZ(X);
                break;
            }

            // increment y register
            case INS_INY: // testing complete
            {
                Y++;
                cycles--;
                setFlagStatus_NZ(Y);
                break;
            }

            /**
             * DECREMENT INSTRUCTIONS
             */

            // decrement memory at zero page address
            case INS_DEC_ZPG: // testing complete
            {
                Byte address = fetchAddressZeroPage(cycles, memory);
                decrement(address);
                break;
            }

            // decrement memory at zero page + x register address
            case INS_DEC_ZPX: // testing complete
            {
                Byte address = fetchAddressZeroPagePlusRegister(cycles, X_REGISTER, memory);
                decrement(address);
                break;
            }

            // decrement memory at absolute address
            case INS_DEC_ABS: // testing complete
            {
                Word address = fetchAddressAbsolute(cycles, memory);
                decrement(address);
                break;
            }

            // decrement memory at absolute + x register address
            case INS_DEC_ABX:
            {
                Word address = fetchAddressAbsolutePlusRegister(cycles, X_REGISTER, memory);
                cycles--; // for arithmetic
                decrement(address);
                break;
            }

            /**
             *  DECREMENT REGISTERS
             */

            // decrement x register
            case INS_DEX:
            {
                X--;
                cycles--;
                setFlagStatus_NZ(X);
                break;
            }

            // decrement y register
            case INS_DEY:
            {
                Y--;
                cycles--;
                setFlagStatus_NZ(Y);
                break;
            }

            /**
             * BRANCH INSTRUCTIONS
             */

            // branch if carry flag clear
            case INS_BCC: // testing complete
            {
                branch(P.bits.C, false);
                break;
            }

            // branch if carry flag set
            case INS_BCS: // testing complete
            {
                branch(P.bits.C, true);
                break;
            }

            // branch if zero flag clear
            case INS_BNE: // testing complete
            {
                branch(P.bits.Z, false);
                break;
            }

            // branch if zero flag set
            case INS_BEQ: // testing complete
            {
                branch(P.bits.Z, true);
                break;
            }

            // branch if negative flag clear
            case INS_BPL: // testing complete
            {
                branch(P.bits.N, false);
                break;
            }

            // branch if negative flag set
            case INS_BMI: // testing complete
            {
                branch(P.bits.N, true);
                break;
            }

            // branch if overflow flag clear
            case INS_BVC: // testing complete
            {
                branch(P.bits.V, false);
                break;
            }

            // branch if overflow flag set
            case INS_BVS: // testing complete
            {
                branch(P.bits.V, true);
                break;
            }

            /**
             * STATUS FLAG CHANGE INSTRUCTIONS
             */

            // clear carry flag
            case INS_CLC: // testing complete
            {
                P.bits.C = false;
                cycles--;
                break;
            }

            // clear decimal flag
            case INS_CLD: // testing complete
            {
                P.bits.D = false;
                cycles--;
                break;
            }

            // clear interrupt flag
            case INS_CLI: // testing complete
            {
                P.bits.I = false;
                cycles--;
                break;
            }

            // clear overflow flag
            case INS_CLV: // testing complete
            {
                P.bits.V = false;
                cycles--;
                break;
            }

            // set carry flag
            case INS_SEC: // testing complete
            {
                P.bits.C = true;
                cycles--;
                break;
            }

            // set decimal flag
            case INS_SED: // testing complete
            {
                P.bits.D = true;
                cycles--;
                break;
            }

            // set interrupt flag
            case INS_SEI: // testing complete
            {
                P.bits.I = true;
                cycles--;
                break;
            }

            /**
             * ARITHMETIC INSTRUCTIONS
             */

            /**
             * ADD WITH CARRY
             */

            // add with carry immediate addressing
            case INS_ADC_IMM: // testing complete
            {
                Byte value = readNextByte(cycles, memory);
                Byte aCopy = A;
                Word result = A + value + (P.bits.C ? 1 : 0);
                A = result & 0xFF; // only storing low byte of result
                setFlagStatus_ADC(result, aCopy, value);
                break;
            }

            // add with carry zero page addressing
            case INS_ADC_ZPG: // testing complete
            {
                Word address = fetchAddressZeroPage(cycles, memory);
                addWithCarry(address);
                break;
            }

            // add with carry zero page + x register addressing
            case INS_ADC_ZPX: // testing complete
            {
                Word address = fetchAddressZeroPagePlusRegister(cycles, X_REGISTER, memory);
                addWithCarry(address);
                break;
            }

            // add with carry absolute addressing
            case INS_ADC_ABS: // testing complete
            {
                Word address = fetchAddressAbsolute(cycles, memory);
                addWithCarry(address);
                break;
            }

            // add with carry absolute + x register addressing
            case INS_ADC_ABX: // testing complete
            {
                Word address = fetchAddressAbsolutePlusRegister(cycles, X_REGISTER, memory);
                addWithCarry(address);
                break;
            }

            // add with carry absolute + y register addressing
            case INS_ADC_ABY: // testing complete
            {
                Word address = fetchAddressAbsolutePlusRegister(cycles, Y_REGISTER, memory);
                addWithCarry(address);
                break;
            }

            // add with carry indexed indirect addressing
            case INS_ADC_INX: // testing complete
            {
                Word address = fetchAddressIndexedIndirect(cycles, memory);
                addWithCarry(address);
                break;
            }

            // add with carry indirect indexed addressing
            case INS_ADC_INY: // testing complete
            {
                Word address = fetchAddressIndirectIndexed(cycles, memory, true);
                addWithCarry(address);
                break;
            }

            /**
             * SUBTRACT WITH CARRY
             */

            // subtract with carry immediate addressing
            case INS_SBC_IMM: // testing complete
            {
                Byte value = readNextByte(cycles, memory);
                Byte aCopy = A;
                Word result = A - value - (P.bits.C ? 0 : 1);
                A = result & 0xFF; // only storing low byte of result
                setFlagStatus_SBC(result, aCopy, value);
                break;
            }

            // subtract with carry zero page addressing
            case INS_SBC_ZPG: // testing complete
            {
                Word address = fetchAddressZeroPage(cycles, memory);
                subWithCarry(address);
                break;
            }

            // subtract with carry zero page + x register addressing
            case INS_SBC_ZPX: // testing complete
            {
                Word address = fetchAddressZeroPagePlusRegister(cycles, X_REGISTER, memory);
                subWithCarry(address);
                break;
            }

            // subtract with carry absolute addressing
            case INS_SBC_ABS: // testing complete
            {
                Word address = fetchAddressAbsolute(cycles, memory);
                subWithCarry(address);
                break;
            }

            // subtract with carry absolute + x register addressing
            case INS_SBC_ABX: // testing complete
            {
                Word address = fetchAddressAbsolutePlusRegister(cycles, X_REGISTER, memory);
                subWithCarry(address);
                break;
            }

            // subtract with carry absolute + y register addressing
            case INS_SBC_ABY: // testing complete
            {
                Word address = fetchAddressAbsolutePlusRegister(cycles, Y_REGISTER, memory);
                subWithCarry(address);
                break;
            }

            // subtract with carry indexed indirect addressing
            case INS_SBC_INX: // testing complete
            {
                Word address = fetchAddressIndexedIndirect(cycles, memory);
                subWithCarry(address);
                break;
            }

            // subtract with carry indirect indexed addressing
            case INS_SBC_INY: // testing complete
            {
                Word address = fetchAddressIndirectIndexed(cycles, memory, true);
                subWithCarry(address);
                break;
            }

            /**
             * COMPARE INSTRUCTIONS
             */

            // compare accumulator immediate addressing
            case INS_CMP_IMM:
            {
                break;
            }

            // compare accumulator zero page addressing
            case INS_CMP_ZPG:
            {
                break;
            }

            // compare accumulator zero page + x register addressing
            case INS_CMP_ZPX:
            {
                break;
            }

            // compare accumulator absolute addressing
            case INS_CMP_ABS:
            {
                break;
            }

            // compare accumulator absolute + x register addressing
            case INS_CMP_ABX:
            {
                break;
            }

            // compare accumulator absolute + y register addressing
            case INS_CMP_ABY:
            {
                break;
            }

            // compare accumulator indirect indexed addressing
            case INS_CMP_INX:
            {
                break;
            }

            // compare accumulator indexed indirect addressing
            case INS_CMP_INY:
            {
                break;
            }

            // compare x register immediate addressing
            case INS_CPX_IMM:
            {
                break;
            }

            // compare x register zero page addressing
            case INS_CPX_ZPG:
            {
                break;
            }

            // compare x register absolute addressing
            case INS_CPX_ABS:
            {
                break;
            }

            // compare y register immediate addressing
            case INS_CPY_IMM:
            {
                break;
            }

            // compare y register zero page addressing
            case INS_CPY_ZPG:
            {
                break;
            }

            // compare y register absolute addressing
            case INS_CPY_ABS:
            {
                break;
            }

            /**
             * SYSTEM FUNCTION INSTRUCTIONS
             */

            // TODO: fix after other commands implemented?
            case INS_BRK:
            {
                Byte signatureByte = readNextByte(cycles, memory);
                pushByteToStack(cycles, PC >> 8, memory);
                pushByteToStack(cycles, PC & 0xFF, memory);
                P.bits.B = true;
                pushByteToStack(cycles, P.value, memory);
                P.bits.I = true;
                Byte irqVectorLow = readByteFromAddress(cycles, IRQ_VECTOR_LOW, memory);
                Byte irqVectorHigh = readByteFromAddress(cycles, IRQ_VECTOR_HIGH, memory);
                Word irqVector = (irqVectorHigh << 8) | irqVectorLow;
                PC = irqVector;
                break;
            }

            case INS_NOP:
            {
                cycles--;
                break;
            }

            // error
            default:
            {
                using namespace std;
                cout << "Instruction not handled: 0x" << hex << (int)instruction << endl;
                PC--;
                throw runtime_error("Unhandled instruction.");
            }
            }
        }

        const s32 cyclesUsed = cyclesRequested - cycles;
        return cyclesUsed;
    }

    Word CPU::loadProgram(Byte *program, u32 numBytes, Mem &memory)
    {
        if (!program || numBytes <= 2)
        {
            throw std::invalid_argument("Invalid program or size too small.");
        }

        u32 programPtr = 0;
        // first word is always address where program is stored (e.g '00 80' @ '$8000')
        const Word loadAddress = program[programPtr] | (program[programPtr + 1] << 8);
        programPtr += 2;

        const u32 programSize = numBytes - 2;
        for (u32 i = 0; i < programSize; i++)
        {
            memory[loadAddress + i] = program[programPtr++];
        }
        return loadAddress;
    }
}