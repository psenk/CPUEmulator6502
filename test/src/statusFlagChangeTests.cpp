#include <gtest/gtest.h>
#include "m6502.h"

class StatusFlagChangeTests : public ::testing::Test
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

TEST_F(StatusFlagChangeTests, ClearCarryFlag)
{
    using namespace m6502;
    static constexpr s32 NUM_CYCLES = 2;
    cpu.P.bits.C = true;

    mem[0xFFFC] = CPU::INS_CLC;

    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    EXPECT_FALSE(cpu.P.bits.C);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

TEST_F(StatusFlagChangeTests, ClearDecimalFlag)
{
    using namespace m6502;
    static constexpr s32 NUM_CYCLES = 2;
    cpu.P.bits.D = true;

    mem[0xFFFC] = CPU::INS_CLD;

    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    EXPECT_FALSE(cpu.P.bits.D);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

TEST_F(StatusFlagChangeTests, ClearInterruptFlag)
{
    using namespace m6502;
    static constexpr s32 NUM_CYCLES = 2;
    cpu.P.bits.I = true;

    mem[0xFFFC] = CPU::INS_CLI;

    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    EXPECT_FALSE(cpu.P.bits.I);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

TEST_F(StatusFlagChangeTests, ClearOverflowFlag)
{
    using namespace m6502;
    static constexpr s32 NUM_CYCLES = 2;
    cpu.P.bits.V = true;

    mem[0xFFFC] = CPU::INS_CLV;

    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    EXPECT_FALSE(cpu.P.bits.V);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

TEST_F(StatusFlagChangeTests, SetCarryFlag)
{
    using namespace m6502;
    static constexpr s32 NUM_CYCLES = 2;
    cpu.P.bits.C = false;

    mem[0xFFFC] = CPU::INS_SEC;

    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    EXPECT_TRUE(cpu.P.bits.C);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

TEST_F(StatusFlagChangeTests, SetDecimalFlag)
{
    using namespace m6502;
    static constexpr s32 NUM_CYCLES = 2;
    cpu.P.bits.D = false;

    mem[0xFFFC] = CPU::INS_SED;

    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    EXPECT_TRUE(cpu.P.bits.D);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

TEST_F(StatusFlagChangeTests, SetInterruptFlag)
{
    using namespace m6502;
    static constexpr s32 NUM_CYCLES = 2;
    cpu.P.bits.I = false;

    mem[0xFFFC] = CPU::INS_SEI;

    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    EXPECT_TRUE(cpu.P.bits.I);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}
