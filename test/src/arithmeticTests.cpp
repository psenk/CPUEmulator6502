#include <gtest/gtest.h>
#include "m6502.h"

class ArithmeticTests : public ::testing::Test
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

    // test flags
    static void testAllFlagsUnchanged(const m6502::CPU &cpuCopy,
                                      const m6502::CPU &cpu)
    {
        EXPECT_EQ(cpuCopy.P.value, cpu.P.value);
    }
};

/**
 * ADD INSTRUCTION MISC TESTS
 */

TEST_F(ArithmeticTests, ADC_ZeroValues)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s32 NUM_CYCLES = 2;
    cpu.A = 0x00;
    cpu.P.bits.C = false;

    mem[0xFFFC] = CPU::INS_ADC_IMM;
    mem[0xFFFD] = 0x00;

    // act:
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0x00);
    EXPECT_TRUE(cpu.P.bits.Z);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

TEST_F(ArithmeticTests, ADC_ZeroResult)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s32 NUM_CYCLES = 2;
    cpu.A = 0x10;
    cpu.P.bits.C = false;

    mem[0xFFFC] = CPU::INS_ADC_IMM;
    mem[0xFFFD] = 0xF0;

    // act:
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0x00);
    EXPECT_TRUE(cpu.P.bits.Z);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

TEST_F(ArithmeticTests, ADC_NegativeResult)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s32 NUM_CYCLES = 2;
    cpu.A = 0x10;
    cpu.P.bits.C = false;

    mem[0xFFFC] = CPU::INS_ADC_IMM;
    mem[0xFFFD] = 0x84;

    // act:
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0x94);
    EXPECT_TRUE(cpu.P.bits.N);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

TEST_F(ArithmeticTests, ADC_CarryFlagBecomesSet)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s32 NUM_CYCLES = 2;
    cpu.A = 0xFE;
    cpu.P.bits.C = false;

    mem[0xFFFC] = CPU::INS_ADC_IMM;
    mem[0xFFFD] = 0x03;

    // act:
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0x01);
    EXPECT_TRUE(cpu.P.bits.C);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

TEST_F(ArithmeticTests, ADC_CarryFlagRemainsSet)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s32 NUM_CYCLES = 2;
    cpu.A = 0xFE;
    cpu.P.bits.C = true;

    mem[0xFFFC] = CPU::INS_ADC_IMM;
    mem[0xFFFD] = 0x02;

    // act:
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0x01);
    EXPECT_TRUE(cpu.P.bits.C);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

TEST_F(ArithmeticTests, ADC_OverflowFlagSet)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s32 NUM_CYCLES = 2;
    cpu.A = 0x7F;

    mem[0xFFFC] = CPU::INS_ADC_IMM;
    mem[0xFFFD] = 0x02;

    // act:
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0x81);
    EXPECT_TRUE(cpu.P.bits.V);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

/**
 * ADD INSTRUCTIONS
 */

TEST_F(ArithmeticTests, ADC_Immediate)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s32 NUM_CYCLES = 2;
    cpu.A = 0x12;

    mem[0xFFFC] = CPU::INS_ADC_IMM;
    mem[0xFFFD] = 0x13;

    // act:
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0x25);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
    testAllFlagsUnchanged(cpuCopy, cpu);
}

TEST_F(ArithmeticTests, ADC_ZeroPage)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s32 NUM_CYCLES = 3;
    cpu.A = 0x12;

    mem[0xFFFC] = CPU::INS_ADC_ZPG;
    mem[0xFFFD] = 0x42;
    mem[0x0042] = 0x13;

    // act:
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0x25);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
    testAllFlagsUnchanged(cpuCopy, cpu);
}

TEST_F(ArithmeticTests, ADC_ZeroPageX)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s32 NUM_CYCLES = 4;
    cpu.A = 0x12;
    cpu.X = 0x10;

    mem[0xFFFC] = CPU::INS_ADC_ZPX;
    mem[0xFFFD] = 0x42;
    mem[0x0052] = 0x13;

    // act:
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0x25);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
    testAllFlagsUnchanged(cpuCopy, cpu);
}

TEST_F(ArithmeticTests, ADC_Absolute)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s32 NUM_CYCLES = 4;
    cpu.A = 0x12;

    mem[0xFFFC] = CPU::INS_ADC_ABS;
    mem[0xFFFD] = 0x20;
    mem[0xFFFE] = 0x04;
    mem[0x0420] = 0x13;

    // act:
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0x25);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
    testAllFlagsUnchanged(cpuCopy, cpu);
}

TEST_F(ArithmeticTests, ADC_AbsoluteX)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s32 NUM_CYCLES = 4;
    cpu.A = 0x12;
    cpu.X = 0x10;

    mem[0xFFFC] = CPU::INS_ADC_ABX;
    mem[0xFFFD] = 0x20;
    mem[0xFFFE] = 0x04;
    mem[0x0430] = 0x13;

    // act:
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0x25);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
    testAllFlagsUnchanged(cpuCopy, cpu);
}

TEST_F(ArithmeticTests, ADC_AbsoluteX_PageCrossed)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s32 NUM_CYCLES = 5;
    cpu.A = 0x12;
    cpu.X = 0xFF;

    mem[0xFFFC] = CPU::INS_ADC_ABX;
    mem[0xFFFD] = 0x20;
    mem[0xFFFE] = 0x04;
    mem[0x051F] = 0x13;

    // act:
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0x25);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
    testAllFlagsUnchanged(cpuCopy, cpu);
}

TEST_F(ArithmeticTests, ADC_AbsoluteY)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s32 NUM_CYCLES = 4;
    cpu.A = 0x12;
    cpu.Y = 0x10;

    mem[0xFFFC] = CPU::INS_ADC_ABY;
    mem[0xFFFD] = 0x20;
    mem[0xFFFE] = 0x04;
    mem[0x0430] = 0x13;

    // act:
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0x25);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
    testAllFlagsUnchanged(cpuCopy, cpu);
}

TEST_F(ArithmeticTests, ADC_IndexedIndirect)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s32 NUM_CYCLES = 6;
    cpu.A = 0x12;
    cpu.X = 0x04;

    mem[0xFFFC] = CPU::INS_ADC_INX;
    mem[0xFFFD] = 0x73;
    mem[0x0077] = 0x20;
    mem[0x0078] = 0x04;
    mem[0x0420] = 0x13;

    // act
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert
    EXPECT_EQ(cpu.A, 0x25);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

TEST_F(ArithmeticTests, ADC_IndirectIndexed)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s32 NUM_CYCLES = 5;
    cpu.A = 0x12;
    cpu.Y = 0x05;

    mem[0xFFFC] = CPU::INS_ADC_INY;
    mem[0xFFFD] = 0x73;
    mem[0x0073] = 0x20;
    mem[0x0074] = 0x04;
    mem[0x0425] = 0x13;

    // act
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert
    EXPECT_EQ(cpu.A, 0x25);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

TEST_F(ArithmeticTests, ADC_IndirectIndexed_PageCrossed)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s32 NUM_CYCLES = 6;
    cpu.A = 0x12;
    cpu.Y = 0xFF;

    mem[0xFFFC] = CPU::INS_ADC_INY;
    mem[0xFFFD] = 0x73;
    mem[0x0073] = 0x20;
    mem[0x0074] = 0x04;
    mem[0x051F] = 0x13;

    // act
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert
    EXPECT_EQ(cpu.A, 0x25);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}