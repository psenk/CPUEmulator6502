#include <gtest/gtest.h>
#include "m6502.h"

// derived class
// :: - start at global scope
// testing - namespace that contains Test class
// ::Test - Test class within testing namespace
class CPUTest : public ::testing::Test
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

    // load immediate
    void testLoadImmediate(m6502::Byte opcode,
                           m6502::Byte m6502::CPU::*reg);
    void testLoadImmediateNegativeValue(m6502::Byte opcode,
                                        m6502::Byte m6502::CPU::*reg);
    void testLoadImmediateZeroValue(m6502::Byte opcode,
                                    m6502::Byte m6502::CPU::*reg);

    // load zero page
    void testLoadZeroPage(m6502::Byte opcode,
                          m6502::Byte m6502::CPU::*reg);

    // load zero page + register
    void testLoadZeroPagePlusRegister(m6502::Byte opcode,
                                      m6502::Byte m6502::CPU::*addedReg,
                                      m6502::Byte m6502::CPU::*reg);
    void testLoadZeroPagePlusRegisterWrapAround(m6502::Byte opcode,
                                                m6502::Byte m6502::CPU::*addedReg,
                                                m6502::Byte m6502::CPU::*reg);
    // load absolute
    void testLoadAbsolute(m6502::Byte opcode,
                          m6502::Byte m6502::CPU::*reg);

    // load absolute + register
    void testLoadAbsolutePlusRegister(m6502::Byte opcode,
                                      m6502::Byte m6502::CPU::*addedReg,
                                      m6502::Byte m6502::CPU::*reg);
    void testLoadAbsolutePlusRegisterPageCrossed(m6502::Byte opcode,
                                                 m6502::Byte m6502::CPU::*addedReg,
                                                 m6502::Byte m6502::CPU::*reg);
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

static void testLoadFlagsUnchanged(const m6502::CPU &cpuCopy,
                                   const m6502::CPU &cpu)
{
    EXPECT_EQ(cpuCopy.C, cpu.C);
    EXPECT_EQ(cpuCopy.I, cpu.I);
    EXPECT_EQ(cpuCopy.D, cpu.D);
    EXPECT_EQ(cpuCopy.B, cpu.B);
    EXPECT_EQ(cpuCopy.V, cpu.V);
}

/**
 * CPU TESTING
 */
TEST_F(CPUTest, CPUDoesNothingWithZeroCyclesExecuted)
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

TEST_F(CPUTest, CPUNotGivenEnoughCyclesForInstruction)
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
    testLoadFlagsUnchanged(cpuCopy, cpu);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
}

TEST_F(CPUTest, CPUExecutesBadFunction)
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
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
}

/**
 * LOAD IMMEDIATE TESTING
 */
void CPUTest::testLoadImmediate(
    m6502::Byte opcode,
    m6502::Byte m6502::CPU::*reg)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s32 NUM_CYCLES = 2;

    mem[0xFFFC] = opcode;
    mem[0xFFFD] = 0x42;

    // act:
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.*reg, 0x42);
    EXPECT_EQ(cyclesExecuted, 2);
    testLoadFlagsUnchanged(cpuCopy, cpu);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
}

void CPUTest::testLoadImmediateNegativeValue(
    m6502::Byte opcode,
    m6502::Byte m6502::CPU::*reg)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s32 NUM_CYCLES = 2;

    mem[0xFFFC] = opcode;
    mem[0xFFFD] = 0x84;

    // act:
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.*reg, 0x84);
    EXPECT_EQ(cyclesExecuted, 2);
    testLoadFlagsUnchanged(cpuCopy, cpu);
    EXPECT_FALSE(cpu.Z);
    EXPECT_TRUE(cpu.N);
}

void CPUTest::testLoadImmediateZeroValue(
    m6502::Byte opcode,
    m6502::Byte m6502::CPU::*reg)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s32 NUM_CYCLES = 2;

    mem[0xFFFC] = opcode;
    mem[0xFFFD] = 0x00;

    // act:
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.*reg, 0x00);
    EXPECT_EQ(cyclesExecuted, 2);
    testLoadFlagsUnchanged(cpuCopy, cpu);
    EXPECT_TRUE(cpu.Z);
    EXPECT_FALSE(cpu.N);
}

TEST_F(CPUTest, LDAImmediate_LoadValue)
{
    using namespace m6502;
    testLoadImmediate(CPU::INS_LDA_IMM, &CPU::A);
}

TEST_F(CPUTest, LDXImmediate_LoadValue)
{
    using namespace m6502;
    testLoadImmediate(CPU::INS_LDX_IMM, &CPU::X);
}

TEST_F(CPUTest, LDYImmediate_LoadValue)
{
    using namespace m6502;
    testLoadImmediate(CPU::INS_LDY_IMM, &CPU::Y);
}

TEST_F(CPUTest, LDAImmediate_LoadNegativeValue)
{
    using namespace m6502;
    testLoadImmediateNegativeValue(CPU::INS_LDA_IMM, &CPU::A);
}

TEST_F(CPUTest, LDXImmediate_LoadNegativeValue)
{
    using namespace m6502;
    testLoadImmediateNegativeValue(CPU::INS_LDX_IMM, &CPU::X);
}

TEST_F(CPUTest, LDYImmediate_LoadNegativeValue)
{
    using namespace m6502;
    testLoadImmediateNegativeValue(CPU::INS_LDY_IMM, &CPU::Y);
}

TEST_F(CPUTest, LDAImmediate_LoadZero)
{
    using namespace m6502;
    testLoadImmediateZeroValue(CPU::INS_LDA_IMM, &CPU::A);
}

TEST_F(CPUTest, LDXImmediate_LoadZero)
{
    using namespace m6502;
    testLoadImmediateZeroValue(CPU::INS_LDX_IMM, &CPU::X);
}

TEST_F(CPUTest, LDYImmediate_LoadZero)
{
    using namespace m6502;
    testLoadImmediateZeroValue(CPU::INS_LDY_IMM, &CPU::Y);
}

/**
 * LOAD ZERO PAGE TESTING
 */

void CPUTest::testLoadZeroPage(
    m6502::Byte opcode,
    m6502::Byte m6502::CPU::*reg)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s32 NUM_CYCLES = 3;

    mem[0xFFFC] = opcode;
    mem[0xFFFD] = 0x73;
    mem[0x0073] = 0x42;

    // act:
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.*reg, 0x42);
    EXPECT_EQ(cyclesExecuted, 3);
    testLoadFlagsUnchanged(cpuCopy, cpu);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
}

TEST_F(CPUTest, LDAZeroPage_LoadValue)
{
    using namespace m6502;
    testLoadZeroPage(CPU::INS_LDA_ZPG, &CPU::A);
}

TEST_F(CPUTest, LDXZeroPage_LoadValue)
{
    using namespace m6502;
    testLoadZeroPage(CPU::INS_LDX_ZPG, &CPU::X);
}

TEST_F(CPUTest, LDYZeroPage_LoadValue)
{
    using namespace m6502;
    testLoadZeroPage(CPU::INS_LDY_ZPG, &CPU::Y);
}

/**
 * LOAD ZERO PAGE + REGISTER TESTING
 */

void CPUTest::testLoadZeroPagePlusRegister(
    m6502::Byte opcode,
    m6502::Byte m6502::CPU::*addedReg,
    m6502::Byte m6502::CPU::*reg)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s32 NUM_CYCLES = 4;
    cpu.*addedReg = 0x05;

    mem[0xFFFC] = opcode;
    mem[0xFFFD] = 0x73;
    mem[0x0078] = 0x42;

    // act:
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.*reg, 0x42);
    EXPECT_EQ(cyclesExecuted, 4);
    testLoadFlagsUnchanged(cpuCopy, cpu);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
}

void CPUTest::testLoadZeroPagePlusRegisterWrapAround(
    m6502::Byte opcode,
    m6502::Byte m6502::CPU::*addedReg,
    m6502::Byte m6502::CPU::*reg)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s32 NUM_CYCLES = 4;
    cpu.*addedReg = 0xFF;

    mem[0xFFFC] = opcode;
    mem[0xFFFD] = 0x80;
    mem[0x007F] = 0x73;

    // act:
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(cpu.*reg, 0x73);
    EXPECT_EQ(cyclesExecuted, 4);
    testLoadFlagsUnchanged(cpuCopy, cpu);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
}

TEST_F(CPUTest, LDAZeroPageX_LoadValue)
{
    using namespace m6502;
    testLoadZeroPagePlusRegister(CPU::INS_LDA_ZPX, &CPU::X, &CPU::A);
}

TEST_F(CPUTest, LDXZeroPageY_LoadValue)
{
    using namespace m6502;
    testLoadZeroPagePlusRegister(CPU::INS_LDX_ZPY, &CPU::Y, &CPU::X);
}

TEST_F(CPUTest, LDYZeroPageX_LoadValue)
{
    using namespace m6502;
    testLoadZeroPagePlusRegister(CPU::INS_LDY_ZPX, &CPU::X, &CPU::Y);
}

TEST_F(CPUTest, LDAZeroPageX_LoadValue_WrapAround)
{
    using namespace m6502;
    testLoadZeroPagePlusRegisterWrapAround(CPU::INS_LDA_ZPX, &CPU::X, &CPU::A);
}

TEST_F(CPUTest, LDXZeroPageY_LoadValue_WrapAround)
{
    using namespace m6502;
    testLoadZeroPagePlusRegister(CPU::INS_LDX_ZPY, &CPU::Y, &CPU::X);
}

TEST_F(CPUTest, LDYZeroPageX_LoadValue_WrapAround)
{
    using namespace m6502;
    testLoadZeroPagePlusRegister(CPU::INS_LDY_ZPX, &CPU::X, &CPU::Y);
}

/**
 * ABSOLUTE TESTING
 */

void CPUTest::testLoadAbsolute(
    m6502::Byte opcode,
    m6502::Byte m6502::CPU::*reg)
{
    // arrange
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s32 NUM_CYCLES = 4;

    mem[0xFFFC] = opcode;
    mem[0xFFFD] = 0x20;
    mem[0xFFFE] = 0x04;
    mem[0x0420] = 0x42;

    // act
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert
    EXPECT_EQ(cpu.*reg, 0x42);
    EXPECT_EQ(cyclesExecuted, 4);
    testLoadFlagsUnchanged(cpuCopy, cpu);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
}

TEST_F(CPUTest, LDAAbsolute_LoadValue)
{
    using namespace m6502;
    testLoadAbsolute(CPU::INS_LDA_ABS, &CPU::A);
}

TEST_F(CPUTest, LDXAbsolute_LoadValue)
{
    using namespace m6502;
    testLoadAbsolute(CPU::INS_LDX_ABS, &CPU::X);
}

TEST_F(CPUTest, LDYAbsolute_LoadValue)
{
    using namespace m6502;
    testLoadAbsolute(CPU::INS_LDY_ABS, &CPU::Y);
}

/**
 * LOAD ABSOLUTE + REGISTER
 */

void CPUTest::testLoadAbsolutePlusRegister(
    m6502::Byte opcode,
    m6502::Byte m6502::CPU::*addedReg,
    m6502::Byte m6502::CPU::*reg)
{
    // arrange
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s32 NUM_CYCLES = 4;
    cpu.*addedReg = 0x05;

    mem[0xFFFC] = opcode;
    mem[0xFFFD] = 0x20;
    mem[0xFFFE] = 0x04;
    mem[0x0425] = 0x42;

    // act
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert
    EXPECT_EQ(cpu.*reg, 0x42);
    EXPECT_EQ(cyclesExecuted, 4);
    testLoadFlagsUnchanged(cpuCopy, cpu);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
}

void CPUTest::testLoadAbsolutePlusRegisterPageCrossed(
    m6502::Byte opcode,
    m6502::Byte m6502::CPU::*addedReg,
    m6502::Byte m6502::CPU::*reg)
{
    // arrange
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s32 NUM_CYCLES = 5;
    cpu.*addedReg = 0x00FF;

    mem[0xFFFC] = opcode;
    mem[0xFFFD] = 0x20;
    mem[0xFFFE] = 0x04;
    mem[0x051F] = 0x42;

    // act
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert
    EXPECT_EQ(cpu.*reg, 0x42);
    EXPECT_EQ(cyclesExecuted, 5);
    testLoadFlagsUnchanged(cpuCopy, cpu);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
}

TEST_F(CPUTest, LDAAbsoluteX_LoadValue)
{
    using namespace m6502;
    testLoadAbsolutePlusRegister(CPU::INS_LDA_ABX, &CPU::X, &CPU::A);
}

TEST_F(CPUTest, LDAAbsoluteY_LoadValue)
{
    using namespace m6502;
    testLoadAbsolutePlusRegister(CPU::INS_LDA_ABY, &CPU::Y, &CPU::A);
}

TEST_F(CPUTest, LDXAbsoluteY_LoadValue)
{
    using namespace m6502;
    testLoadAbsolutePlusRegister(CPU::INS_LDX_ABY, &CPU::Y, &CPU::X);
}

TEST_F(CPUTest, LDYAbsoluteX_LoadValue)
{
    using namespace m6502;
    testLoadAbsolutePlusRegister(CPU::INS_LDY_ABX, &CPU::X, &CPU::Y);
}

TEST_F(CPUTest, LDAAbsoluteX_LoadValue_PageCrossed)
{
    using namespace m6502;
    testLoadAbsolutePlusRegisterPageCrossed(CPU::INS_LDA_ABX, &CPU::X, &CPU::A);
}

TEST_F(CPUTest, LDAAbsoluteY_LoadValue_PageCrossed)
{
    using namespace m6502;
    testLoadAbsolutePlusRegisterPageCrossed(CPU::INS_LDA_ABY, &CPU::Y, &CPU::A);
}

TEST_F(CPUTest, LDXAbsoluteY_LoadValue_PageCrossed)
{
    using namespace m6502;
    testLoadAbsolutePlusRegisterPageCrossed(CPU::INS_LDX_ABY, &CPU::Y, &CPU::X);
}

TEST_F(CPUTest, LDYAbsoluteX_LoadValue_PageCrossed)
{
    using namespace m6502;
    testLoadAbsolutePlusRegisterPageCrossed(CPU::INS_LDY_ABX, &CPU::X, &CPU::Y);
}

/**
 * INDEXED INDIRECT TESTING
 */

TEST_F(CPUTest, LDAIndirectX_LoadValue)
{
    // arrange
    using namespace m6502;
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
    testLoadFlagsUnchanged(cpuCopy, cpu);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
}

/**
 * INDIRECT INDEXED TESTING
 */

TEST_F(CPUTest, LDAIndirectY_LoadValue)
{
    // arrange
    using namespace m6502;
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
    testLoadFlagsUnchanged(cpuCopy, cpu);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
}

TEST_F(CPUTest, LDAIndirectY_LoadValue_PageCrossed)
{
    // arrange
    using namespace m6502;
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
    testLoadFlagsUnchanged(cpuCopy, cpu);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
}