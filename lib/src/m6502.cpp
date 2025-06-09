#include "m6502.h"

namespace m6502
{

    /**
     * ADDRESSING MODES
     */

    s_int32 CPU::fetchAddressZeroPage(s_int32 &cycles, const Mem &memory)
    {
        const Byte zeroPageAddress = readNextByte(cycles, memory);
        return zeroPageAddress;
    }

    s_int32 CPU::fetchAddressZeroPagePlusRegister(s_int32 &cycles, RegisterType reg, const Mem &memory)
    {
        const Byte zeroPageAddress = readNextByte(cycles, memory);
        Byte registerValue;
        if (reg == X_REGISTER)
            registerValue = X;
        else
            registerValue = Y;
        const Byte effectiveAddress = zeroPageAddress + registerValue; // cycle is taken for addition here
        cycles--;
        return effectiveAddress;
    }

    s_int32 CPU::fetchAddressAbsolute(s_int32 &cycles, const Mem &memory)
    {
        const Word absoluteAddress = readNextWord(cycles, memory);
        return absoluteAddress;
    }

    s_int32 CPU::fetchAddressAbsolutePlusRegister(s_int32 &cycles, RegisterType reg, const Mem &memory)
    {
        const Word absoluteAddress = readNextWord(cycles, memory);
        Byte registerValue;
        if (reg == X_REGISTER)
            registerValue = X;
        else
            registerValue = Y;
        const Word newAddress = absoluteAddress + registerValue;

        const bool pageCrossed = (absoluteAddress & 0xFF00) != (newAddress & 0xFF00);
        if (pageCrossed)
            cycles--;
        return newAddress;
    }

    s_int32 CPU::fetchAddressIndexedIndirect(s_int32 &cycles, const Mem &memory)
    {
        const Byte zeroPageAddress = readNextByte(cycles, memory);
        const Word indexedAddress = (zeroPageAddress + X) & 0xFF; // cycle is taken for addition here
        cycles--;
        const Word effectiveAddress = readWordFromAddress(cycles, indexedAddress, memory);
        return effectiveAddress;
    }

    s_int32 CPU::fetchAddressIndirectIndexed(s_int32 &cycles, const Mem &memory, bool notStoreInstruction)
    {
        const Byte zeroPageAddress = readNextByte(cycles, memory);
        const Word address = readWordFromAddress(cycles, zeroPageAddress, memory);
        const Word effectiveAddress = address + Y; // no cycles taken adding here

        if (notStoreInstruction)
        {
            const bool pageCrossed = (address & 0xFF00) != (effectiveAddress & 0xFF00);
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

    void CPU::setFlagStatus_NZ(Byte value)
    {
        const bool zFlag = (value == 0) ? 1 : 0;
        setZeroFlag(zFlag);

        const bool nFlag = (value & NEGATIVE_FLAG_BIT) ? 1 : 0;
        setNegativeFlag(nFlag);
    }

    void CPU::setFlagStatus_BIT(Byte value)
    {
        Byte andResult = A & value;

        const bool zFlag = ((andResult & ZERO_FLAG_BIT) == 0) ? 1 : 0;
        setZeroFlag(zFlag);

        const bool vFlag = (value & OVERFLOW_FLAG_BIT) ? 1 : 0;
        setOverflowFlag(vFlag);

        const bool nFlag = (value & NEGATIVE_FLAG_BIT) ? 1 : 0;
        setNegativeFlag(nFlag);
    }

    void CPU::setFlagStatus_ADC(Word value,
                                Byte regCopy,
                                Byte operand)
    {
        const Byte lowByte = value & 0xFF;

        setFlagStatus_NZ(lowByte);

        const bool cFlag = value > 0xFF;
        setCarryFlag(cFlag);

        const Byte aXORResult = regCopy ^ lowByte;
        const Byte operXORResult = operand ^ lowByte;
        const Byte signBit = aXORResult & operXORResult & 0x80;
        const bool vFlag = (signBit != 0);
        setOverflowFlag(vFlag);

        if (vFlag)
            setCarryFlag(vFlag);
    }

    void CPU::setFlagStatus_SBC(Word value,
                                Byte regCopy,
                                Byte operand)
    {
        const Byte lowByte = value & 0xFF;

        setFlagStatus_NZ(lowByte);

        const bool currentCFlag = getCarryFlag();
        const Word fullValue = operand + (currentCFlag ? 0 : 1);
        const bool cFlag = (regCopy >= fullValue);
        setCarryFlag(cFlag);

        const Byte aXORResult = regCopy ^ operand;
        const Byte operXORResult = regCopy ^ lowByte;
        const Byte signBit = aXORResult & operXORResult & 0x80;
        const bool vFlag = (signBit != 0);
        setOverflowFlag(vFlag);
    }

    void CPU::setFlagStatus_CMP(Byte operand,
                                const Byte &reg)
    {
        const Byte result = reg - operand;
        const bool cFlag = (reg >= operand);
        setCarryFlag(cFlag);

        const bool zFlag = (reg == operand);
        setZeroFlag(zFlag);

        const bool nFlag = (result & NEGATIVE_FLAG_BIT) ? 1 : 0;
        setNegativeFlag(nFlag);
    }

    void CPU::setFlagStatus_Arithmetic(Byte oldValue,
                                       Byte newValue,
                                       bool left = false)
    {
        const bool cFlag = oldValue & (left ? 0x80 : 0x01);
        setCarryFlag(cFlag);

        const bool zFlag = A == 0 ? 1 : 0;
        setZeroFlag(zFlag);

        const bool nFlag = newValue & 0x80;
        setNegativeFlag(nFlag);
    }

    /**
     * CPU EXECUTE FUNCTION
     */

    s_int32 CPU::execute(s_int32 cycles, Mem &memory)
    {

        // load register from a memory address
        auto loadRegister = [&cycles, &memory, this](Word address, Byte &reg)
        {
            reg = readByteFromAddress(cycles, address, memory);
            setFlagStatus_NZ(reg);
        };

        // and byte with a register
        auto andOperation = [&cycles, &memory, this](Word address, Byte &reg)
        {
            reg &= readByteFromAddress(cycles, address, memory);
            setFlagStatus_NZ(reg);
        };

        // xor bite with a register
        auto xorOperation = [&cycles, &memory, this](Word address, Byte &reg)
        {
            reg ^= readByteFromAddress(cycles, address, memory);
            setFlagStatus_NZ(reg);
        };

        // or bite with a register
        auto orOperation = [&cycles, &memory, this](Word address, Byte &reg)
        {
            reg |= readByteFromAddress(cycles, address, memory);
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
        auto branch = [&cycles, &memory, this](bool flag, bool flagSet)
        {
            const Byte operand = readNextByte(cycles, memory);
            const s_int32 offset = (sByte)operand;
            if (flag == flagSet)
            {
                const Word pcCopy = PC;
                PC += offset;
                cycles--; // for PC arithmetic
                const bool pageCrossed = (pcCopy & 0xFF00) != (PC & 0xFF00);
                if (pageCrossed)
                    cycles--;
            }
        };

        // add with carry
        auto addWithCarry = [&cycles, &memory, this](Word address)
        {
            const Byte value = readByteFromAddress(cycles, address, memory);
            const Byte aCopy = A;
            const bool cFlag = getCarryFlag();
            const Word result = A + value + (cFlag ? 1 : 0);
            A = result & 0xFF; // only storing low byte of result
            setFlagStatus_ADC(result, aCopy, value);
        };

        // subtract with carry
        auto subWithCarry = [&cycles, &memory, this](Word address)
        {
            const Byte value = readByteFromAddress(cycles, address, memory);
            const Byte aCopy = A;
            const bool cFlag = getCarryFlag();
            const Word result = A - value - (cFlag ? 0 : 1);
            A = result & 0xFF; // only storing low byte of result
            setFlagStatus_SBC(result, aCopy, value);
        };

        // bit shift left/right
        auto bitShift = [&cycles, &memory, this](Word address, bool left = false)
        {
            const Byte oldValue = readByteFromAddress(cycles, address, memory);
            const Byte newValue = left ? oldValue << 1 : oldValue >> 1;
            cycles--;
            writeByte(cycles, newValue, address, memory);
            if (left)
                setFlagStatus_Arithmetic(oldValue, newValue, true);
            else
                setFlagStatus_Arithmetic(oldValue, newValue);
        };

        // bit rotate left/right
        auto bitRotate = [&cycles, &memory, this](Word address, bool left = false)
        {
            const Byte oldValue = readByteFromAddress(cycles, address, memory);
            Byte newValue = left ? oldValue << 1 : oldValue >> 1;
            newValue |= (P.bits.C << 7);
            cycles--;
            writeByte(cycles, newValue, address, memory);
            if (left)
                setFlagStatus_Arithmetic(oldValue, newValue, true);
            else
                setFlagStatus_Arithmetic(oldValue, newValue);
        };

        const s_int32 cyclesRequested = cycles;
        while (cycles > 0)
        {
            const Byte instruction = readNextByte(cycles, memory);
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
                const Byte address = fetchAddressZeroPage(cycles, memory);
                loadRegister(address, A);
                break;
            }

            // load accumulator from zero page address + x register
            case INS_LDA_ZPX: // testing complete
            {
                const Byte address = fetchAddressZeroPagePlusRegister(cycles, X_REGISTER, memory);
                loadRegister(address, A);
                break;
            }

            // load accumulator from absolute address
            case INS_LDA_ABS: // testing complete
            {
                const Word address = fetchAddressAbsolute(cycles, memory);
                loadRegister(address, A);
                break;
            }

            // load accumulator from absolute address + x register
            case INS_LDA_ABX: // testing complete
            {
                const Word address = fetchAddressAbsolutePlusRegister(cycles, X_REGISTER, memory);
                loadRegister(address, A);
                break;
            }

            // load accumulator from absolute address + y register
            case INS_LDA_ABY: // testing complete
            {
                const Word address = fetchAddressAbsolutePlusRegister(cycles, Y_REGISTER, memory);
                loadRegister(address, A);
                break;
            }

            // load accumulator with indexed indirect addressing + x register
            case INS_LDA_INX: // testing complete
            {
                const Word address = fetchAddressIndexedIndirect(cycles, memory);
                loadRegister(address, A);
                break;
            }

            // load accumulator with indirect indexed addressing + y register
            case INS_LDA_INY: // testing complete
            {
                const Word address = fetchAddressIndirectIndexed(cycles, memory, true);
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
                const Byte address = fetchAddressZeroPage(cycles, memory);
                loadRegister(address, X);
                break;
            }

            // load x register from zero page + y register address
            case INS_LDX_ZPY: // testing complete
            {
                const Byte address = fetchAddressZeroPagePlusRegister(cycles, Y_REGISTER, memory);
                loadRegister(address, X);
                break;
            }

            // load x register from absolute address
            case INS_LDX_ABS: // testing complete
            {
                const Word address = fetchAddressAbsolute(cycles, memory);
                loadRegister(address, X);
                break;
            }

            // load x register from absolute + y register address
            case INS_LDX_ABY: // testing complete
            {
                const Word address = fetchAddressAbsolutePlusRegister(cycles, Y_REGISTER, memory);
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
                const Byte address = fetchAddressZeroPage(cycles, memory);
                loadRegister(address, Y);
                break;
            }

            // load y register from zero page + x register address
            case INS_LDY_ZPX: // testing complete
            {
                const Byte address = fetchAddressZeroPagePlusRegister(cycles, X_REGISTER, memory);
                loadRegister(address, Y);
                break;
            }

            // load y register from absolute address
            case INS_LDY_ABS: // testing complete
            {
                const Word address = fetchAddressAbsolute(cycles, memory);
                loadRegister(address, Y);
                break;
            }

            // load y register from absolute + x register address
            case INS_LDY_ABX: // testing complete
            {
                const Word address = fetchAddressAbsolutePlusRegister(cycles, X_REGISTER, memory);
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
                const Byte address = fetchAddressZeroPage(cycles, memory);
                writeByte(cycles, A, address, memory);
                break;
            }

            // store accumulator value to memory from zero page + x register address
            case INS_STA_ZPX: // testing complete
            {
                const Byte address = fetchAddressZeroPagePlusRegister(cycles, X_REGISTER, memory);
                writeByte(cycles, A, address, memory);
                break;
            }

            // store accumulator value to memory from absolute address
            case INS_STA_ABS: // testing complete
            {
                const Word address = fetchAddressAbsolute(cycles, memory);
                writeByte(cycles, A, address, memory);
                break;
            }

            // store accumulator to memory from absolute + x register address
            case INS_STA_ABX: // testing complete
            {
                const Word address = fetchAddressAbsolutePlusRegister(cycles, X_REGISTER, memory);
                cycles--; // for arithmetic
                writeByte(cycles, A, address, memory);
                break;
            }

            // store accumulator to memory from absolute + y register address
            case INS_STA_ABY: // testing complete
            {
                const Word address = fetchAddressAbsolutePlusRegister(cycles, Y_REGISTER, memory);
                cycles--; // for arithmetic
                writeByte(cycles, A, address, memory);
                break;
            }

            // store accumulator to memory from indexed indirect + x register address
            case INS_STA_INX: // testing complete
            {
                const Word address = fetchAddressIndexedIndirect(cycles, memory);
                writeByte(cycles, A, address, memory);
                break;
            }

            // store accumulator to memeory from indirect indexed + y register address
            case INS_STA_INY: // testing complete
            {
                const Word address = fetchAddressIndirectIndexed(cycles, memory, false);
                writeByte(cycles, A, address, memory);
                break;
            }

            /**
             * X REGISTER
             */

            // store x register value to memory from zero page address
            case INS_STX_ZPG: // testing complete
            {
                const Byte address = fetchAddressZeroPage(cycles, memory);
                writeByte(cycles, X, address, memory);
                break;
            }

            // store x register value to memory from zero page + x register address
            case INS_STX_ZPY: // testing complete
            {
                const Byte address = fetchAddressZeroPagePlusRegister(cycles, Y_REGISTER, memory);
                writeByte(cycles, X, address, memory);
                break;
            }

            // store x register value to memory from absolute address
            case INS_STX_ABS: // testing complete
            {
                const Word address = fetchAddressAbsolute(cycles, memory);
                writeByte(cycles, X, address, memory);
                break;
            }

            /**
             * Y REGISTER
             */

            // store y register to memory from zero page address
            case INS_STY_ZPG: // testing complete
            {
                const Byte address = fetchAddressZeroPage(cycles, memory);
                writeByte(cycles, Y, address, memory);
                break;
            }

            // store y register to memory from zero page + x register address
            case INS_STY_ZPX: // testing complete
            {
                const Byte address = fetchAddressZeroPagePlusRegister(cycles, X_REGISTER, memory);
                writeByte(cycles, Y, address, memory);
                break;
            }

            // store y register value to memory from absolute address
            case INS_STY_ABS: // testing complete
            {
                const Word address = fetchAddressAbsolute(cycles, memory);
                writeByte(cycles, Y, address, memory);
                break;
            }

            /**
             * JUMP AND CALL INSTRUCTIONS
             */

            // jump to location from absolute address
            case INS_JMP_ABS: // testing complete
            {
                const Word address = readNextWord(cycles, memory);
                PC = address;
                break;
            }

            // jump to location from indirect address
            case INS_JMP_IND: // testing complete
            {
                const Word address = readNextWord(cycles, memory);
                const Word effectiveAddress = readWordFromAddress(cycles, address, memory);
                PC = effectiveAddress;
                break;
            }

            // jump to subroutine from absolute address
            case INS_JSR_ABS: // testing complete
            {
                const Word address = readNextWord(cycles, memory);
                pushWordToStack(cycles, PC - 1, memory);
                PC = address;
                cycles--; // for arithmetic
                break;
            }

            // return from subroutine
            case INS_RTS: // testing complete
            {
                const Word address = popWordFromStack(cycles, memory);
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
                const Byte value = popByteFromStack(cycles, memory);
                A = value;
                cycles--; // extra cycle
                setFlagStatus_NZ(A);
                break;
            }

            // pull processor from stack
            case INS_PLP: // testing complete
            {
                const Byte value = popByteFromStack(cycles, memory);
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
                const Byte address = fetchAddressZeroPage(cycles, memory);
                andOperation(address, A);
                break;
            }

            // and zero page + x register address
            case INS_AND_ZPX: // testing complete
            {
                const Byte address = fetchAddressZeroPagePlusRegister(cycles, X_REGISTER, memory);
                andOperation(address, A);
                break;
            }

            // and zero page absolute address
            case INS_AND_ABS: // testing complete
            {
                const Word address = fetchAddressAbsolute(cycles, memory);
                andOperation(address, A);
                break;
            }

            // and zero page absolute + x register address
            case INS_AND_ABX: // testing complete
            {
                const Word address = fetchAddressAbsolutePlusRegister(cycles, X_REGISTER, memory);
                andOperation(address, A);
                break;
            }

            // and zero page absolute + y register address
            case INS_AND_ABY: // testing complete
            {
                const Word address = fetchAddressAbsolutePlusRegister(cycles, Y_REGISTER, memory);
                andOperation(address, A);
                break;
            }

            // and indexed indirect addressing
            case INS_AND_INX: // testing complete
            {
                const Word address = fetchAddressIndexedIndirect(cycles, memory);
                andOperation(address, A);
                break;
            }

            // and indirect indexed addressing
            case INS_AND_INY: // testing complete
            {
                const Word address = fetchAddressIndirectIndexed(cycles, memory, true);
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
                const Byte address = fetchAddressZeroPage(cycles, memory);
                xorOperation(address, A);
                break;
            }

            // or zero page + x register address
            case INS_EOR_ZPX: // testing complete
            {
                const Byte address = fetchAddressZeroPagePlusRegister(cycles, X_REGISTER, memory);
                xorOperation(address, A);
                break;
            }

            // or zero page absolute address
            case INS_EOR_ABS: // testing complete
            {
                const Word address = fetchAddressAbsolute(cycles, memory);
                xorOperation(address, A);
                break;
            }

            // or zero page absolute + x register address
            case INS_EOR_ABX: // testing complete
            {
                const Word address = fetchAddressAbsolutePlusRegister(cycles, X_REGISTER, memory);
                xorOperation(address, A);
                break;
            }

            // or zero page absolute + y register address
            case INS_EOR_ABY: // testing complete
            {
                const Word address = fetchAddressAbsolutePlusRegister(cycles, Y_REGISTER, memory);
                xorOperation(address, A);
                break;
            }

            // or indexed indirect addressing
            case INS_EOR_INX: // testing complete
            {
                const Word address = fetchAddressIndexedIndirect(cycles, memory);
                xorOperation(address, A);
                break;
            }

            // or indirect indexed addressing
            case INS_EOR_INY: // testing complete
            {
                const Word address = fetchAddressIndirectIndexed(cycles, memory, true);
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
                const Byte address = fetchAddressZeroPage(cycles, memory);
                orOperation(address, A);
                break;
            }

            // or zero page + x register address
            case INS_ORA_ZPX: // testing complete
            {
                const Byte address = fetchAddressZeroPagePlusRegister(cycles, X_REGISTER, memory);
                orOperation(address, A);
                break;
            }

            // or zero page absolute address
            case INS_ORA_ABS: // testing complete
            {
                const Word address = fetchAddressAbsolute(cycles, memory);
                orOperation(address, A);
                break;
            }

            // or zero page absolute + x register address
            case INS_ORA_ABX: // testing complete
            {
                const Word address = fetchAddressAbsolutePlusRegister(cycles, X_REGISTER, memory);
                orOperation(address, A);
                break;
            }

            // or zero page absolute + y register address
            case INS_ORA_ABY: // testing complete
            {
                const Word address = fetchAddressAbsolutePlusRegister(cycles, Y_REGISTER, memory);
                orOperation(address, A);
                break;
            }

            // or indexed indirect addressing
            case INS_ORA_INX: // testing complete
            {
                const Word address = fetchAddressIndexedIndirect(cycles, memory);
                orOperation(address, A);
                break;
            }

            // or indirect indexed addressing
            case INS_ORA_INY: // testing complete
            {
                const Word address = fetchAddressIndirectIndexed(cycles, memory, true);
                orOperation(address, A);
                break;
            }

            /**
             * BIT TEST OPERATIONS
             */

            // bit test zero page addressing
            case INS_BIT_ZPG: // testing complete
            {
                const Byte address = fetchAddressZeroPage(cycles, memory);
                const Byte value = readByteFromAddress(cycles, address, memory);
                setFlagStatus_BIT(value);
                break;
            }

            // bit test absolute addressing
            case INS_BIT_ABS: // testing complete
            {
                const Word address = fetchAddressAbsolute(cycles, memory);
                const Byte value = readByteFromAddress(cycles, address, memory);
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
                const Byte address = fetchAddressZeroPage(cycles, memory);
                increment(address);
                break;
            }

            // increment memory at zero page + x register address
            case INS_INC_ZPX: // testing complete
            {
                const Byte address = fetchAddressZeroPagePlusRegister(cycles, X_REGISTER, memory);
                increment(address);
                break;
            }

            // increment memory at absolute address
            case INS_INC_ABS: // testing complete
            {
                const Word address = fetchAddressAbsolute(cycles, memory);
                increment(address);
                break;
            }

            // increment memory at absolute + x register address
            case INS_INC_ABX:
            {
                const Word address = fetchAddressAbsolutePlusRegister(cycles, X_REGISTER, memory);
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
                const Byte address = fetchAddressZeroPage(cycles, memory);
                decrement(address);
                break;
            }

            // decrement memory at zero page + x register address
            case INS_DEC_ZPX: // testing complete
            {
                const Byte address = fetchAddressZeroPagePlusRegister(cycles, X_REGISTER, memory);
                decrement(address);
                break;
            }

            // decrement memory at absolute address
            case INS_DEC_ABS: // testing complete
            {
                const Word address = fetchAddressAbsolute(cycles, memory);
                decrement(address);
                break;
            }

            // decrement memory at absolute + x register address
            case INS_DEC_ABX:
            {
                const Word address = fetchAddressAbsolutePlusRegister(cycles, X_REGISTER, memory);
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
                const bool cFlag = getCarryFlag();
                branch(cFlag, false);
                break;
            }

            // branch if carry flag set
            case INS_BCS: // testing complete
            {
                const bool cFlag = getCarryFlag();
                branch(cFlag, true);
                break;
            }

            // branch if zero flag clear
            case INS_BNE: // testing complete
            {
                const bool zFlag = getZeroFlag();
                branch(zFlag, false);
                break;
            }

            // branch if zero flag set
            case INS_BEQ: // testing complete
            {
                const bool zFlag = getZeroFlag();
                branch(zFlag, true);
                break;
            }

            // branch if negative flag clear
            case INS_BPL: // testing complete
            {
                const bool nFlag = getNegativeFlag();
                branch(nFlag, false);
                break;
            }

            // branch if negative flag set
            case INS_BMI: // testing complete
            {
                const bool nFlag = getNegativeFlag();
                branch(nFlag, true);
                break;
            }

            // branch if overflow flag clear
            case INS_BVC: // testing complete
            {
                const bool vFlag = getOverflowFlag();
                branch(vFlag, false);
                break;
            }

            // branch if overflow flag set
            case INS_BVS: // testing complete
            {
                const bool vFlag = getOverflowFlag();
                branch(vFlag, true);
                break;
            }

            /**
             * STATUS FLAG CHANGE INSTRUCTIONS
             */

            // clear carry flag
            case INS_CLC: // testing complete
            {
                setCarryFlag(false);
                cycles--;
                break;
            }

            // clear decimal flag
            case INS_CLD: // testing complete
            {
                setDecimalFlag(false);
                cycles--;
                break;
            }

            // clear interrupt flag
            case INS_CLI: // testing complete
            {
                setInterruptFlag(false);
                cycles--;
                break;
            }

            // clear overflow flag
            case INS_CLV: // testing complete
            {
                setOverflowFlag(false);
                cycles--;
                break;
            }

            // set carry flag
            case INS_SEC: // testing complete
            {
                setCarryFlag(true);
                cycles--;
                break;
            }

            // set decimal flag
            case INS_SED: // testing complete
            {
                setDecimalFlag(true);
                cycles--;
                break;
            }

            // set interrupt flag
            case INS_SEI: // testing complete
            {
                setInterruptFlag(true);
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
                const Byte value = readNextByte(cycles, memory);
                const Byte aCopy = A;
                const bool cFlag = getCarryFlag();
                const Word result = A + value + (cFlag ? 1 : 0);
                A = result & 0xFF; // only storing low byte of result
                setFlagStatus_ADC(result, aCopy, value);
                break;
            }

            // add with carry zero page addressing
            case INS_ADC_ZPG: // testing complete
            {
                const Byte address = fetchAddressZeroPage(cycles, memory);
                addWithCarry(address);
                break;
            }

            // add with carry zero page + x register addressing
            case INS_ADC_ZPX: // testing complete
            {
                const Byte address = fetchAddressZeroPagePlusRegister(cycles, X_REGISTER, memory);
                addWithCarry(address);
                break;
            }

            // add with carry absolute addressing
            case INS_ADC_ABS: // testing complete
            {
                const Word address = fetchAddressAbsolute(cycles, memory);
                addWithCarry(address);
                break;
            }

            // add with carry absolute + x register addressing
            case INS_ADC_ABX: // testing complete
            {
                const Word address = fetchAddressAbsolutePlusRegister(cycles, X_REGISTER, memory);
                addWithCarry(address);
                break;
            }

            // add with carry absolute + y register addressing
            case INS_ADC_ABY: // testing complete
            {
                const Word address = fetchAddressAbsolutePlusRegister(cycles, Y_REGISTER, memory);
                addWithCarry(address);
                break;
            }

            // add with carry indexed indirect addressing
            case INS_ADC_INX: // testing complete
            {
                const Word address = fetchAddressIndexedIndirect(cycles, memory);
                addWithCarry(address);
                break;
            }

            // add with carry indirect indexed addressing
            case INS_ADC_INY: // testing complete
            {
                const Word address = fetchAddressIndirectIndexed(cycles, memory, true);
                addWithCarry(address);
                break;
            }

            /**
             * SUBTRACT WITH CARRY
             */

            // subtract with carry immediate addressing
            case INS_SBC_IMM: // testing complete
            {
                const Byte value = readNextByte(cycles, memory);
                const Byte aCopy = A;
                const bool cFlag = getCarryFlag();
                const Word result = A - value - (cFlag ? 0 : 1);
                A = result & 0xFF; // only storing low byte of result
                setFlagStatus_SBC(result, aCopy, value);
                break;
            }

            // subtract with carry zero page addressing
            case INS_SBC_ZPG: // testing complete
            {
                const Byte address = fetchAddressZeroPage(cycles, memory);
                subWithCarry(address);
                break;
            }

            // subtract with carry zero page + x register addressing
            case INS_SBC_ZPX: // testing complete
            {
                const Byte address = fetchAddressZeroPagePlusRegister(cycles, X_REGISTER, memory);
                subWithCarry(address);
                break;
            }

            // subtract with carry absolute addressing
            case INS_SBC_ABS: // testing complete
            {
                const Word address = fetchAddressAbsolute(cycles, memory);
                subWithCarry(address);
                break;
            }

            // subtract with carry absolute + x register addressing
            case INS_SBC_ABX: // testing complete
            {
                const Word address = fetchAddressAbsolutePlusRegister(cycles, X_REGISTER, memory);
                subWithCarry(address);
                break;
            }

            // subtract with carry absolute + y register addressing
            case INS_SBC_ABY: // testing complete
            {
                const Word address = fetchAddressAbsolutePlusRegister(cycles, Y_REGISTER, memory);
                subWithCarry(address);
                break;
            }

            // subtract with carry indexed indirect addressing
            case INS_SBC_INX: // testing complete
            {
                const Word address = fetchAddressIndexedIndirect(cycles, memory);
                subWithCarry(address);
                break;
            }

            // subtract with carry indirect indexed addressing
            case INS_SBC_INY: // testing complete
            {
                const Word address = fetchAddressIndirectIndexed(cycles, memory, true);
                subWithCarry(address);
                break;
            }

            /**
             * COMPARE INSTRUCTIONS
             */

            /**
             * COMPARE ACCUMULATOR
             */

            // compare accumulator immediate addressing
            case INS_CMP_IMM: // testing complete
            {
                const Byte value = readNextByte(cycles, memory);
                setFlagStatus_CMP(value, A);
                break;
            }

            // compare accumulator zero page addressing
            case INS_CMP_ZPG: // testing complete
            {
                const Byte address = fetchAddressZeroPage(cycles, memory);
                const Byte value = readByteFromAddress(cycles, address, memory);
                setFlagStatus_CMP(value, A);
                break;
            }

            // compare accumulator zero page + x register addressing
            case INS_CMP_ZPX: // testing complete
            {
                const Byte address = fetchAddressZeroPagePlusRegister(cycles, X_REGISTER, memory);
                const Byte value = readByteFromAddress(cycles, address, memory);
                setFlagStatus_CMP(value, A);
                break;
            }

            // compare accumulator absolute addressing
            case INS_CMP_ABS: // testing complete
            {
                const Word address = fetchAddressAbsolute(cycles, memory);
                const Byte value = readByteFromAddress(cycles, address, memory);
                setFlagStatus_CMP(value, A);
                break;
            }

            // compare accumulator absolute + x register addressing
            case INS_CMP_ABX: // testing complete
            {
                const Word address = fetchAddressAbsolutePlusRegister(cycles, X_REGISTER, memory);
                const Byte value = readByteFromAddress(cycles, address, memory);
                setFlagStatus_CMP(value, A);
                break;
            }

            // compare accumulator absolute + y register addressing
            case INS_CMP_ABY: // testing complete
            {
                const Word address = fetchAddressAbsolutePlusRegister(cycles, Y_REGISTER, memory);
                const Byte value = readByteFromAddress(cycles, address, memory);
                setFlagStatus_CMP(value, A);
                break;
            }

            // compare accumulator indexed indirect addressing
            case INS_CMP_INX: // testing complete
            {
                const Word address = fetchAddressIndexedIndirect(cycles, memory);
                const Byte value = readByteFromAddress(cycles, address, memory);
                setFlagStatus_CMP(value, A);
                break;
            }

            // compare accumulator indirect indexed addressing
            case INS_CMP_INY: // testing complete
            {
                const Word address = fetchAddressIndirectIndexed(cycles, memory, true);
                const Byte value = readByteFromAddress(cycles, address, memory);
                setFlagStatus_CMP(value, A);
                break;
            }

            /**
             * COMPARE X REGISTER
             */

            // compare x register immediate addressing
            case INS_CPX_IMM:
            {
                const Byte value = readNextByte(cycles, memory);
                setFlagStatus_CMP(value, X);
                break;
            }

            // compare x register zero page addressing
            case INS_CPX_ZPG:
            {
                const Byte address = fetchAddressZeroPage(cycles, memory);
                const Byte value = readByteFromAddress(cycles, address, memory);
                setFlagStatus_CMP(value, X);
                break;
            }

            // compare x register absolute addressing
            case INS_CPX_ABS:
            {
                const Word address = fetchAddressAbsolute(cycles, memory);
                const Byte value = readByteFromAddress(cycles, address, memory);
                setFlagStatus_CMP(value, X);
                break;
            }

            // compare y register immediate addressing
            case INS_CPY_IMM:
            {
                const Byte value = readNextByte(cycles, memory);
                setFlagStatus_CMP(value, Y);
                break;
            }

            // compare y register zero page addressing
            case INS_CPY_ZPG:
            {
                const Byte address = fetchAddressZeroPage(cycles, memory);
                const Byte value = readByteFromAddress(cycles, address, memory);
                setFlagStatus_CMP(value, Y);
                break;
            }

            // compare y register absolute addressing
            case INS_CPY_ABS:
            {
                const Word address = fetchAddressAbsolute(cycles, memory);
                const Byte value = readByteFromAddress(cycles, address, memory);
                setFlagStatus_CMP(value, Y);
                break;
            }

            /**
             * BIT SHIFT INSTRUCTIONS
             */

            /**
             * ARITHMETIC SHIFT LEFT
             */

            // shift accumulator value left
            case INS_ASL_ACC: // testing complete
            {
                const Byte aCopy = A;
                A <<= 1;
                cycles--;
                setFlagStatus_Arithmetic(aCopy, A, true);
                break;
            }

            // shift value at zero page address left
            case INS_ASL_ZPG: // testing complete
            {
                const Byte address = fetchAddressZeroPage(cycles, memory);
                bitShift(address, true);
                break;
            }

            // shift value at zero page address + x register left
            case INS_ASL_ZPX: // testing complete
            {
                const Byte address = fetchAddressZeroPagePlusRegister(cycles, X_REGISTER, memory);
                bitShift(address, true);
                break;
            }

            // shift value at absolute address left
            case INS_ASL_ABS: // testing complete
            {
                const Word address = fetchAddressAbsolute(cycles, memory);
                bitShift(address, true);
                break;
            }

            // shift value at absolute address + x register left
            case INS_ASL_ABX: // testing complete
            {
                const Word address = fetchAddressAbsolutePlusRegister(cycles, X_REGISTER, memory);
                bitShift(address, true);
                cycles--; // extra cycle
                break;
            }

            /**
             * LOGICAL SHIFT RIGHT
             */

            // shift accumulator value right
            case INS_LSR_ACC: // testing complete
            {
                const Byte aCopy = A;
                A >>= 1;
                cycles--;
                setFlagStatus_Arithmetic(aCopy, A);
                break;
            }

            // shift value at zero page address right
            case INS_LSR_ZPG: // testing complete
            {
                const Byte address = fetchAddressZeroPage(cycles, memory);
                bitShift(address);
                break;
            }

            // shift value at zero page address + x register right
            case INS_LSR_ZPX: // testing complete
            {
                const Byte address = fetchAddressZeroPagePlusRegister(cycles, X_REGISTER, memory);
                bitShift(address);
                break;
            }

            // shift value at absolute address + x register right
            case INS_LSR_ABS: // testing complete
            {
                const Word address = fetchAddressAbsolute(cycles, memory);
                bitShift(address);
                break;
            }

            // shift value at absolute address + x register right
            case INS_LSR_ABX: // testing complete
            {
                const Word address = fetchAddressAbsolutePlusRegister(cycles, X_REGISTER, memory);
                bitShift(address);
                cycles--; // extra cycle
                break;
            }

            /**
             * ROTATE LEFT
             */

            // rotate accumulator value left
            case INS_ROL_ACC: // testing complete
            {
                const Byte aCopy = A;
                A <<= 1;
                A |= P.bits.C;
                cycles--;
                setFlagStatus_Arithmetic(aCopy, A, true);
                break;
            }

            // rotate value at zero page address left
            case INS_ROL_ZPG: // testing complete
            {
                const Byte address = fetchAddressZeroPage(cycles, memory);
                bitRotate(address, true);
                break;
            }

            // rotate value at zero page address + x register left
            case INS_ROL_ZPX: // testing complete
            {
                const Byte address = fetchAddressZeroPagePlusRegister(cycles, X_REGISTER, memory);
                bitRotate(address, true);
                break;
            }

            // rotate value at absolute address left
            case INS_ROL_ABS: // testing complete
            {
                const Word address = fetchAddressAbsolute(cycles, memory);
                bitRotate(address, true);
                break;
            }

            // rotate value at absolute address + x register left
            case INS_ROL_ABX: // testing complete
            {
                const Word address = fetchAddressAbsolutePlusRegister(cycles, X_REGISTER, memory);
                bitRotate(address, true);
                cycles--; // extra cycle
                break;
            }

            /**
             * ROTATE RIGHT
             */

            // rotate accumulator value right
            case INS_ROR_ACC: // testing complete
            {
                const Byte aCopy = A;
                A >>= 1;
                A |= (P.bits.C << 7);
                cycles--;
                setFlagStatus_Arithmetic(aCopy, A);
                break;
            }

            // rotate value at zero page address right
            case INS_ROR_ZPG: // testing complete
            {
                const Byte address = fetchAddressZeroPage(cycles, memory);
                bitRotate(address);
                break;
            }

            // rotate value at zero page address + x register right
            case INS_ROR_ZPX: // testing complete
            {
                const Byte address = fetchAddressZeroPagePlusRegister(cycles, X_REGISTER, memory);
                bitRotate(address);
                break;
            }

            // rotate value at absolute address right
            case INS_ROR_ABS: // testing complete
            {
                const Word address = fetchAddressAbsolute(cycles, memory);
                bitRotate(address);
                break;
            }

            // rotate value at absolute address + x register right
            case INS_ROR_ABX: // testing complete
            {
                const Word address = fetchAddressAbsolutePlusRegister(cycles, X_REGISTER, memory);
                bitRotate(address);
                cycles--; // extra cycle
                break;
            }

            /**
             * SYSTEM FUNCTION INSTRUCTIONS
             */

            // TODO: fix after other commands implemented?
            case INS_BRK:
            {
                const Byte signatureByte = readNextByte(cycles, memory);
                pushByteToStack(cycles, PC >> 8, memory);
                pushByteToStack(cycles, PC & 0xFF, memory);
                setBreakFlag(true);
                pushByteToStack(cycles, P.value, memory);
                setInterruptFlag(true);
                const Byte irqVectorLow = readByteFromAddress(cycles, IRQ_VECTOR_LOW, memory);
                const Byte irqVectorHigh = readByteFromAddress(cycles, IRQ_VECTOR_HIGH, memory);
                const Word irqVector = (irqVectorHigh << 8) | irqVectorLow;
                PC = irqVector;
                break;
            }

            case INS_NOP:
            {
                cycles--;
                break;
            }

            case INS_RTI:
            {
                const Byte statusFlags = popByteFromStack(cycles, memory);
                std::cout << "Status Flags: 0x" << std::hex << (int)statusFlags << std::endl;
                setStatusFlags(statusFlags);
                const Byte programCounter = popByteFromStack(cycles, memory);
                PC = programCounter;
                cycles--; // extra cycle
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

        const s_int32 cyclesUsed = cyclesRequested - cycles;
        return cyclesUsed;
    }

    Word CPU::loadProgram(const Byte *program, u_int32 numBytes, Mem &memory)
    {
        if (!program || numBytes <= 2)
        {
            throw std::invalid_argument("Invalid program or size too small.");
        }

        u_int32 programPtr = 0;
        // first word is always address where program is stored (e.g '00 80' @ '$8000')
        const Word loadAddress = program[programPtr] | (program[programPtr + 1] << 8);
        programPtr += 2;

        const u_int32 programSize = numBytes - 2;
        for (u_int32 i = 0; i < programSize; i++)
        {
            memory[loadAddress + i] = program[programPtr++];
        }
        return loadAddress;
    }
}