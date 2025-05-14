#include <gtest/gtest.h>
#include "main_6502.h"

// derived class
// :: - start at global scope
// testing - namespace that contains Test class
// ::Test - Test class within testing namespace
class CPUTest : public ::testing::Test
{
protected:
    Mem mem;
    CPU cpu;

    void SetUp()
    {
        cpu.reset(mem);
    }

    void TearDown()
    {
    }
};

static void LDAFlags_TestUnchanged(CPU &cpuCopy, CPU &cpu)
{
    EXPECT_EQ(cpuCopy.C, cpu.C);
    EXPECT_EQ(cpuCopy.I, cpu.I);
    EXPECT_EQ(cpuCopy.D, cpu.D);
    EXPECT_EQ(cpuCopy.B, cpu.B);
    EXPECT_EQ(cpuCopy.V, cpu.V);
}

static void CPU_TestFullCPUUnchanged(CPU &cpuCopy, CPU &cpu)
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

TEST_F(CPUTest, CPUDoesNothingWithZeroCyclesExecuted)
{
    // arrange:
    CPU cpuCopy = cpu;
    static constexpr s32 NUM_CYCLES = 0;

    // act:
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cyclesExecuted, 0);
    LDAFlags_TestUnchanged(cpuCopy, cpu);
}

TEST_F(CPUTest, CPUNotGivenEnoughCyclesForInstruction)
{
    // arrange
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
    LDAFlags_TestUnchanged(cpuCopy, cpu);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
}

TEST_F(CPUTest, CPUExecutesABadFunction)
{
    // arrange
    CPU cpuCopy = cpu;
    static constexpr s32 NUM_CYCLES = 1;

    mem[0xFFFC] = 0x02; // invalid opcode

    // act:
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cyclesExecuted, -1);
    CPU_TestFullCPUUnchanged(cpuCopy, cpu);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
}

TEST_F(CPUTest, LDAImmediate_LoadValueIntoARegister)
{
    // arrange:
    CPU cpuCopy = cpu;
    static constexpr s32 NUM_CYCLES = 2;

    mem[0xFFFC] = CPU::INS_LDA_IMM;
    mem[0xFFFD] = 0x42;

    // act:
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0x42);
    EXPECT_EQ(cyclesExecuted, 2);
    LDAFlags_TestUnchanged(cpuCopy, cpu);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
}

TEST_F(CPUTest, LDAImmediate_LoadNegativeValueIntoARegister)
{
    // arrange:
    CPU cpuCopy = cpu;
    static constexpr s32 NUM_CYCLES = 2;

    mem[0xFFFC] = CPU::INS_LDA_IMM;
    mem[0xFFFD] = 0x84;

    // act:
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0x84);
    EXPECT_EQ(cyclesExecuted, 2);
    LDAFlags_TestUnchanged(cpuCopy, cpu);
    EXPECT_FALSE(cpu.Z);
    EXPECT_TRUE(cpu.N);
}

TEST_F(CPUTest, LDAImmediate_LoadZeroIntoARegister)
{
    // arrange:
    CPU cpuCopy = cpu;
    static constexpr s32 NUM_CYCLES = 2;

    mem[0xFFFC] = CPU::INS_LDA_IMM;
    mem[0xFFFD] = 0x00;

    // act:
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0x0);
    EXPECT_EQ(cyclesExecuted, 2);
    LDAFlags_TestUnchanged(cpuCopy, cpu);
    EXPECT_TRUE(cpu.Z);
    EXPECT_FALSE(cpu.N);
}

TEST_F(CPUTest, LDAZeroPage_LoadValueIntoARegister)
{
    // arrange:
    CPU cpuCopy = cpu;
    static constexpr s32 NUM_CYCLES = 3;

    mem[0xFFFC] = CPU::INS_LDA_ZPG;
    mem[0xFFFD] = 0x73;
    mem[0x0073] = 0x42;

    // act:
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0x42);
    EXPECT_EQ(cyclesExecuted, 3);
    LDAFlags_TestUnchanged(cpuCopy, cpu);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
}

TEST_F(CPUTest, LDAZeroPageX_LoadValueIntoARegister)
{
    // arrange:
    CPU cpuCopy = cpu;
    static constexpr s32 NUM_CYCLES = 4;
    cpu.X = 0x05;

    mem[0xFFFC] = CPU::INS_LDA_ZPX;
    mem[0xFFFD] = 0x73;
    mem[0x0078] = 0x42;

    // act:
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0x42);
    EXPECT_EQ(cyclesExecuted, 4);
    LDAFlags_TestUnchanged(cpuCopy, cpu);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
}

TEST_F(CPUTest, LDAZeroPageX_LoadValueIntoARegister_WrapAround)
{
    // arrange:
    CPU cpuCopy = cpu;
    static constexpr s32 NUM_CYCLES = 4;
    cpu.X = 0xFF;

    mem[0xFFFC] = CPU::INS_LDA_ZPX;
    mem[0xFFFD] = 0x80;
    mem[0x007F] = 0x73;

    // act:
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.A, 0x73);
    EXPECT_EQ(cyclesExecuted, 4);
    LDAFlags_TestUnchanged(cpuCopy, cpu);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
}

TEST_F(CPUTest, LDAAbsolute_LoadValueIntoARegister)
{
    // arrange
    CPU cpuCopy = cpu;
    static constexpr s32 NUM_CYCLES = 4;

    mem[0xFFFC] = CPU::INS_LDA_ABS;
    mem[0xFFFD] = 0x20;
    mem[0xFFFE] = 0x04;
    mem[0x0420] = 0x42;

    // act
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert
    EXPECT_EQ(cpu.A, 0x42);
    EXPECT_EQ(cyclesExecuted, 4);
    LDAFlags_TestUnchanged(cpuCopy, cpu);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
}

TEST_F(CPUTest, LDAAbsoluteX_LoadValueIntoARegister)
{
    // arrange
    CPU cpuCopy = cpu;
    static constexpr s32 NUM_CYCLES = 4;
    cpu.X = 0x05;

    mem[0xFFFC] = CPU::INS_LDA_ABX;
    mem[0xFFFD] = 0x20;
    mem[0xFFFE] = 0x04;
    mem[0x0425] = 0x42;

    // act
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert
    EXPECT_EQ(cpu.A, 0x42);
    EXPECT_EQ(cyclesExecuted, 4);
    LDAFlags_TestUnchanged(cpuCopy, cpu);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
}

TEST_F(CPUTest, LDAAbsoluteX_LoadValueIntoARegisterPageCrossed)
{
    // arrange
    CPU cpuCopy = cpu;
    static constexpr s32 NUM_CYCLES = 5;
    cpu.X = 0x00FF;

    mem[0xFFFC] = CPU::INS_LDA_ABX;
    mem[0xFFFD] = 0x20;
    mem[0xFFFE] = 0x04;
    mem[0x051F] = 0x42;

    // act
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert
    EXPECT_EQ(cpu.A, 0x42);
    EXPECT_EQ(cyclesExecuted, 5);
    LDAFlags_TestUnchanged(cpuCopy, cpu);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
}

TEST_F(CPUTest, LDAAbsoluteY_LoadValueIntoARegister)
{
    // arrange
    CPU cpuCopy = cpu;
    static constexpr s32 NUM_CYCLES = 4;
    cpu.Y = 0x05;

    mem[0xFFFC] = CPU::INS_LDA_ABY;
    mem[0xFFFD] = 0x20;
    mem[0xFFFE] = 0x04;
    mem[0x0425] = 0x42;

    // act
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert
    EXPECT_EQ(cpu.A, 0x42);
    EXPECT_EQ(cyclesExecuted, 4);
    LDAFlags_TestUnchanged(cpuCopy, cpu);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
}

TEST_F(CPUTest, LDAAbsoluteY_LoadValueIntoARegisterPageCrossed)
{
    // arrange
    CPU cpuCopy = cpu;
    static constexpr s32 NUM_CYCLES = 5;
    cpu.Y = 0xFF;

    mem[0xFFFC] = CPU::INS_LDA_ABY;
    mem[0xFFFD] = 0x20;
    mem[0xFFFE] = 0x04;
    mem[0x051F] = 0x42;

    // act
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert
    EXPECT_EQ(cpu.A, 0x42);
    EXPECT_EQ(cyclesExecuted, 5);
    LDAFlags_TestUnchanged(cpuCopy, cpu);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
}

TEST_F(CPUTest, LDAIndirectX_LoadValueIntoARegister)
{
    // arrange
    CPU cpuCopy = cpu;
    static constexpr s32 NUM_CYCLES = 6;
    cpu.X = 0x04;

    mem[0xFFFC] = CPU::INS_LDA_INX;
    mem[0xFFFD] = 0x73;
    mem[0x0077] = 0x20;
    mem[0x0078] = 0x04;
    mem[0x0420] = 0x42;

    // act
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert
    EXPECT_EQ(cpu.A, 0x42);
    EXPECT_EQ(cyclesExecuted, 6);
    LDAFlags_TestUnchanged(cpuCopy, cpu);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
}

TEST_F(CPUTest, LDAIndirectY_LoadValueIntoARegister)
{
    // arrange
    CPU cpuCopy = cpu;
    static constexpr s32 NUM_CYCLES = 5;
    cpu.Y = 0x05;
    
    mem[0xFFFC] = CPU::INS_LDA_INY;
    mem[0xFFFD] = 0x73;
    mem[0x0073] = 0x20;
    mem[0x0074] = 0x04;
    mem[0x0425] = 0x42;

    // act
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert
    EXPECT_EQ(cpu.A, 0x42);
    EXPECT_EQ(cyclesExecuted, 5);
    LDAFlags_TestUnchanged(cpuCopy, cpu);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
}

TEST_F(CPUTest, LDAIndirectY_LoadValueIntoARegisterPageCrossed)
{
    // arrange
    CPU cpuCopy = cpu;
    static constexpr s32 NUM_CYCLES = 6;
    cpu.Y = 0xFF;
    
    mem[0xFFFC] = CPU::INS_LDA_INY;
    mem[0xFFFD] = 0x73;
    mem[0x0073] = 0x20;
    mem[0x0074] = 0x04;
    mem[0x051F] = 0x42;

    // act
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert
    EXPECT_EQ(cpu.A, 0x42);
    EXPECT_EQ(cyclesExecuted, 6);
    LDAFlags_TestUnchanged(cpuCopy, cpu);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
}