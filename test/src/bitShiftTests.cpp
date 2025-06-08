#include <gtest/gtest.h>
#include "m6502.h"

class BitShiftTests : public ::testing::Test
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
 * FLAG TESTS
 */

TEST_F(BitShiftTests, Shift_CarrySet)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 2;
    cpu.A = 0xC0;
    cpu.P.bits.C = false;

    mem[0xFFFC] = CPU::INS_ASL_ACC;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0x80);
    EXPECT_TRUE(cpu.P.bits.C);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

TEST_F(BitShiftTests, Shift_CarryNotSet)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 2;
    cpu.A = 0x01;

    mem[0xFFFC] = CPU::INS_ASL_ACC;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0x02);
    EXPECT_FALSE(cpu.P.bits.C);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

TEST_F(BitShiftTests, Shift_CarryRemainsSet)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 2;
    cpu.A = 0xC0;
    cpu.P.bits.C = true;

    mem[0xFFFC] = CPU::INS_ASL_ACC;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0x80);
    EXPECT_TRUE(cpu.P.bits.C);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

TEST_F(BitShiftTests, Shift_ZeroResult)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 2;
    cpu.A = 0x80;
    cpu.P.bits.Z = false;

    mem[0xFFFC] = CPU::INS_ASL_ACC;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0x00);
    EXPECT_TRUE(cpu.P.bits.Z);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

TEST_F(BitShiftTests, Shift_NegativeResult)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 2;
    cpu.A = 0x80;
    cpu.P.bits.Z = false;

    mem[0xFFFC] = CPU::INS_ASL_ACC;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0x00);
    EXPECT_TRUE(cpu.P.bits.Z);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

/**
 * ARITHMETIC SHIFT LEFT TESTS
 */

TEST_F(BitShiftTests, ASL_Accumulator)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 2;
    cpu.A = 0x01;

    mem[0xFFFC] = CPU::INS_ASL_ACC;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0x02);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

TEST_F(BitShiftTests, ASL_ZeroPage)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 5;

    mem[0xFFFC] = CPU::INS_ASL_ZPG;
    mem[0xFFFD] = 0x42;
    mem[0x0042] = 0x01;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(mem[0x0042], 0x02);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

TEST_F(BitShiftTests, ASL_ZeroPageX)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    cpu.X = 0x10;
    static constexpr s_int32 NUM_CYCLES = 6;

    mem[0xFFFC] = CPU::INS_ASL_ZPX;
    mem[0xFFFD] = 0x42;
    mem[0x0052] = 0x01;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(mem[0x0052], 0x02);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

TEST_F(BitShiftTests, ASL_Absolute)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 6;

    mem[0xFFFC] = CPU::INS_ASL_ABS;
    mem[0xFFFD] = 0x20;
    mem[0xFFFE] = 0x04;
    mem[0x0420] = 0x01;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(mem[0x0420], 0x02);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

TEST_F(BitShiftTests, ASL_AbsoluteX)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    cpu.X = 0x10;
    static constexpr s_int32 NUM_CYCLES = 7;

    mem[0xFFFC] = CPU::INS_ASL_ABX;
    mem[0xFFFD] = 0x20;
    mem[0xFFFE] = 0x04;
    mem[0x0430] = 0x01;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(mem[0x0430], 0x02);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

/**
 * ARITHMETIC SHIFT LEFT TESTS
 */

TEST_F(BitShiftTests, LSR_Accumulator)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 2;
    cpu.A = 0x02;
    cpu.P.bits.C = false;

    mem[0xFFFC] = CPU::INS_LSR_ACC;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0x01);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

TEST_F(BitShiftTests, LSR_ZeroPage)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 5;

    mem[0xFFFC] = CPU::INS_LSR_ZPG;
    mem[0xFFFD] = 0x42;
    mem[0x0042] = 0x02;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(mem[0x0042], 0x01);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

TEST_F(BitShiftTests, LSR_ZeroPageX)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    cpu.X = 0x10;
    static constexpr s_int32 NUM_CYCLES = 6;

    mem[0xFFFC] = CPU::INS_LSR_ZPX;
    mem[0xFFFD] = 0x42;
    mem[0x0052] = 0x02;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(mem[0x0052], 0x01);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

TEST_F(BitShiftTests, LSR_Absolute)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 6;

    mem[0xFFFC] = CPU::INS_LSR_ABS;
    mem[0xFFFD] = 0x20;
    mem[0xFFFE] = 0x04;
    mem[0x0420] = 0x02;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(mem[0x0420], 0x01);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

TEST_F(BitShiftTests, LSR_AbsoluteX)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    cpu.X = 0x10;
    static constexpr s_int32 NUM_CYCLES = 7;

    mem[0xFFFC] = CPU::INS_LSR_ABX;
    mem[0xFFFD] = 0x20;
    mem[0xFFFE] = 0x04;
    mem[0x0430] = 0x02;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(mem[0x0430], 0x01);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}
