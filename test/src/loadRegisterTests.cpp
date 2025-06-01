#include <gtest/gtest.h>
#include "m6502.h"

// derived class
// :: - start at global scope
// testing - namespace that contains Test class
// ::Test - Test class within testing namespace
class LoadRegisterTests : public ::testing::Test
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
    void testLoadImmediate(const m6502::Byte opcode,
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
        EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
        testAllFlagsUnchanged(cpuCopy, cpu);
    }
    void testLoadImmediateNegativeValue(const m6502::Byte opcode,
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
        EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
        testLoadFlagsUnchanged(cpuCopy, cpu);
        EXPECT_FALSE(cpu.P.bits.Z);
        EXPECT_TRUE(cpu.P.bits.N);
    }
    void testLoadImmediateZeroValue(const m6502::Byte opcode,
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
        EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
        testLoadFlagsUnchanged(cpuCopy, cpu);
        EXPECT_TRUE(cpu.P.bits.Z);
        EXPECT_FALSE(cpu.P.bits.N);
    }

    // load zero page
    void testLoadZeroPage(const m6502::Byte opcode,
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
        EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
        testAllFlagsUnchanged(cpuCopy, cpu);
    }

    // load zero page + register
    void testLoadZeroPagePlusRegister(const m6502::Byte opcode,
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
        EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
        testAllFlagsUnchanged(cpuCopy, cpu);
    }
    void testLoadZeroPagePlusRegisterWrapAround(const m6502::Byte opcode,
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
        EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
        testAllFlagsUnchanged(cpuCopy, cpu);
    }

    // load absolute
    void testLoadAbsolute(const m6502::Byte opcode,
                          m6502::Byte m6502::CPU::*reg)
    {
        // arrange:
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
        EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
        testAllFlagsUnchanged(cpuCopy, cpu);
    }

    // load absolute + register
    void testLoadAbsolutePlusRegister(const m6502::Byte opcode,
                                      m6502::Byte m6502::CPU::*addedReg,
                                      m6502::Byte m6502::CPU::*reg)
    {
        // arrange:
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
        EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
        testAllFlagsUnchanged(cpuCopy, cpu);
    }
    void testLoadAbsolutePlusRegisterPageCrossed(const m6502::Byte opcode,
                                                 m6502::Byte m6502::CPU::*addedReg,
                                                 m6502::Byte m6502::CPU::*reg)
    {
        // arrange:
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
        EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
        testAllFlagsUnchanged(cpuCopy, cpu);
    }

    // test flags
    static void testAllFlagsUnchanged(const m6502::CPU &cpuCopy,
                                          const m6502::CPU &cpu)
    {
        EXPECT_EQ(cpuCopy.P.value, cpu.P.value);
    }

    static void testLoadFlagsUnchanged(const m6502::CPU &cpuCopy,
                                       const m6502::CPU &cpu)
    {
        EXPECT_EQ(cpuCopy.P.bits.C, cpu.P.bits.C);
        EXPECT_EQ(cpuCopy.P.bits.I, cpu.P.bits.I);
        EXPECT_EQ(cpuCopy.P.bits.D, cpu.P.bits.D);
        EXPECT_EQ(cpuCopy.P.bits.B, cpu.P.bits.B);
        EXPECT_EQ(cpuCopy.P.bits.V, cpu.P.bits.V);
    }
};

/**
 * LOAD IMMEDIATE TESTING
 */

TEST_F(LoadRegisterTests, LDAImmediate_LoadValue)
{
    using namespace m6502;
    testLoadImmediate(CPU::INS_LDA_IMM, &CPU::A);
}

TEST_F(LoadRegisterTests, LDXImmediate_LoadValue)
{
    using namespace m6502;
    testLoadImmediate(CPU::INS_LDX_IMM, &CPU::X);
}

TEST_F(LoadRegisterTests, LDYImmediate_LoadValue)
{
    using namespace m6502;
    testLoadImmediate(CPU::INS_LDY_IMM, &CPU::Y);
}

TEST_F(LoadRegisterTests, LDAImmediate_LoadNegativeValue)
{
    using namespace m6502;
    testLoadImmediateNegativeValue(CPU::INS_LDA_IMM, &CPU::A);
}

TEST_F(LoadRegisterTests, LDXImmediate_LoadNegativeValue)
{
    using namespace m6502;
    testLoadImmediateNegativeValue(CPU::INS_LDX_IMM, &CPU::X);
}

TEST_F(LoadRegisterTests, LDYImmediate_LoadNegativeValue)
{
    using namespace m6502;
    testLoadImmediateNegativeValue(CPU::INS_LDY_IMM, &CPU::Y);
}

TEST_F(LoadRegisterTests, LDAImmediate_LoadZero)
{
    using namespace m6502;
    testLoadImmediateZeroValue(CPU::INS_LDA_IMM, &CPU::A);
}

TEST_F(LoadRegisterTests, LDXImmediate_LoadZero)
{
    using namespace m6502;
    testLoadImmediateZeroValue(CPU::INS_LDX_IMM, &CPU::X);
}

TEST_F(LoadRegisterTests, LDYImmediate_LoadZero)
{
    using namespace m6502;
    testLoadImmediateZeroValue(CPU::INS_LDY_IMM, &CPU::Y);
}

/**
 * LOAD ZERO PAGE TESTING
 */

TEST_F(LoadRegisterTests, LDAZeroPage_LoadValue)
{
    using namespace m6502;
    testLoadZeroPage(CPU::INS_LDA_ZPG, &CPU::A);
}

TEST_F(LoadRegisterTests, LDXZeroPage_LoadValue)
{
    using namespace m6502;
    testLoadZeroPage(CPU::INS_LDX_ZPG, &CPU::X);
}

TEST_F(LoadRegisterTests, LDYZeroPage_LoadValue)
{
    using namespace m6502;
    testLoadZeroPage(CPU::INS_LDY_ZPG, &CPU::Y);
}

/**
 * LOAD ZERO PAGE + REGISTER TESTING
 */

TEST_F(LoadRegisterTests, LDAZeroPageX_LoadValue)
{
    using namespace m6502;
    testLoadZeroPagePlusRegister(CPU::INS_LDA_ZPX, &CPU::X, &CPU::A);
}

TEST_F(LoadRegisterTests, LDXZeroPageY_LoadValue)
{
    using namespace m6502;
    testLoadZeroPagePlusRegister(CPU::INS_LDX_ZPY, &CPU::Y, &CPU::X);
}

TEST_F(LoadRegisterTests, LDYZeroPageX_LoadValue)
{
    using namespace m6502;
    testLoadZeroPagePlusRegister(CPU::INS_LDY_ZPX, &CPU::X, &CPU::Y);
}

TEST_F(LoadRegisterTests, LDAZeroPageX_LoadValue_WrapAround)
{
    using namespace m6502;
    testLoadZeroPagePlusRegisterWrapAround(CPU::INS_LDA_ZPX, &CPU::X, &CPU::A);
}

TEST_F(LoadRegisterTests, LDXZeroPageY_LoadValue_WrapAround)
{
    using namespace m6502;
    testLoadZeroPagePlusRegister(CPU::INS_LDX_ZPY, &CPU::Y, &CPU::X);
}

TEST_F(LoadRegisterTests, LDYZeroPageX_LoadValue_WrapAround)
{
    using namespace m6502;
    testLoadZeroPagePlusRegister(CPU::INS_LDY_ZPX, &CPU::X, &CPU::Y);
}

/**
 * LOAD ABSOLUTE TESTING
 */

TEST_F(LoadRegisterTests, LDAAbsolute_LoadValue)
{
    using namespace m6502;
    testLoadAbsolute(CPU::INS_LDA_ABS, &CPU::A);
}

TEST_F(LoadRegisterTests, LDXAbsolute_LoadValue)
{
    using namespace m6502;
    testLoadAbsolute(CPU::INS_LDX_ABS, &CPU::X);
}

TEST_F(LoadRegisterTests, LDYAbsolute_LoadValue)
{
    using namespace m6502;
    testLoadAbsolute(CPU::INS_LDY_ABS, &CPU::Y);
}

/**
 * LOAD ABSOLUTE + REGISTER
 */

TEST_F(LoadRegisterTests, LDAAbsoluteX_LoadValue)
{
    using namespace m6502;
    testLoadAbsolutePlusRegister(CPU::INS_LDA_ABX, &CPU::X, &CPU::A);
}

TEST_F(LoadRegisterTests, LDAAbsoluteY_LoadValue)
{
    using namespace m6502;
    testLoadAbsolutePlusRegister(CPU::INS_LDA_ABY, &CPU::Y, &CPU::A);
}

TEST_F(LoadRegisterTests, LDXAbsoluteY_LoadValue)
{
    using namespace m6502;
    testLoadAbsolutePlusRegister(CPU::INS_LDX_ABY, &CPU::Y, &CPU::X);
}

TEST_F(LoadRegisterTests, LDYAbsoluteX_LoadValue)
{
    using namespace m6502;
    testLoadAbsolutePlusRegister(CPU::INS_LDY_ABX, &CPU::X, &CPU::Y);
}

TEST_F(LoadRegisterTests, LDAAbsoluteX_LoadValue_PageCrossed)
{
    using namespace m6502;
    testLoadAbsolutePlusRegisterPageCrossed(CPU::INS_LDA_ABX, &CPU::X, &CPU::A);
}

TEST_F(LoadRegisterTests, LDAAbsoluteY_LoadValue_PageCrossed)
{
    using namespace m6502;
    testLoadAbsolutePlusRegisterPageCrossed(CPU::INS_LDA_ABY, &CPU::Y, &CPU::A);
}

TEST_F(LoadRegisterTests, LDXAbsoluteY_LoadValue_PageCrossed)
{
    using namespace m6502;
    testLoadAbsolutePlusRegisterPageCrossed(CPU::INS_LDX_ABY, &CPU::Y, &CPU::X);
}

TEST_F(LoadRegisterTests, LDYAbsoluteX_LoadValue_PageCrossed)
{
    using namespace m6502;
    testLoadAbsolutePlusRegisterPageCrossed(CPU::INS_LDY_ABX, &CPU::X, &CPU::Y);
}

/**
 * LOAD INDEXED INDIRECT TESTING
 */

TEST_F(LoadRegisterTests, LDAIndirectX_LoadValue)
{
    // arrange:
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
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
    testLoadFlagsUnchanged(cpuCopy, cpu);
    EXPECT_FALSE(cpu.P.bits.Z);
    EXPECT_FALSE(cpu.P.bits.N);
}

/**
 * LOAD INDIRECT INDEXED TESTING
 */

TEST_F(LoadRegisterTests, LDAIndirectY_LoadValue)
{
    // arrange:
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
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
    testLoadFlagsUnchanged(cpuCopy, cpu);
    EXPECT_FALSE(cpu.P.bits.Z);
    EXPECT_FALSE(cpu.P.bits.N);
}

TEST_F(LoadRegisterTests, LDAIndirectY_LoadValue_PageCrossed)
{
    // arrange:
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
    EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
    testLoadFlagsUnchanged(cpuCopy, cpu);
    EXPECT_FALSE(cpu.P.bits.Z);
    EXPECT_FALSE(cpu.P.bits.N);
}