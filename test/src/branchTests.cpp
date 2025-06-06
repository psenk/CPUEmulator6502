#include <gtest/gtest.h>
#include "m6502.h"

class BranchTests : public ::testing::Test
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
    static void testAllFlagsUnchanged(const m6502::CPU &cpuCopy,
                                      const m6502::CPU &cpu)
    {
        EXPECT_EQ(cpuCopy.P.value, cpu.P.value);
    }
};

TEST_F(BranchTests, Branch_CarryFlagClear_BranchTaken)
{
    // arrange:
    using namespace m6502;
    cpu.reset(mem, 0xFF00);
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 3;
    cpu.P.bits.C = false;

    mem[0xFF00] = CPU::INS_BCC;
    mem[0xFF01] = 0x10;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.PC, 0xFF12);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

TEST_F(BranchTests, Branch_CarryFlagClear_BranchTaken_PageCrossed)
{
    // arrange:
    using namespace m6502;
    cpu.reset(mem, 0xFEFD);
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 4;
    cpu.P.bits.C = false;

    mem[0xFEFD] = CPU::INS_BCC;
    mem[0xFEFE] = 0x10;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.PC, 0xFF0F);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

TEST_F(BranchTests, Branch_CarryFlagClear_BranchTaken_NegativeValue)
{
    // arrange:
    using namespace m6502;
    cpu.reset(mem, 0xFF00);
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 4;
    cpu.P.bits.C = false;

    mem[0xFF00] = CPU::INS_BCC;
    mem[0xFF01] = 0x90;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.PC, 0xFE92);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

TEST_F(BranchTests, Branch_CarryFlagClear_BranchNotTaken)
{
    // arrange:
    using namespace m6502;
    cpu.reset(mem, 0xFF00);
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 2;
    cpu.P.bits.C = true;

    mem[0xFF00] = CPU::INS_BCC;
    mem[0xFF01] = 0x10;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.PC, 0xFF02);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

TEST_F(BranchTests, Branch_CarryFlagSet_BranchTaken)
{
    // arrange:
    using namespace m6502;
    cpu.reset(mem, 0xFF00);
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 3;
    cpu.P.bits.C = true;

    mem[0xFF00] = CPU::INS_BCS;
    mem[0xFF01] = 0x10;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.PC, 0xFF12);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

TEST_F(BranchTests, Branch_CarryFlagSet_BranchNotTaken)
{
    // arrange:
    using namespace m6502;
    cpu.reset(mem, 0xFF00);
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 2;
    cpu.P.bits.C = false;

    mem[0xFF00] = CPU::INS_BCS;
    mem[0xFF01] = 0x10;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.PC, 0xFF02);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

TEST_F(BranchTests, Branch_ZeroFlagClear_BranchTaken)
{
    // arrange:
    using namespace m6502;
    cpu.reset(mem, 0xFF00);
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 3;
    cpu.P.bits.Z = false;

    mem[0xFF00] = CPU::INS_BNE;
    mem[0xFF01] = 0x10;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.PC, 0xFF12);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

TEST_F(BranchTests, Branch_ZeroFlagClear_BranchNotTaken)
{
    // arrange:
    using namespace m6502;
    cpu.reset(mem, 0xFF00);
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 2;
    cpu.P.bits.Z = true;

    mem[0xFF00] = CPU::INS_BNE;
    mem[0xFF01] = 0x10;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.PC, 0xFF02);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

TEST_F(BranchTests, Branch_ZeroFlagSet_BranchTaken)
{
    // arrange:
    using namespace m6502;
    cpu.reset(mem, 0xFF00);
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 3;
    cpu.P.bits.Z = true;

    mem[0xFF00] = CPU::INS_BEQ;
    mem[0xFF01] = 0x10;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.PC, 0xFF12);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

TEST_F(BranchTests, Branch_ZeroFlagSet_BranchNotTaken)
{
    // arrange:
    using namespace m6502;
    cpu.reset(mem, 0xFF00);
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 2;
    cpu.P.bits.Z = false;

    mem[0xFF00] = CPU::INS_BEQ;
    mem[0xFF01] = 0x10;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.PC, 0xFF02);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

TEST_F(BranchTests, Branch_NegativeFlagClear_BranchTaken)
{
    // arrange:
    using namespace m6502;
    cpu.reset(mem, 0xFF00);
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 3;
    cpu.P.bits.N = false;

    mem[0xFF00] = CPU::INS_BPL;
    mem[0xFF01] = 0x10;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.PC, 0xFF12);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

TEST_F(BranchTests, Branch_NegativeFlagClear_BranchNotTaken)
{
    // arrange:
    using namespace m6502;
    cpu.reset(mem, 0xFF00);
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 2;
    cpu.P.bits.N = true;

    mem[0xFF00] = CPU::INS_BPL;
    mem[0xFF01] = 0x10;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.PC, 0xFF02);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

TEST_F(BranchTests, Branch_NegativeFlagSet_BranchTaken)
{
    // arrange:
    using namespace m6502;
    cpu.reset(mem, 0xFF00);
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 3;
    cpu.P.bits.N = true;

    mem[0xFF00] = CPU::INS_BMI;
    mem[0xFF01] = 0x10;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.PC, 0xFF12);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

TEST_F(BranchTests, Branch_NegativeFlagSet_BranchNotTaken)
{
    // arrange:
    using namespace m6502;
    cpu.reset(mem, 0xFF00);
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 2;
    cpu.P.bits.N = false;

    mem[0xFF00] = CPU::INS_BMI;
    mem[0xFF01] = 0x10;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.PC, 0xFF02);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

TEST_F(BranchTests, Branch_OverflowFlagClear_BranchTaken)
{
    // arrange:
    using namespace m6502;
    cpu.reset(mem, 0xFF00);
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 3;
    cpu.P.bits.V = false;

    mem[0xFF00] = CPU::INS_BVC;
    mem[0xFF01] = 0x10;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.PC, 0xFF12);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

TEST_F(BranchTests, Branch_OverflowFlagClear_BranchNotTaken)
{
    // arrange:
    using namespace m6502;
    cpu.reset(mem, 0xFF00);
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 2;
    cpu.P.bits.V = true;

    mem[0xFF00] = CPU::INS_BVC;
    mem[0xFF01] = 0x10;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.PC, 0xFF02);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

TEST_F(BranchTests, Branch_OverflowFlagSet_BranchTaken)
{
    // arrange:
    using namespace m6502;
    cpu.reset(mem, 0xFF00);
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 3;
    cpu.P.bits.V = true;

    mem[0xFF00] = CPU::INS_BVS;
    mem[0xFF01] = 0x10;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.PC, 0xFF12);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}

TEST_F(BranchTests, Branch_OverflowFlagSet_BranchNotTaken)
{
    // arrange:
    using namespace m6502;
    cpu.reset(mem, 0xFF00);
    CPU cpuCopy = cpu;
    static constexpr s_int32 NUM_CYCLES = 2;
    cpu.P.bits.V = false;

    mem[0xFF00] = CPU::INS_BVS;
    mem[0xFF01] = 0x10;

    // act:
    s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.PC, 0xFF02);
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
}
