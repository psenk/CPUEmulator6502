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
    // arrange:
    using namespace m6502;
    static constexpr s32 NUM_CYCLES = 2;
    cpu.P.bits.C = true;

    mem[0xFFFC] = CPU::INS_CLC;

    // act:
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_FALSE(cpu.P.bits.C);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

TEST_F(StatusFlagChangeTests, ClearDecimalFlag)
{
    // arrange:
    using namespace m6502;
    static constexpr s32 NUM_CYCLES = 2;
    cpu.P.bits.D = true;

    mem[0xFFFC] = CPU::INS_CLD;

    // act:
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_FALSE(cpu.P.bits.D);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

TEST_F(StatusFlagChangeTests, ClearInterruptFlag)
{
    // arrange:
    using namespace m6502;
    static constexpr s32 NUM_CYCLES = 2;
    cpu.P.bits.I = true;

    mem[0xFFFC] = CPU::INS_CLI;

    // act:
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_FALSE(cpu.P.bits.I);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

TEST_F(StatusFlagChangeTests, ClearOverflowFlag)
{
    // arrange:
    using namespace m6502;
    static constexpr s32 NUM_CYCLES = 2;
    cpu.P.bits.V = true;

    mem[0xFFFC] = CPU::INS_CLV;

    // act:
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_FALSE(cpu.P.bits.V);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

TEST_F(StatusFlagChangeTests, SetCarryFlag)
{
    // arrange:
    using namespace m6502;
    static constexpr s32 NUM_CYCLES = 2;
    cpu.P.bits.C = false;

    mem[0xFFFC] = CPU::INS_SEC;

    // act:
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_TRUE(cpu.P.bits.C);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

TEST_F(StatusFlagChangeTests, SetDecimalFlag)
{
    // arrange:
    using namespace m6502;
    static constexpr s32 NUM_CYCLES = 2;
    cpu.P.bits.D = false;

    mem[0xFFFC] = CPU::INS_SED;

    // act:
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_TRUE(cpu.P.bits.D);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

TEST_F(StatusFlagChangeTests, SetInterruptFlag)
{
    // arrange:
    using namespace m6502;
    static constexpr s32 NUM_CYCLES = 2;
    cpu.P.bits.I = false;

    mem[0xFFFC] = CPU::INS_SEI;

    // act:
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_TRUE(cpu.P.bits.I);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}
