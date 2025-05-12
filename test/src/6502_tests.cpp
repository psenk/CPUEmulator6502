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

TEST_F(CPUTest, LDAImmediate_LoadValueIntoARegister)
{
    mem[0xFFFC] = CPU::INS_LDA_IMM;
    mem[0xFFFD] = 0x42;

    cpu.execute(2, mem);

    EXPECT_EQ(cpu.A, 0x42);
}

TEST_F(CPUTest, LDAZeroPage_LoadValueIntoARegister)
{
    mem[0xFFFC] = CPU::INS_LDA_ZPG;
    mem[0xFFFD] = 0x42;
    mem[0x0042] = 0x73;

    cpu.execute(3, mem);

    EXPECT_EQ(cpu.A, 0x73);
}

TEST_F(CPUTest, LDAZeroPageX_LoadValueIntoARegister)
{
    cpu.X = 5;
    mem[0xFFFC] = CPU::INS_LDA_ZPX;
    mem[0xFFFD] = 0x42;
    mem[0x0047] = 0x73;

    cpu.execute(4, mem);

    EXPECT_EQ(cpu.A, 0x73);
}

TEST_F(CPUTest, LDAZeroPageX_LoadValueIntoARegister_WrapAround)
{
    cpu.X = 0xFF;
    mem[0xFFFC] = CPU::INS_LDA_ZPX;
    mem[0xFFFD] = 0x80;
    mem[0x007F] = 0x73;

    cpu.execute(4, mem);

    EXPECT_EQ(cpu.A, 0x73);
}