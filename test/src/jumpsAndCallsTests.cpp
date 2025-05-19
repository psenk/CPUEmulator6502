#include <gtest/gtest.h>
#include "m6502.h"

class JumpsAndCallsTests : public ::testing::Test
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
    static void testJumpsAndCallsFlagsUnchanged(const m6502::CPU &cpuCopy,
                                             const m6502::CPU &cpu)
    {
        EXPECT_EQ(cpuCopy.P.value, cpu.P.value);
    }
};

TEST_F(JumpsAndCallsTests, JSR_JumpToAddress)
{
    // arrange
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s32 NUM_CYCLES = 6;
    cpu.PC = 0xFF00;

    mem[0xFF00] = CPU::INS_JSR_ABS;
    mem[0xFF01] = 0x20;
    mem[0xFF02] = 0x04;

    // act
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert
    EXPECT_EQ(cpu.PC, 0x0420);
    EXPECT_EQ(cpu.SP, cpuCopy.SP - 2);
    EXPECT_EQ(mem[0x0100 + cpu.SP + 1], 0x02);
    EXPECT_EQ(mem[0x0100 + cpu.SP + 2], 0xFF);
    EXPECT_EQ(cyclesExecuted, 6);
    testJumpsAndCallsFlagsUnchanged(cpuCopy, cpu);
}

TEST_F(JumpsAndCallsTests, RTS_ReturnFromAddress)
{
    // arrange
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s32 NUM_CYCLES = 6;
    cpu.PC = 0xFF00;
    cpu.SP -= 2;
    mem[0x01FF] = 0x04;
    mem[0x01FE] = 0x20;

    mem[0xFF00] = CPU::INS_RTS;

    // act
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert
    EXPECT_EQ(cpu.PC, 0x0421);
    EXPECT_EQ(cpu.SP, cpuCopy.SP);
    EXPECT_EQ(cyclesExecuted, 6);
    testJumpsAndCallsFlagsUnchanged(cpuCopy, cpu);
}