#include <gtest/gtest.h>
#include "m6502.h"

class StackOperationsTests : public ::testing::Test
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
    static void testStackFlagsUnchanged(const m6502::CPU &cpuCopy,
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

/**
 * TRANSFER STACK POINTER TO X TESTING
 */

TEST_F(StackOperationsTests, TSX_TransferSPToX)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s32 NUM_CYCLES = 2;
    cpu.SP = 0x42;

    mem[0xFFFC] = CPU::INS_TSX;

    // act:
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.X, 0x42);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
    testStackFlagsUnchanged(cpuCopy, cpu);
}

TEST_F(StackOperationsTests, TSX_TransferSPToX_ZeroValue)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s32 NUM_CYCLES = 2;
    cpu.SP = 0x00;

    mem[0xFFFC] = CPU::INS_TSX;

    // act:
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.X, 0x00);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
    EXPECT_TRUE(cpu.P.bits.Z);
    testStackFlagsUnchanged(cpuCopy, cpu);
}

TEST_F(StackOperationsTests, TSX_TransferSPToX_NegativeValue)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s32 NUM_CYCLES = 2;
    cpu.SP = 0x84;

    mem[0xFFFC] = CPU::INS_TSX;

    // act:
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.X, 0x84);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
    EXPECT_TRUE(cpu.P.bits.N);
    testStackFlagsUnchanged(cpuCopy, cpu);
}

/**
 * TRANSFER X REGISTER TO STACK POINTER
 */

TEST_F(StackOperationsTests, TXS_TransferXToSP)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s32 NUM_CYCLES = 2;
    cpu.X = 0x42;

    mem[0xFFFC] = CPU::INS_TXS;

    // act:
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.SP, 0x42);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
    testAllFlagsUnchanged(cpuCopy, cpu);
}

/**
 * PUSH ACCUMULATOR TO STACK
 */

TEST_F(StackOperationsTests, PHA_PushAToStack)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s32 NUM_CYCLES = 3;
    cpu.A = 0x42;

    mem[0xFFFC] = CPU::INS_PHA;

    // act:
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(mem[0x0100 + cpu.SP + 1], 0x42);
    EXPECT_EQ(cpu.SP, 0xFE);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
    testAllFlagsUnchanged(cpuCopy, cpu);
}

/**
 * PUSH PROCESSOR STATUS TO STACK
 */

TEST_F(StackOperationsTests, PHP_PushPStatusToStack)
{
    // arrange:
    using namespace m6502;
    static constexpr s32 NUM_CYCLES = 3;
    cpu.P.value = 0x6B;
    CPU cpuCopy = cpu;

    mem[0xFFFC] = CPU::INS_PHP;

    // act:
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(mem[0x0100 + cpu.SP + 1], 0x6B);
    EXPECT_EQ(cpu.SP, 0xFE);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
    testAllFlagsUnchanged(cpuCopy, cpu);
}

/**
 * PULL FROM STACK TO ACCUMULATOR
 */

TEST_F(StackOperationsTests, PLA_PullFromStackToAccumulator)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s32 NUM_CYCLES = 4;
    cpu.SP = 0xFE;

    mem[0xFFFC] = CPU::INS_PLA;
    mem[0x01FF] = 0x42;

    // act:
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0x42);
    EXPECT_EQ(cpu.SP, 0xFF);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
    testAllFlagsUnchanged(cpuCopy, cpu);
}

TEST_F(StackOperationsTests, PLA_PullFromStackToAccumulator_ZeroValue)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s32 NUM_CYCLES = 4;
    cpu.SP = 0xFE;

    mem[0xFFFC] = CPU::INS_PLA;
    mem[0x01FF] = 0x00;

    // act:
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0x00);
    EXPECT_EQ(cpu.SP, 0xFF);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
    EXPECT_TRUE(cpu.P.bits.Z);
    testStackFlagsUnchanged(cpuCopy, cpu);
}

TEST_F(StackOperationsTests, PLA_PullFromStackToAccumulator_NegativeValue)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s32 NUM_CYCLES = 4;
    cpu.SP = 0xFE;

    mem[0xFFFC] = CPU::INS_PLA;
    mem[0x01FF] = 0x84;

    // act:
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0x84);
    EXPECT_EQ(cpu.SP, 0xFF);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
    EXPECT_TRUE(cpu.P.bits.N);
    testStackFlagsUnchanged(cpuCopy, cpu);
}

/**
 * PULL PROCESSOR STATUS FROM STACK
 */

TEST_F(StackOperationsTests, PLP_PullPStatusFromStack)
{
    // arrange:
    using namespace m6502;
    static constexpr s32 NUM_CYCLES = 4;
    cpu.SP = 0xFC;
    CPU cpuCopy = cpu;

    mem[0xFFFC] = CPU::INS_PLP;
    mem[0x01FD] = 0x6B;

    // act:
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.P.value, 0x6B);
    EXPECT_EQ(cpu.SP, 0xFD);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}