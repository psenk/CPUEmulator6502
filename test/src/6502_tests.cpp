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

void TestLDAFlagsUnchanged(CPU cpuCopy, CPU cpu)
{
    EXPECT_EQ(cpuCopy.C, cpu.C);
    EXPECT_EQ(cpuCopy.I, cpu.I);
    EXPECT_EQ(cpuCopy.D, cpu.D);
    EXPECT_EQ(cpuCopy.B, cpu.B);
    EXPECT_EQ(cpuCopy.V, cpu.V);
}

TEST_F(CPUTest, LDAImmediate_LoadValueIntoARegister)
{
    CPU cpuCopy = cpu;
    mem[0xFFFC] = CPU::INS_LDA_IMM;
    mem[0xFFFD] = 0x42;

    cpu.execute(2, mem);

    EXPECT_EQ(cpu.A, 0x42);
    TestLDAFlagsUnchanged(cpuCopy, cpu);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
}

TEST_F(CPUTest, LDAImmediate_LoadNegativeValueIntoARegister)
{
    CPU cpuCopy = cpu;
    mem[0xFFFC] = CPU::INS_LDA_IMM;
    mem[0xFFFD] = 0x84;

    cpu.execute(2, mem);

    EXPECT_EQ(cpu.A, 0x84);
    TestLDAFlagsUnchanged(cpuCopy, cpu);
    EXPECT_FALSE(cpu.Z);
    EXPECT_TRUE(cpu.N);
}

TEST_F(CPUTest, LDAZeroPage_LoadValueIntoARegister)
{
    CPU cpuCopy = cpu;
    mem[0xFFFC] = CPU::INS_LDA_ZPG;
    mem[0xFFFD] = 0x42;
    mem[0x0042] = 0x73;

    cpu.execute(3, mem);

    EXPECT_EQ(cpu.A, 0x73);
    TestLDAFlagsUnchanged(cpuCopy, cpu);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
}

TEST_F(CPUTest, LDAZeroPageX_LoadValueIntoARegister)
{
    CPU cpuCopy = cpu;
    cpu.X = 5;
    mem[0xFFFC] = CPU::INS_LDA_ZPX;
    mem[0xFFFD] = 0x42;
    mem[0x0047] = 0x73;

    cpu.execute(4, mem);

    EXPECT_EQ(cpu.A, 0x73);
    TestLDAFlagsUnchanged(cpuCopy, cpu);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
}

TEST_F(CPUTest, LDAZeroPageX_LoadValueIntoARegister_WrapAround)
{
    CPU cpuCopy = cpu;
    cpu.X = 0xFF;
    mem[0xFFFC] = CPU::INS_LDA_ZPX;
    mem[0xFFFD] = 0x80;
    mem[0x007F] = 0x73;

    cpu.execute(4, mem);

    EXPECT_EQ(cpu.A, 0x73);
    TestLDAFlagsUnchanged(cpuCopy, cpu);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
}