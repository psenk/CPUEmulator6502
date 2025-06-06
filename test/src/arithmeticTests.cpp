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
    static void testUnaffectedFlagsUnchanged(const m6502::CPU &cpuCopy,
                                             const m6502::CPU &cpu)
    {
        EXPECT_EQ(cpuCopy.P.bits.I, cpu.P.bits.I);
        EXPECT_EQ(cpuCopy.P.bits.D, cpu.P.bits.D);
        EXPECT_EQ(cpuCopy.P.bits.B, cpu.P.bits.B);
    }

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
    static constexpr s_int32 NUM_CYCLES = 2;
    cpu.A = 0x00;
    cpu.P.bits.C = false;

    mem[0xFFFC] = CPU::INS_ADC_IMM;
    mem[0xFFFD] = 0x00;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0x00);
    EXPECT_TRUE(cpu.P.bits.Z);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
    testUnaffectedFlagsUnchanged(cpuCopy, cpu);
}

TEST_F(ArithmeticTests, ADC_ZeroResult)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 2;
    cpu.A = 0x10;
    cpu.P.bits.C = false;

    mem[0xFFFC] = CPU::INS_ADC_IMM;
    mem[0xFFFD] = 0xF0;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0x00);
    EXPECT_TRUE(cpu.P.bits.Z);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
    testUnaffectedFlagsUnchanged(cpuCopy, cpu);
}

TEST_F(ArithmeticTests, ADC_NegativeResult)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 2;
    cpu.A = 0x10;
    cpu.P.bits.C = false;

    mem[0xFFFC] = CPU::INS_ADC_IMM;
    mem[0xFFFD] = 0x84;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0x94);
    EXPECT_TRUE(cpu.P.bits.N);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
    testUnaffectedFlagsUnchanged(cpuCopy, cpu);
}

TEST_F(ArithmeticTests, ADC_CarryFlagBecomesSet)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 2;
    cpu.A = 0xFE;
    cpu.P.bits.C = false;

    mem[0xFFFC] = CPU::INS_ADC_IMM;
    mem[0xFFFD] = 0x03;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0x01);
    EXPECT_TRUE(cpu.P.bits.C);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
    testUnaffectedFlagsUnchanged(cpuCopy, cpu);
}

TEST_F(ArithmeticTests, ADC_CarryFlagRemainsSet)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 2;
    cpu.A = 0xFE;
    cpu.P.bits.C = true;

    mem[0xFFFC] = CPU::INS_ADC_IMM;
    mem[0xFFFD] = 0x02;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0x01);
    EXPECT_TRUE(cpu.P.bits.C);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
    testUnaffectedFlagsUnchanged(cpuCopy, cpu);
}

TEST_F(ArithmeticTests, ADC_OverflowFlagSet)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 2;
    cpu.A = 0x7F;

    mem[0xFFFC] = CPU::INS_ADC_IMM;
    mem[0xFFFD] = 0x02;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0x81);
    EXPECT_TRUE(cpu.P.bits.V);
    EXPECT_TRUE(cpu.P.bits.C);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
    testUnaffectedFlagsUnchanged(cpuCopy, cpu);
}

TEST_F(ArithmeticTests, ADC_FlagsSetTest)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 2;
    cpu.A = 0x00;
    cpu.P.bits.C = false;
    cpu.P.bits.Z = false;
    cpu.P.bits.V = true;
    cpu.P.bits.N = true;

    mem[0xFFFC] = CPU::INS_ADC_IMM;
    mem[0xFFFD] = 0x00;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0x00);
    EXPECT_TRUE(cpu.P.bits.Z);
    EXPECT_FALSE(cpu.P.bits.C);
    EXPECT_FALSE(cpu.P.bits.V);
    EXPECT_FALSE(cpu.P.bits.N);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
    testUnaffectedFlagsUnchanged(cpuCopy, cpu);
}

/**
 * ADD INSTRUCTIONS
 */

TEST_F(ArithmeticTests, ADC_Immediate)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 2;
    cpu.A = 0x12;

    mem[0xFFFC] = CPU::INS_ADC_IMM;
    mem[0xFFFD] = 0x13;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

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
    static constexpr s_int32 NUM_CYCLES = 3;
    cpu.A = 0x12;

    mem[0xFFFC] = CPU::INS_ADC_ZPG;
    mem[0xFFFD] = 0x42;
    mem[0x0042] = 0x13;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

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
    static constexpr s_int32 NUM_CYCLES = 4;
    cpu.A = 0x12;
    cpu.X = 0x10;

    mem[0xFFFC] = CPU::INS_ADC_ZPX;
    mem[0xFFFD] = 0x42;
    mem[0x0052] = 0x13;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

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
    static constexpr s_int32 NUM_CYCLES = 4;
    cpu.A = 0x12;

    mem[0xFFFC] = CPU::INS_ADC_ABS;
    mem[0xFFFD] = 0x20;
    mem[0xFFFE] = 0x04;
    mem[0x0420] = 0x13;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

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
    static constexpr s_int32 NUM_CYCLES = 4;
    cpu.A = 0x12;
    cpu.X = 0x10;

    mem[0xFFFC] = CPU::INS_ADC_ABX;
    mem[0xFFFD] = 0x20;
    mem[0xFFFE] = 0x04;
    mem[0x0430] = 0x13;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

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
    static constexpr s_int32 NUM_CYCLES = 5;
    cpu.A = 0x12;
    cpu.X = 0xFF;

    mem[0xFFFC] = CPU::INS_ADC_ABX;
    mem[0xFFFD] = 0x20;
    mem[0xFFFE] = 0x04;
    mem[0x051F] = 0x13;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

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
    static constexpr s_int32 NUM_CYCLES = 4;
    cpu.A = 0x12;
    cpu.Y = 0x10;

    mem[0xFFFC] = CPU::INS_ADC_ABY;
    mem[0xFFFD] = 0x20;
    mem[0xFFFE] = 0x04;
    mem[0x0430] = 0x13;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

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
    static constexpr s_int32 NUM_CYCLES = 6;
    cpu.A = 0x12;
    cpu.X = 0x04;

    mem[0xFFFC] = CPU::INS_ADC_INX;
    mem[0xFFFD] = 0x73;
    mem[0x0077] = 0x20;
    mem[0x0078] = 0x04;
    mem[0x0420] = 0x13;

    // act
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert
    EXPECT_EQ(cpu.A, 0x25);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
    testAllFlagsUnchanged(cpuCopy, cpu);
}

TEST_F(ArithmeticTests, ADC_IndirectIndexed)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 5;
    cpu.A = 0x12;
    cpu.Y = 0x05;

    mem[0xFFFC] = CPU::INS_ADC_INY;
    mem[0xFFFD] = 0x73;
    mem[0x0073] = 0x20;
    mem[0x0074] = 0x04;
    mem[0x0425] = 0x13;

    // act
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert
    EXPECT_EQ(cpu.A, 0x25);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
    testAllFlagsUnchanged(cpuCopy, cpu);
}

TEST_F(ArithmeticTests, ADC_IndirectIndexed_PageCrossed)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 6;
    cpu.A = 0x12;
    cpu.Y = 0xFF;

    mem[0xFFFC] = CPU::INS_ADC_INY;
    mem[0xFFFD] = 0x73;
    mem[0x0073] = 0x20;
    mem[0x0074] = 0x04;
    mem[0x051F] = 0x13;

    // act
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert
    EXPECT_EQ(cpu.A, 0x25);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
    testAllFlagsUnchanged(cpuCopy, cpu);
}

/**
 * SUBTRACT INSTRUCTION MISC TESTS
 */

// carry flag set = no borrow occured
// carry flag unset = borrow occured

TEST_F(ArithmeticTests, SBC_ZeroValues)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 2;
    cpu.A = 0x00;
    cpu.P.bits.C = true;

    mem[0xFFFC] = CPU::INS_SBC_IMM;
    mem[0xFFFD] = 0x00;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0x00);
    EXPECT_TRUE(cpu.P.bits.Z);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
    testUnaffectedFlagsUnchanged(cpuCopy, cpu);
}

TEST_F(ArithmeticTests, SBC_ZeroResult)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 2;
    cpu.A = 0x10;
    cpu.P.bits.C = true;

    mem[0xFFFC] = CPU::INS_SBC_IMM;
    mem[0xFFFD] = 0x10;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0x00);
    EXPECT_TRUE(cpu.P.bits.Z);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
    testUnaffectedFlagsUnchanged(cpuCopy, cpu);
}

TEST_F(ArithmeticTests, SBC_NegativeResult)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 2;
    cpu.A = 0x10;
    cpu.P.bits.C = true;

    mem[0xFFFC] = CPU::INS_SBC_IMM;
    mem[0xFFFD] = 0x84;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0x8C);
    EXPECT_TRUE(cpu.P.bits.N);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
    testUnaffectedFlagsUnchanged(cpuCopy, cpu);
}

TEST_F(ArithmeticTests, SBC_CarryFlagUnset_BecomesSet)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 2;
    cpu.A = 0x64;
    cpu.P.bits.C = false;

    mem[0xFFFC] = CPU::INS_SBC_IMM;
    mem[0xFFFD] = 0x1E;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0x45);
    EXPECT_TRUE(cpu.P.bits.C);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
    testUnaffectedFlagsUnchanged(cpuCopy, cpu);
}

TEST_F(ArithmeticTests, SBC_CarryFlagSet_RemainsSet)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 2;
    cpu.A = 0x1E;
    cpu.P.bits.C = true;

    mem[0xFFFC] = CPU::INS_SBC_IMM;
    mem[0xFFFD] = 0x14;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0x0A);
    EXPECT_TRUE(cpu.P.bits.C);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
    testUnaffectedFlagsUnchanged(cpuCopy, cpu);
}

TEST_F(ArithmeticTests, SBC_CarryFlagSet_BecomesUnset)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 2;
    cpu.A = 0x1E;
    cpu.P.bits.C = true;

    mem[0xFFFC] = CPU::INS_SBC_IMM;
    mem[0xFFFD] = 0x32;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0xEC);
    EXPECT_FALSE(cpu.P.bits.C);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
    testUnaffectedFlagsUnchanged(cpuCopy, cpu);
}

TEST_F(ArithmeticTests, SBC_CarryFlagUnset_RemainsUnset)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 2;
    cpu.A = 0x1E;
    cpu.P.bits.C = false;

    mem[0xFFFC] = CPU::INS_SBC_IMM;
    mem[0xFFFD] = 0x32;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0xEB);
    EXPECT_FALSE(cpu.P.bits.C);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
    testUnaffectedFlagsUnchanged(cpuCopy, cpu);
}

TEST_F(ArithmeticTests, SBC_OverflowFlagSet)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 2;
    cpu.A = 0x80;
    cpu.P.bits.C = true;

    mem[0xFFFC] = CPU::INS_SBC_IMM;
    mem[0xFFFD] = 0x01;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0x7F);
    EXPECT_TRUE(cpu.P.bits.V);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
    testUnaffectedFlagsUnchanged(cpuCopy, cpu);
}

TEST_F(ArithmeticTests, SBC_FlagsSetTest)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 2;
    cpu.A = 0x00;
    cpu.P.bits.C = true;
    cpu.P.bits.Z = false;
    cpu.P.bits.V = true;
    cpu.P.bits.N = true;

    mem[0xFFFC] = CPU::INS_SBC_IMM;
    mem[0xFFFD] = 0x00;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0x00);
    EXPECT_TRUE(cpu.P.bits.Z);
    EXPECT_TRUE(cpu.P.bits.C);
    EXPECT_FALSE(cpu.P.bits.V);
    EXPECT_FALSE(cpu.P.bits.N);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
    testUnaffectedFlagsUnchanged(cpuCopy, cpu);
}

/**
 * SUBTRACT INSTRUCTIONS
 */

TEST_F(ArithmeticTests, SBC_Immediate)
{
    // arrange:
    using namespace m6502;
    cpu.P.bits.C = true;
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 2;
    cpu.A = 0x13;

    mem[0xFFFC] = CPU::INS_SBC_IMM;
    mem[0xFFFD] = 0x12;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0x01);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
    testAllFlagsUnchanged(cpuCopy, cpu);
}

TEST_F(ArithmeticTests, SBC_ZeroPage)
{
    // arrange:
    using namespace m6502;
    cpu.P.bits.C = true;
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 3;
    cpu.A = 0x13;

    mem[0xFFFC] = CPU::INS_SBC_ZPG;
    mem[0xFFFD] = 0x42;
    mem[0x0042] = 0x12;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0x01);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
    testAllFlagsUnchanged(cpuCopy, cpu);
}

TEST_F(ArithmeticTests, SBC_ZeroPageX)
{
    // arrange:
    using namespace m6502;
    cpu.P.bits.C = true;
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 4;
    cpu.A = 0x13;
    cpu.X = 0x10;

    mem[0xFFFC] = CPU::INS_SBC_ZPX;
    mem[0xFFFD] = 0x42;
    mem[0x0052] = 0x12;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0x01);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
    testAllFlagsUnchanged(cpuCopy, cpu);
}

TEST_F(ArithmeticTests, SBC_Absolute)
{
    // arrange:
    using namespace m6502;
    cpu.P.bits.C = true;
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 4;
    cpu.A = 0x13;

    mem[0xFFFC] = CPU::INS_SBC_ABS;
    mem[0xFFFD] = 0x20;
    mem[0xFFFE] = 0x04;
    mem[0x0420] = 0x12;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0x01);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
    testAllFlagsUnchanged(cpuCopy, cpu);
}

TEST_F(ArithmeticTests, SBC_AbsoluteX)
{
    // arrange:
    using namespace m6502;
    cpu.P.bits.C = true;
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 4;
    cpu.A = 0x13;
    cpu.X = 0x10;

    mem[0xFFFC] = CPU::INS_SBC_ABX;
    mem[0xFFFD] = 0x20;
    mem[0xFFFE] = 0x04;
    mem[0x0430] = 0x12;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0x01);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
    testAllFlagsUnchanged(cpuCopy, cpu);
}

TEST_F(ArithmeticTests, SBC_AbsoluteX_PageCrossed)
{
    // arrange:
    using namespace m6502;
    cpu.P.bits.C = true;
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 5;
    cpu.A = 0x13;
    cpu.X = 0xFF;

    mem[0xFFFC] = CPU::INS_SBC_ABX;
    mem[0xFFFD] = 0x20;
    mem[0xFFFE] = 0x04;
    mem[0x051F] = 0x12;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0x01);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
    testAllFlagsUnchanged(cpuCopy, cpu);
}

TEST_F(ArithmeticTests, SBC_AbsoluteY)
{
    // arrange:
    using namespace m6502;
    cpu.P.bits.C = true;
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 4;
    cpu.A = 0x13;
    cpu.Y = 0x10;

    mem[0xFFFC] = CPU::INS_SBC_ABY;
    mem[0xFFFD] = 0x20;
    mem[0xFFFE] = 0x04;
    mem[0x0430] = 0x12;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0x01);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
    testAllFlagsUnchanged(cpuCopy, cpu);
}

TEST_F(ArithmeticTests, SBC_IndexedIndirect)
{
    // arrange:
    using namespace m6502;
    cpu.P.bits.C = true;
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 6;
    cpu.A = 0x13;
    cpu.X = 0x04;

    mem[0xFFFC] = CPU::INS_SBC_INX;
    mem[0xFFFD] = 0x73;
    mem[0x0077] = 0x20;
    mem[0x0078] = 0x04;
    mem[0x0420] = 0x12;

    // act
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert
    EXPECT_EQ(cpu.A, 0x01);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
    testAllFlagsUnchanged(cpuCopy, cpu);
}

TEST_F(ArithmeticTests, SBC_IndirectIndexed)
{
    // arrange:
    using namespace m6502;
    cpu.P.bits.C = true;
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 5;
    cpu.A = 0x13;
    cpu.Y = 0x05;

    mem[0xFFFC] = CPU::INS_SBC_INY;
    mem[0xFFFD] = 0x73;
    mem[0x0073] = 0x20;
    mem[0x0074] = 0x04;
    mem[0x0425] = 0x12;

    // act
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert
    EXPECT_EQ(cpu.A, 0x01);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
    testAllFlagsUnchanged(cpuCopy, cpu);
}

TEST_F(ArithmeticTests, SBC_IndirectIndexed_PageCrossed)
{
    // arrange:
    using namespace m6502;
    cpu.P.bits.C = true;
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 6;
    cpu.A = 0x13;
    cpu.Y = 0xFF;

    mem[0xFFFC] = CPU::INS_SBC_INY;
    mem[0xFFFD] = 0x73;
    mem[0x0073] = 0x20;
    mem[0x0074] = 0x04;
    mem[0x051F] = 0x12;

    // act
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert
    EXPECT_EQ(cpu.A, 0x01);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
    testAllFlagsUnchanged(cpuCopy, cpu);
}

/**
 * COMPARE INSTRUCTION MISC TESTS
 */

TEST_F(ArithmeticTests, CMP_ZeroValues)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 2;
    cpu.A = 0x00;
    cpu.P.bits.C = false;

    mem[0xFFFC] = CPU::INS_CMP_IMM;
    mem[0xFFFD] = 0x00;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0x00);
    EXPECT_TRUE(cpu.P.bits.Z);
    EXPECT_TRUE(cpu.P.bits.C);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
    testUnaffectedFlagsUnchanged(cpuCopy, cpu);
}

TEST_F(ArithmeticTests, CMP_ZeroResult)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 2;
    cpu.A = 0x10;
    cpu.P.bits.C = false;

    mem[0xFFFC] = CPU::INS_CMP_IMM;
    mem[0xFFFD] = 0x10;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0x10);
    EXPECT_TRUE(cpu.P.bits.Z);
    EXPECT_TRUE(cpu.P.bits.C);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
    testUnaffectedFlagsUnchanged(cpuCopy, cpu);
}

TEST_F(ArithmeticTests, CMP_NegativeResult)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 2;
    cpu.A = 0x10;
    cpu.P.bits.C = false;

    mem[0xFFFC] = CPU::INS_CMP_IMM;
    mem[0xFFFD] = 0x84;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0x10);
    EXPECT_TRUE(cpu.P.bits.N);
    EXPECT_FALSE(cpu.P.bits.C);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
    testUnaffectedFlagsUnchanged(cpuCopy, cpu);
}

TEST_F(ArithmeticTests, CMP_CarryFlagBecomesSet)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 2;
    cpu.A = 0x10;
    cpu.P.bits.C = false;

    mem[0xFFFC] = CPU::INS_CMP_IMM;
    mem[0xFFFD] = 0x00;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0x10);
    EXPECT_TRUE(cpu.P.bits.C);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
    testUnaffectedFlagsUnchanged(cpuCopy, cpu);
}

TEST_F(ArithmeticTests, CMP_CarryFlagBecomesUnset)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 2;
    cpu.A = 0x00;
    cpu.P.bits.C = true;

    mem[0xFFFC] = CPU::INS_CMP_IMM;
    mem[0xFFFD] = 0x10;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0x00);
    EXPECT_FALSE(cpu.P.bits.C);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
    testUnaffectedFlagsUnchanged(cpuCopy, cpu);
}

TEST_F(ArithmeticTests, CMP_FlagsSetTest)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 2;
    cpu.A = 0x00;
    cpu.P.bits.C = false;
    cpu.P.bits.Z = false;
    cpu.P.bits.N = true;

    mem[0xFFFC] = CPU::INS_CMP_IMM;
    mem[0xFFFD] = 0x00;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0x00);
    EXPECT_TRUE(cpu.P.bits.Z);
    EXPECT_TRUE(cpu.P.bits.C);
    EXPECT_FALSE(cpu.P.bits.N);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
    testUnaffectedFlagsUnchanged(cpuCopy, cpu);
}

/**
 * COMPARE INSTRUCTIONS
 */

/**
 * COMPARE ACCUMULATOR
 */

TEST_F(ArithmeticTests, CMP_Immediate)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 2;
    cpu.P.bits.C = false;
    cpu.A = 0x10;

    mem[0xFFFC] = CPU::INS_CMP_IMM;
    mem[0xFFFD] = 0x00;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0x10);
    EXPECT_TRUE(cpu.P.bits.C);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

TEST_F(ArithmeticTests, CMP_ZeroPage)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 3;
    cpu.P.bits.C = false;
    cpu.A = 0x10;

    mem[0xFFFC] = CPU::INS_CMP_ZPG;
    mem[0xFFFD] = 0x42;
    mem[0x0042] = 0x00;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0x10);
    EXPECT_TRUE(cpu.P.bits.C);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

TEST_F(ArithmeticTests, CMP_ZeroPageX)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 4;
    cpu.P.bits.C = false;
    cpu.A = 0x10;
    cpu.X = 0x10;

    mem[0xFFFC] = CPU::INS_CMP_ZPX;
    mem[0xFFFD] = 0x42;
    mem[0x0052] = 0x00;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0x10);
    EXPECT_TRUE(cpu.P.bits.C);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

TEST_F(ArithmeticTests, CMP_Absolute)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 4;
    cpu.P.bits.C = false;
    cpu.A = 0x10;

    mem[0xFFFC] = CPU::INS_CMP_ABS;
    mem[0xFFFD] = 0x20;
    mem[0xFFFE] = 0x04;
    mem[0x0420] = 0x00;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0x10);
    EXPECT_TRUE(cpu.P.bits.C);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

TEST_F(ArithmeticTests, CMP_AbsoluteX)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 4;
    cpu.P.bits.C = false;
    cpu.A = 0x10;
    cpu.X = 0x10;

    mem[0xFFFC] = CPU::INS_CMP_ABX;
    mem[0xFFFD] = 0x20;
    mem[0xFFFE] = 0x04;
    mem[0x0430] = 0x00;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0x10);
    EXPECT_TRUE(cpu.P.bits.C);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

TEST_F(ArithmeticTests, CMP_AbsoluteX_PageCrossed)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 5;
    cpu.P.bits.C = false;
    cpu.A = 0x10;
    cpu.X = 0xFF;

    mem[0xFFFC] = CPU::INS_CMP_ABX;
    mem[0xFFFD] = 0x20;
    mem[0xFFFE] = 0x04;
    mem[0x051F] = 0x00;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0x10);
    EXPECT_TRUE(cpu.P.bits.C);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

TEST_F(ArithmeticTests, CMP_AbsoluteY)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 4;
    cpu.P.bits.C = false;
    cpu.A = 0x10;
    cpu.Y = 0x10;

    mem[0xFFFC] = CPU::INS_CMP_ABY;
    mem[0xFFFD] = 0x20;
    mem[0xFFFE] = 0x04;
    mem[0x0430] = 0x00;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0x10);
    EXPECT_TRUE(cpu.P.bits.C);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

TEST_F(ArithmeticTests, CMP_IndexedIndirect)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 6;
    cpu.P.bits.C = false;
    cpu.A = 0x10;
    cpu.X = 0x04;

    mem[0xFFFC] = CPU::INS_CMP_INX;
    mem[0xFFFD] = 0x73;
    mem[0x0077] = 0x20;
    mem[0x0078] = 0x04;
    mem[0x0420] = 0x00;

    // act
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert
    EXPECT_EQ(cpu.A, 0x10);
    EXPECT_TRUE(cpu.P.bits.C);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

TEST_F(ArithmeticTests, CMP_IndirectIndexed)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 5;
    cpu.P.bits.C = false;
    cpu.A = 0x10;
    cpu.Y = 0x05;

    mem[0xFFFC] = CPU::INS_CMP_INY;
    mem[0xFFFD] = 0x73;
    mem[0x0073] = 0x20;
    mem[0x0074] = 0x04;
    mem[0x0425] = 0x00;

    // act
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert
    EXPECT_EQ(cpu.A, 0x10);
    EXPECT_TRUE(cpu.P.bits.C);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

TEST_F(ArithmeticTests, CMP_IndirectIndexed_PageCrossed)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 6;
    cpu.P.bits.C = false;
    cpu.A = 0x10;
    cpu.Y = 0xFF;

    mem[0xFFFC] = CPU::INS_CMP_INY;
    mem[0xFFFD] = 0x73;
    mem[0x0073] = 0x20;
    mem[0x0074] = 0x04;
    mem[0x051F] = 0x00;

    // act
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert
    EXPECT_EQ(cpu.A, 0x10);
    EXPECT_TRUE(cpu.P.bits.C);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

/**
 * COMPARE X REGISTER
 */

TEST_F(ArithmeticTests, CPX_Immediate)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 2;
    cpu.P.bits.C = false;
    cpu.X = 0x00;

    mem[0xFFFC] = CPU::INS_CPX_IMM;
    mem[0xFFFD] = 0x00;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.X, 0x00);
    EXPECT_TRUE(cpu.P.bits.C);
    EXPECT_TRUE(cpu.P.bits.Z);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

TEST_F(ArithmeticTests, CPX_ZeroPage)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 3;
    cpu.P.bits.C = false;
    cpu.X = 0x00;

    mem[0xFFFC] = CPU::INS_CPX_ZPG;
    mem[0xFFFD] = 0x42;
    mem[0x0042] = 0x00;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.X, 0x00);
    EXPECT_TRUE(cpu.P.bits.C);
    EXPECT_TRUE(cpu.P.bits.Z);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

TEST_F(ArithmeticTests, CPX_Absolute)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 4;
    cpu.P.bits.C = false;
    cpu.X = 0x00;

    mem[0xFFFC] = CPU::INS_CPX_ABS;
    mem[0xFFFD] = 0x20;
    mem[0xFFFE] = 0x04;
    mem[0x0420] = 0x00;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.X, 0x00);
    EXPECT_TRUE(cpu.P.bits.C);
    EXPECT_TRUE(cpu.P.bits.Z);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

/**
 * COMPARE Y REGISTER
 */

TEST_F(ArithmeticTests, CPY_Immediate)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 2;
    cpu.P.bits.C = false;
    cpu.Y = 0x00;

    mem[0xFFFC] = CPU::INS_CPY_IMM;
    mem[0xFFFD] = 0x00;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.Y, 0x00);
    EXPECT_TRUE(cpu.P.bits.C);
    EXPECT_TRUE(cpu.P.bits.Z);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

TEST_F(ArithmeticTests, CPY_ZeroPage)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 3;
    cpu.P.bits.C = false;
    cpu.Y = 0x00;

    mem[0xFFFC] = CPU::INS_CPY_ZPG;
    mem[0xFFFD] = 0x42;
    mem[0x0042] = 0x00;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.Y, 0x00);
    EXPECT_TRUE(cpu.P.bits.C);
    EXPECT_TRUE(cpu.P.bits.Z);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

TEST_F(ArithmeticTests, CPY_Absolute)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 4;
    cpu.P.bits.C = false;
    cpu.Y = 0x00;

    mem[0xFFFC] = CPU::INS_CPY_ABS;
    mem[0xFFFD] = 0x20;
    mem[0xFFFE] = 0x04;
    mem[0x0420] = 0x00;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.Y, 0x00);
    EXPECT_TRUE(cpu.P.bits.C);
    EXPECT_TRUE(cpu.P.bits.Z);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

// todo: copy for cpy
// todo: write instruction code