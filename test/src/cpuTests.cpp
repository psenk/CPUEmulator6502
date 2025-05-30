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

    static void testFullCPUUnchanged(const m6502::CPU &cpuCopy,
                                     const m6502::CPU &cpu)
    {
        EXPECT_EQ(cpuCopy.PC, cpu.PC);
        EXPECT_EQ(cpuCopy.SP, cpu.SP);
        EXPECT_EQ(cpuCopy.A, cpu.A);
        EXPECT_EQ(cpuCopy.X, cpu.X);
        EXPECT_EQ(cpuCopy.Y, cpu.Y);
        EXPECT_EQ(cpuCopy.P.value, cpu.P.value);
    }
    
    static void testAllFlagsUnchanged(const m6502::CPU &cpuCopy,
                                     const m6502::CPU &cpu)
    {
        EXPECT_EQ(cpuCopy.P.value, cpu.P.value);
    }
};

TEST_F(CPUTests, CPUDoesNothingWithZeroCycles)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s32 NUM_CYCLES = 0;

    // act:
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
    testFullCPUUnchanged(cpuCopy, cpu);
}

TEST_F(CPUTests, CPUNotGivenEnoughCycles)
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

TEST_F(CPUTests, CPUExecutesBadInstruction)
{
    // arrange
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s32 NUM_CYCLES = 1;

    mem[0xFFFC] = 0x02; // invalid opcode

    // act/assert:
    EXPECT_THROW(cpu.execute(NUM_CYCLES, mem), std::runtime_error);
    testFullCPUUnchanged(cpuCopy, cpu);
}

TEST_F(CPUTests, CPULoadProgram)
{
    // arrange:
    using namespace m6502;
    const u32 numBytes = 10;
    Byte testProgram[numBytes] = {0x20, 0x04, 0x02, 0x03, 0x04,
                                  0x05, 0x06, 0x07, 0x08, 0x09};

    // act:
    cpu.loadProgram(testProgram, numBytes, mem);

    // assert:
    EXPECT_EQ(mem[0x0420], 0x02);
    EXPECT_EQ(mem[0x0421], 0x03);
    EXPECT_EQ(mem[0x0422], 0x04);
    EXPECT_EQ(mem[0x0423], 0x05);
    EXPECT_EQ(mem[0x0424], 0x06);
    EXPECT_EQ(mem[0x0425], 0x07);
    EXPECT_EQ(mem[0x0426], 0x08);
    EXPECT_EQ(mem[0x0427], 0x09);
}

TEST_F(CPUTests, CPULoadProgram_InvalidProgram)
{
    // arrange:
    using namespace m6502;
    const u32 numBytes = 2;

    // act/assert:
    EXPECT_THROW(cpu.loadProgram(NULL, numBytes, mem), std::invalid_argument);
}

TEST_F(CPUTests, CPULoadProgram_ProgramTooSmall)
{
    // arrange:
    using namespace m6502;
    const u32 numBytes = 2;
    Byte testProgram[numBytes] = {0x20, 0x40};

    // act/assert:
    EXPECT_THROW(cpu.loadProgram(testProgram, numBytes, mem), std::invalid_argument);
}

TEST_F(CPUTests, CPURunProgram)
{
    // arrange:
    using namespace m6502;
    const u32 numBytes = 7;
    Byte testProgram[numBytes] = {0x20, 0x04, 0xA9, 0x42, 0xB4,
                                  0x50, 0x00};
    Word address = cpu.loadProgram(testProgram, numBytes, mem);
    cpu.PC = address;
    cpu.X = 0x10;
    Word brkAddress = address + 4;
    mem[0x0060] = 0x42;
    mem[0xFFFE] = brkAddress & 0xFF;
    mem[0xFFFF] = brkAddress >> 8;

    // act:
    for (s32 clock = 100; clock > 0;)
    {
        clock -= cpu.execute(clock, mem);
        if (cpu.A == 0x42 && cpu.Y == 0x42)
            break;
    }

    // assert:
    EXPECT_EQ(cpu.A, 0x42);
    EXPECT_EQ(cpu.Y, 0x42);
}

TEST_F(CPUTests, NOPInstruction)
{
    // arrange
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s32 NUM_CYCLES = 2;

    mem[0xFFFC] = CPU::INS_NOP;

    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // act/assert:
    EXPECT_EQ(cpu.PC, 0xFFFD);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
    testAllFlagsUnchanged(cpuCopy, cpu);
}