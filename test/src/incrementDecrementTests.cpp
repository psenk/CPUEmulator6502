#include <gtest/gtest.h>
#include "m6502.h"

class IncrementDecrementTests : public ::testing::Test
{
protected:
    m6502::Mem mem;
    m6502::CPU cpu;

    void SetUp()
    {
        cpu.reset(mem);
    }

    void TearDown()
    {
    }

    void testIncDecZeroPageAddressing(m6502::Byte opcode,
                                      bool increment)
    {
        // arrange:
        using namespace m6502;
        static constexpr s32 NUM_CYCLES = 5;
        CPU cpuCopy = cpu;
        mem[0x0072] = 0x42;

        mem[0xFFFC] = opcode;
        mem[0xFFFD] = 0x72;

        // act:
        s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

        // assert:
        if (increment)
        {
            EXPECT_EQ(mem[0x0072], 0x43);
        }
        else
        {
            EXPECT_EQ(mem[0x0072], 0x41);
        }
        EXPECT_EQ(cyclesExecuted, 5);
        testAllFlagsUnchanged(cpuCopy, cpu);
    }

    void testIncDecZeroPageXAddressing(m6502::Byte opcode,
                                       bool increment)
    {
        // arrange:
        using namespace m6502;
        static constexpr s32 NUM_CYCLES = 6;
        CPU cpuCopy = cpu;
        mem[0x0082] = 0x42;
        cpu.X = 0x10;

        mem[0xFFFC] = opcode;
        mem[0xFFFD] = 0x72;

        // act:
        s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

        // assert:
        if (increment)
        {
            EXPECT_EQ(mem[0x0082], 0x43);
        }
        else
        {
            EXPECT_EQ(mem[0x0082], 0x41);
        }
        EXPECT_EQ(cyclesExecuted, 6);
        testAllFlagsUnchanged(cpuCopy, cpu);
    }

    void testIncDecAbsoluteAddressing(m6502::Byte opcode,
                                      bool increment)
    {
        // arrange:
        using namespace m6502;
        static constexpr s32 NUM_CYCLES = 6;
        CPU cpuCopy = cpu;
        mem[0x0420] = 0x42;

        mem[0xFFFC] = opcode;
        mem[0xFFFD] = 0x20;
        mem[0xFFFE] = 0x04;

        // act:
        s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

        // assert:
        if (increment)
        {
            EXPECT_EQ(mem[0x0420], 0x43);
        }
        else
        {
            EXPECT_EQ(mem[0x0420], 0x41);
        }
        EXPECT_EQ(cyclesExecuted, 6);
        testAllFlagsUnchanged(cpuCopy, cpu);
    }

    void testIncDecAbsoluteXAddressing(m6502::Byte opcode,
                                       bool increment)
    {
        // arrange:
        using namespace m6502;
        static constexpr s32 NUM_CYCLES = 7;
        CPU cpuCopy = cpu;
        mem[0x0430] = 0x42;
        cpu.X = 0x10;

        mem[0xFFFC] = opcode;
        mem[0xFFFD] = 0x20;
        mem[0xFFFE] = 0x04;

        // act:
        s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

        // assert:
        if (increment)
        {
            EXPECT_EQ(mem[0x0430], 0x43);
        }
        else
        {
            EXPECT_EQ(mem[0x0430], 0x41);
        }
        EXPECT_EQ(cyclesExecuted, 7);
        testAllFlagsUnchanged(cpuCopy, cpu);
    }

    void testIncrement(m6502::Byte opcode,
                       m6502::Byte value,
                       m6502::Byte m6502::CPU::*reg)
    {
        // arrange:
        using namespace m6502;
        static constexpr s32 NUM_CYCLES = 2;
        cpu.*reg = value;
        Byte newValue = ++value;

        mem[0xFFFC] = opcode;

        // act:
        s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

        // assert:
        EXPECT_EQ(cpu.*reg, newValue);
        EXPECT_EQ(cyclesExecuted, 2);
    }

    void testDecrement(m6502::Byte opcode,
                       m6502::Byte value,
                       m6502::Byte m6502::CPU::*reg)
    {
        // arrange:
        using namespace m6502;
        static constexpr s32 NUM_CYCLES = 2;
        cpu.*reg = value;
        Byte newValue = --value;

        mem[0xFFFC] = opcode;

        // act:
        s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

        // assert:
        EXPECT_EQ(cpu.*reg, newValue);
        EXPECT_EQ(cyclesExecuted, 2);
    }

    // test flags
    static void testIncDecFlagsUnchanged(const m6502::CPU &cpuCopy,
                                         const m6502::CPU &cpu)
    {
        EXPECT_EQ(cpuCopy.P.bits.C, cpu.P.bits.C);
        EXPECT_EQ(cpuCopy.P.bits.I, cpu.P.bits.I);
        EXPECT_EQ(cpuCopy.P.bits.D, cpu.P.bits.D);
        EXPECT_EQ(cpuCopy.P.bits.B, cpu.P.bits.B);
        EXPECT_EQ(cpuCopy.P.bits.V, cpu.P.bits.V);
    }

    static void testAllFlagsUnchanged(const m6502::CPU &cpuCopy,
                                      const m6502::CPU &cpu)
    {
        EXPECT_EQ(cpuCopy.P.value, cpu.P.value);
    }
};

TEST_F(IncrementDecrementTests, IncrementMemory_ZeroPageAddressing)
{
    using namespace m6502;
    testIncDecZeroPageAddressing(CPU::INS_INC_ZPG, true);
}

TEST_F(IncrementDecrementTests, IncrementMemory_ZeroPageXAddressing)
{
    using namespace m6502;
    testIncDecZeroPageXAddressing(CPU::INS_INC_ZPX, true);
}

TEST_F(IncrementDecrementTests, IncrementMemory_AbsoluteAddressing)
{
    using namespace m6502;
    testIncDecAbsoluteAddressing(CPU::INS_INC_ABS, true);
}

TEST_F(IncrementDecrementTests, IncrementMemory_AbsoluteXAddressing)
{
    using namespace m6502;
    testIncDecAbsoluteXAddressing(CPU::INS_INC_ABX, true);
}

TEST_F(IncrementDecrementTests, IncrementXRegister)
{
    using namespace m6502;
    CPU cpuCopy = cpu;
    testIncrement(CPU::INS_INX, 0x42, &CPU::X);
    testAllFlagsUnchanged(cpuCopy, cpu);
}

TEST_F(IncrementDecrementTests, IncrementXRegister_ZeroValue)
{
    using namespace m6502;
    CPU cpuCopy = cpu;
    testIncrement(CPU::INS_INX, 0xFF, &CPU::X);
    testIncDecFlagsUnchanged(cpuCopy, cpu);
    EXPECT_TRUE(cpu.P.bits.Z);
}

TEST_F(IncrementDecrementTests, IncrementXRegister_NegativeValue)
{
    using namespace m6502;
    CPU cpuCopy = cpu;
    testIncrement(CPU::INS_INX, 0x84, &CPU::X);
    testIncDecFlagsUnchanged(cpuCopy, cpu);
    EXPECT_TRUE(cpu.P.bits.N);
}

TEST_F(IncrementDecrementTests, IncrementYRegister)
{
    using namespace m6502;
    CPU cpuCopy = cpu;
    testIncrement(CPU::INS_INY, 0x42, &CPU::Y);
    testAllFlagsUnchanged(cpuCopy, cpu);
}

TEST_F(IncrementDecrementTests, IncrementYRegister_ZeroValue)
{
    using namespace m6502;
    CPU cpuCopy = cpu;
    testIncrement(CPU::INS_INY, 0xFF, &CPU::Y);
    testIncDecFlagsUnchanged(cpuCopy, cpu);
    EXPECT_TRUE(cpu.P.bits.Z);
}

TEST_F(IncrementDecrementTests, IncrementYRegister_NegativeValue)
{
    using namespace m6502;
    CPU cpuCopy = cpu;
    testIncrement(CPU::INS_INY, 0x84, &CPU::Y);
    testIncDecFlagsUnchanged(cpuCopy, cpu);
    EXPECT_TRUE(cpu.P.bits.N);
}

TEST_F(IncrementDecrementTests, DecrementMemory_ZeroPageAddressing)
{
    using namespace m6502;
    testIncDecZeroPageAddressing(CPU::INS_DEC_ZPG, false);
}

TEST_F(IncrementDecrementTests, DecrementMemory_ZeroPageXAddressing)
{
    using namespace m6502;
    testIncDecZeroPageXAddressing(CPU::INS_DEC_ZPX, false);
}

TEST_F(IncrementDecrementTests, DecrementMemory_AbsoluteAddressing)
{
    using namespace m6502;
    testIncDecAbsoluteAddressing(CPU::INS_DEC_ABS, false);
}

TEST_F(IncrementDecrementTests, DecrementMemory_AbsoluteXAddressing)
{
    using namespace m6502;
    testIncDecAbsoluteXAddressing(CPU::INS_DEC_ABX, false);
}

TEST_F(IncrementDecrementTests, DecrementXRegister)
{
    using namespace m6502;
    CPU cpuCopy = cpu;
    testDecrement(CPU::INS_DEX, 0x42, &CPU::X);
    testAllFlagsUnchanged(cpuCopy, cpu);
}

TEST_F(IncrementDecrementTests, DecrementXRegister_ZeroValue)
{
    using namespace m6502;
    CPU cpuCopy = cpu;
    testDecrement(CPU::INS_DEX, 0x01, &CPU::X);
    testIncDecFlagsUnchanged(cpuCopy, cpu);
}

TEST_F(IncrementDecrementTests, DecrementXRegister_NegativeValue)
{
    using namespace m6502;
    CPU cpuCopy = cpu;
    testDecrement(CPU::INS_DEX, 0x84, &CPU::X);
    testIncDecFlagsUnchanged(cpuCopy, cpu);
}

TEST_F(IncrementDecrementTests, DecrementYRegister)
{
    using namespace m6502;
    CPU cpuCopy = cpu;
    testDecrement(CPU::INS_DEY, 0x42, &CPU::Y);
    testAllFlagsUnchanged(cpuCopy, cpu);
}

TEST_F(IncrementDecrementTests, DecrementYRegister_ZeroValue)
{
    using namespace m6502;
    CPU cpuCopy = cpu;
    testDecrement(CPU::INS_DEY, 0x01, &CPU::Y);
    testIncDecFlagsUnchanged(cpuCopy, cpu);
}

TEST_F(IncrementDecrementTests, DecrementYRegister_NegativeValue)
{
    using namespace m6502;
    CPU cpuCopy = cpu;
    testDecrement(CPU::INS_DEY, 0x84, &CPU::Y);
    testIncDecFlagsUnchanged(cpuCopy, cpu);
}