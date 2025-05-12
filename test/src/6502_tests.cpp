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

TEST_F(CPUTest, RunInlineProgram)
{
    mem[0xFFFC] = CPU::INS_LDA_IMM;
    mem[0xFFFD] = 0x42;

    cpu.execute(2, mem);

    EXPECT_EQ(cpu.A, 0x42);
}