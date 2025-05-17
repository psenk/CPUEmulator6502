#include <gtest/gtest.h>
#include "m6502.h"

class CPUTests : public ::testing::Test
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
};

static void testFullCPUUnchanged(const m6502::CPU &cpuCopy,
                                 const m6502::CPU &cpu)
{
    EXPECT_EQ(cpuCopy.PC, cpu.PC);
    EXPECT_EQ(cpuCopy.SP, cpu.SP);
    EXPECT_EQ(cpuCopy.A, cpu.A);
    EXPECT_EQ(cpuCopy.X, cpu.X);
    EXPECT_EQ(cpuCopy.Y, cpu.Y);
    EXPECT_EQ(cpuCopy.C, cpu.C);
    EXPECT_EQ(cpuCopy.Z, cpu.Z);
    EXPECT_EQ(cpuCopy.I, cpu.I);
    EXPECT_EQ(cpuCopy.D, cpu.D);
    EXPECT_EQ(cpuCopy.B, cpu.B);
    EXPECT_EQ(cpuCopy.V, cpu.V);
    EXPECT_EQ(cpuCopy.N, cpu.N);
}

TEST_F(CPUTests, CPUDoesNothingWithZeroCyclesExecuted)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s32 NUM_CYCLES = 0;

    // act:
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cyclesExecuted, 0);
    testFullCPUUnchanged(cpuCopy, cpu);
}

TEST_F(CPUTests, CPUNotGivenEnoughCyclesForInstruction)
{
    // arrange
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s32 NUM_CYCLES = 1;

    mem[0xFFFC] = CPU::INS_LDA_IMM;
    mem[0xFFFD] = 0x42;
    mem[0x0042] = 0x00;

    // act:
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cyclesExecuted, 2);
    EXPECT_EQ(mem[0x0042], 0x00);
}

TEST_F(CPUTests, CPUExecutesBadFunction)
{
    // arrange
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s32 NUM_CYCLES = 1;

    mem[0xFFFC] = 0x02; // invalid opcode

    // act:
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cyclesExecuted, -1);
    testFullCPUUnchanged(cpuCopy, cpu);
}
