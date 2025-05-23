#include <gtest/gtest.h>
#include "m6502.h"

class LogicalTests : public ::testing::Test
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

    // logical immediate
    void testLogicalImmediate(m6502::Byte opcode)
    {
        // arrange:
        using namespace m6502;
        CPU cpuCopy = cpu;
        static constexpr s32 NUM_CYCLES = 2;
        cpu.A = 0x50;

        mem[0xFFFC] = opcode;
        mem[0xFFFD] = 0x42;

        // act:
        s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

        // assert:
        switch (opcode)
        {
        case CPU::INS_AND_IMM:
            EXPECT_EQ(cpu.A, 0x40);
            break;
        case CPU::INS_EOR_IMM:
            EXPECT_EQ(cpu.A, 0x12);
            break;
        case CPU::INS_ORA_IMM:
            EXPECT_EQ(cpu.A, 0x52);
            break;
        }
        EXPECT_EQ(cyclesExecuted, 2);
        testAllLogicalFlagsUnchanged(cpuCopy, cpu);
    }
    void testLogicalImmediateNegativeValue(m6502::Byte opcode)
    {
        // arrange:
        using namespace m6502;
        CPU cpuCopy = cpu;
        static constexpr s32 NUM_CYCLES = 2;
        cpu.A = 0x85;

        mem[0xFFFC] = opcode;

        switch (opcode)
        {
        case CPU::INS_AND_IMM:
            mem[0xFFFD] = 0x80;
            break;
        case CPU::INS_EOR_IMM:
            mem[0xFFFD] = 0x05;
            break;
        case CPU::INS_ORA_IMM:
            mem[0xFFFD] = 0x00;
            break;
        }

        // act:
        s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

        // assert:
        switch (opcode)
        {
        case CPU::INS_AND_IMM:
        case CPU::INS_EOR_IMM:
            EXPECT_EQ(cpu.A, 0x80);
            break;
        case CPU::INS_ORA_IMM:
            EXPECT_EQ(cpu.A, 0x85);
            break;
        }
        EXPECT_EQ(cyclesExecuted, 2);
        testLogicalFlagsUnchanged(cpuCopy, cpu);
        EXPECT_FALSE(cpu.P.bits.Z);
        EXPECT_TRUE(cpu.P.bits.N);
    }
    void testLogicalImmediateZeroValue(m6502::Byte opcode)
    {
        // arrange:
        using namespace m6502;
        CPU cpuCopy = cpu;
        static constexpr s32 NUM_CYCLES = 2;
        cpu.A = 0x0F;

        mem[0xFFFC] = opcode;

        switch (opcode)
        {
        case CPU::INS_AND_IMM:
            mem[0xFFFD] = 0xF0;
            break;
        case CPU::INS_EOR_IMM:
            mem[0xFFFD] = 0x0F;
            break;
        case CPU::INS_ORA_IMM:
            cpu.A = 0x00;
            mem[0xFFFD] = 0x00;
            break;
        }

        // act:
        s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

        // assert:
        EXPECT_EQ(cpu.A, 0x00);
        EXPECT_EQ(cyclesExecuted, 2);
        testLogicalFlagsUnchanged(cpuCopy, cpu);
        EXPECT_TRUE(cpu.P.bits.Z);
        EXPECT_FALSE(cpu.P.bits.N);
    }

    // logical zero page
    void testLogicalZeroPage(m6502::Byte opcode)
    {
        // arrange:
        using namespace m6502;
        CPU cpuCopy = cpu;
        static constexpr s32 NUM_CYCLES = 3;
        cpu.A = 0x50;

        mem[0xFFFC] = opcode;
        mem[0xFFFD] = 0x73;
        mem[0x0073] = 0x42;

        // act:
        s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

        // assert:
        switch (opcode)
        {
        case CPU::INS_AND_ZPG:
            EXPECT_EQ(cpu.A, 0x40);
            break;
        case CPU::INS_EOR_ZPG:
            EXPECT_EQ(cpu.A, 0x12);
            break;
        case CPU::INS_ORA_ZPG:
            EXPECT_EQ(cpu.A, 0x52);
            break;
        }
        EXPECT_EQ(cyclesExecuted, 3);
        testAllLogicalFlagsUnchanged(cpuCopy, cpu);
    }

    // logical zero page + register
    void testLogicalZeroPagePlusRegister(m6502::Byte opcode,
                                         m6502::Byte m6502::CPU::*addedReg)
    {
        // arrange:
        using namespace m6502;
        CPU cpuCopy = cpu;
        static constexpr s32 NUM_CYCLES = 4;
        cpu.A = 0x50;
        cpu.*addedReg = 0x05;

        mem[0xFFFC] = opcode;
        mem[0xFFFD] = 0x73;
        mem[0x0078] = 0x42;

        // act:
        s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

        // assert:
        switch (opcode)
        {
        case CPU::INS_AND_ZPX:
            EXPECT_EQ(cpu.A, 0x40);
            break;
        case CPU::INS_EOR_ZPX:
            EXPECT_EQ(cpu.A, 0x12);
            break;
        case CPU::INS_ORA_ZPX:
            EXPECT_EQ(cpu.A, 0x52);
            break;
        }
        EXPECT_EQ(cyclesExecuted, 4);
        testAllLogicalFlagsUnchanged(cpuCopy, cpu);
    }
    void testLogicalZeroPagePlusRegisterWrapAround(m6502::Byte opcode,
                                                   m6502::Byte m6502::CPU::*addedReg)
    {
        // arrange:
        using namespace m6502;
        CPU cpuCopy = cpu;
        static constexpr s32 NUM_CYCLES = 4;
        cpu.A = 0x50;
        cpu.*addedReg = 0xFF;

        mem[0xFFFC] = opcode;
        mem[0xFFFD] = 0x80;
        mem[0x007F] = 0x42;

        // act:
        s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

        // assert:
        switch (opcode)
        {
        case CPU::INS_AND_ZPX:
            EXPECT_EQ(cpu.A, 0x40);
            break;
        case CPU::INS_EOR_ZPX:
            EXPECT_EQ(cpu.A, 0x12);
            break;
        case CPU::INS_ORA_ZPX:
            EXPECT_EQ(cpu.A, 0x52);
            break;
        }
        EXPECT_EQ(cyclesExecuted, 4);
        testAllLogicalFlagsUnchanged(cpuCopy, cpu);
    }

    // logical absolute
    void testLogicalAbsolute(m6502::Byte opcode)
    {
        // arrange
        using namespace m6502;
        CPU cpuCopy = cpu;
        static constexpr s32 NUM_CYCLES = 4;
        cpu.A = 0x50;

        mem[0xFFFC] = opcode;
        mem[0xFFFD] = 0x20;
        mem[0xFFFE] = 0x04;
        mem[0x0420] = 0x42;

        // act
        s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

        // assert
        switch (opcode)
        {
        case CPU::INS_AND_ZPX:
            EXPECT_EQ(cpu.A, 0x40);
            break;
        case CPU::INS_EOR_ZPX:
            EXPECT_EQ(cpu.A, 0x12);
            break;
        case CPU::INS_ORA_ZPX:
            EXPECT_EQ(cpu.A, 0x52);
            break;
        }
        EXPECT_EQ(cyclesExecuted, 4);
        testAllLogicalFlagsUnchanged(cpuCopy, cpu);
    }

    // logical absolute + register
    void testLogicalAbsolutePlusRegister(m6502::Byte opcode,
                                         m6502::Byte m6502::CPU::*addedReg)
    {
        // arrange
        using namespace m6502;
        CPU cpuCopy = cpu;
        static constexpr s32 NUM_CYCLES = 4;
        cpu.A = 0x50;
        cpu.*addedReg = 0x05;

        mem[0xFFFC] = opcode;
        mem[0xFFFD] = 0x20;
        mem[0xFFFE] = 0x04;
        mem[0x0425] = 0x42;

        // act
        s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

        // assert
        switch (opcode)
        {
        case CPU::INS_AND_ABX:
        case CPU::INS_AND_ABY:
            EXPECT_EQ(cpu.A, 0x40);
            break;
        case CPU::INS_EOR_ABX:
        case CPU::INS_EOR_ABY:
            EXPECT_EQ(cpu.A, 0x12);
            break;
        case CPU::INS_ORA_ABX:
        case CPU::INS_ORA_ABY:
            EXPECT_EQ(cpu.A, 0x52);
            break;
        }
        EXPECT_EQ(cyclesExecuted, 4);
        testAllLogicalFlagsUnchanged(cpuCopy, cpu);
    }
    void testLogicalAbsolutePlusRegisterPageCrossed(m6502::Byte opcode,
                                                    m6502::Byte m6502::CPU::*addedReg)
    {
        // arrange
        using namespace m6502;
        CPU cpuCopy = cpu;
        static constexpr s32 NUM_CYCLES = 5;
        cpu.A = 0x50;
        cpu.*addedReg = 0x00FF;

        mem[0xFFFC] = opcode;
        mem[0xFFFD] = 0x20;
        mem[0xFFFE] = 0x04;
        mem[0x051F] = 0x42;

        // act
        s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

        // assert
        switch (opcode)
        {
        case CPU::INS_AND_ABX:
        case CPU::INS_AND_ABY:
            EXPECT_EQ(cpu.A, 0x40);
            break;
        case CPU::INS_EOR_ABX:
        case CPU::INS_EOR_ABY:
            EXPECT_EQ(cpu.A, 0x12);
            break;
        case CPU::INS_ORA_ABX:
        case CPU::INS_ORA_ABY:
            EXPECT_EQ(cpu.A, 0x52);
            break;
        }
        EXPECT_EQ(cyclesExecuted, 5);
        testAllLogicalFlagsUnchanged(cpuCopy, cpu);
    }

    // test logical indirect indexed
    void testLogicalIndexedIndirect(m6502::Byte opcode)
    {
        // arrange
        using namespace m6502;
        CPU cpuCopy = cpu;
        static constexpr s32 NUM_CYCLES = 6;
        cpu.A = 0x50;
        cpu.X = 0x04;

        mem[0xFFFC] = opcode;
        mem[0xFFFD] = 0x73;
        mem[0x0077] = 0x20;
        mem[0x0078] = 0x04;
        mem[0x0420] = 0x42;

        // act
        s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

        // assert
        switch (opcode)
        {
        case CPU::INS_AND_IMM:
            EXPECT_EQ(cpu.A, 0x40);
            break;
        case CPU::INS_EOR_IMM:
            EXPECT_EQ(cpu.A, 0x12);
            break;
        case CPU::INS_ORA_IMM:
            EXPECT_EQ(cpu.A, 0x52);
            break;
        }
        EXPECT_EQ(cyclesExecuted, 6);
        testAllLogicalFlagsUnchanged(cpuCopy, cpu);
    }

    // test logical indexed indirect
    void testLogicalIndirectIndexed(m6502::Byte opcode)
    {
        // arrange
        using namespace m6502;
        CPU cpuCopy = cpu;
        static constexpr s32 NUM_CYCLES = 5;
        cpu.A = 0x50;
        cpu.Y = 0x05;

        mem[0xFFFC] = opcode;
        mem[0xFFFD] = 0x73;
        mem[0x0073] = 0x20;
        mem[0x0074] = 0x04;
        mem[0x0425] = 0x42;

        // act
        s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

        // assert
        switch (opcode)
        {
        case CPU::INS_AND_IMM:
            EXPECT_EQ(cpu.A, 0x40);
            break;
        case CPU::INS_EOR_IMM:
            EXPECT_EQ(cpu.A, 0x12);
            break;
        case CPU::INS_ORA_IMM:
            EXPECT_EQ(cpu.A, 0x52);
            break;
        }
        EXPECT_EQ(cyclesExecuted, 5);
        testAllLogicalFlagsUnchanged(cpuCopy, cpu);
    }

    void testLogicalIndirectIndexedPageCrossed(m6502::Byte opcode)
    { // arrange
        using namespace m6502;
        CPU cpuCopy = cpu;
        static constexpr s32 NUM_CYCLES = 6;
        cpu.A = 0x50;
        cpu.Y = 0xFF;

        mem[0xFFFC] = CPU::INS_ORA_INY;
        mem[0xFFFD] = 0x73;
        mem[0x0073] = 0x20;
        mem[0x0074] = 0x04;
        mem[0x051F] = 0x42;

        // act
        s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

        // assert
        switch (opcode)
        {
        case CPU::INS_AND_IMM:
            EXPECT_EQ(cpu.A, 0x40);
            break;
        case CPU::INS_EOR_IMM:
            EXPECT_EQ(cpu.A, 0x12);
            break;
        case CPU::INS_ORA_IMM:
            EXPECT_EQ(cpu.A, 0x52);
            break;
        }
        EXPECT_EQ(cyclesExecuted, 6);
        testAllLogicalFlagsUnchanged(cpuCopy, cpu);
    }

    // test bit tests
    void testLogicalBITTestZeroPage(m6502::Byte testInput, bool allTest)
    {
        // arrange:
        using namespace m6502;
        static constexpr s32 NUM_CYCLES = 3;
        if (allTest)
        {
            cpu.A = 0xC0;
        }
        else
        {
            cpu.A = 0xC2;
        }

        mem[0xFFFC] = CPU::INS_BIT_ZPG;
        mem[0xFFFD] = 0x73;
        mem[0x0073] = testInput;

        // act:
        s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

        // assert:
        EXPECT_EQ(cyclesExecuted, 3);
    }

    void testLogicalBITTestAbsolute(m6502::Byte testInput, bool allTest)
    {
        // arrange:
        using namespace m6502;
        static constexpr s32 NUM_CYCLES = 4;
        if (allTest)
        {
            cpu.A = 0xC0;
        }
        else
        {
            cpu.A = 0xC2;
        }

        mem[0xFFFC] = CPU::INS_BIT_ABS;
        mem[0xFFFD] = 0x20;
        mem[0xFFFE] = 0x04;
        mem[0x0420] = testInput;

        // act:
        s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

        // assert:
        EXPECT_EQ(cyclesExecuted, 4);
    }

    // test flags
    static void testAllLogicalFlagsUnchanged(const m6502::CPU &cpuCopy,
                                             const m6502::CPU &cpu)
    {
        EXPECT_EQ(cpuCopy.P.value, cpu.P.value);
    }

    static void testLogicalFlagsUnchanged(const m6502::CPU &cpuCopy,
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
 * LOGICAL IMMEDIATE TESTING
 */

TEST_F(LogicalTests, ANDImmediate_LoadValue)
{
    using namespace m6502;
    testLogicalImmediate(CPU::INS_AND_IMM);
}

TEST_F(LogicalTests, EORImmediate_LoadValue)
{
    using namespace m6502;
    testLogicalImmediate(CPU::INS_EOR_IMM);
}

TEST_F(LogicalTests, ORAImmediate_LoadValue)
{
    using namespace m6502;
    testLogicalImmediate(CPU::INS_ORA_IMM);
}

TEST_F(LogicalTests, ANDImmediate_LoadNegativeValue)
{
    using namespace m6502;
    testLogicalImmediateNegativeValue(CPU::INS_AND_IMM);
}

TEST_F(LogicalTests, EORImmediate_LoadNegativeValue)
{
    using namespace m6502;
    testLogicalImmediateNegativeValue(CPU::INS_EOR_IMM);
}

TEST_F(LogicalTests, ORAImmediate_LoadNegativeValue)
{
    using namespace m6502;
    testLogicalImmediateNegativeValue(CPU::INS_ORA_IMM);
}

TEST_F(LogicalTests, ANDImmediate_LoadZero)
{
    using namespace m6502;
    testLogicalImmediateZeroValue(CPU::INS_AND_IMM);
}

TEST_F(LogicalTests, EORImmediate_LoadZero)
{
    using namespace m6502;
    testLogicalImmediateZeroValue(CPU::INS_EOR_IMM);
}

TEST_F(LogicalTests, ORAImmediate_LoadZero)
{
    using namespace m6502;
    testLogicalImmediateZeroValue(CPU::INS_ORA_IMM);
}

/**
 * LOGICAL ZERO PAGE TESTING
 */

TEST_F(LogicalTests, ANDZeroPage_LoadValue)
{
    using namespace m6502;
    testLogicalZeroPage(CPU::INS_AND_ZPG);
}

TEST_F(LogicalTests, EORZeroPage_LoadValue)
{
    using namespace m6502;
    testLogicalZeroPage(CPU::INS_EOR_ZPG);
}

TEST_F(LogicalTests, ORAZeroPage_LoadValue)
{
    using namespace m6502;
    testLogicalZeroPage(CPU::INS_ORA_ZPG);
}

/**
 * LOGICAL ZERO PAGE + REGISTER TESTING
 */

TEST_F(LogicalTests, ANDZeroPageX_LoadValue)
{
    using namespace m6502;
    testLogicalZeroPagePlusRegister(CPU::INS_AND_ZPX, &CPU::X);
}

TEST_F(LogicalTests, EORZeroPageX_LoadValue)
{
    using namespace m6502;
    testLogicalZeroPagePlusRegister(CPU::INS_EOR_ZPX, &CPU::X);
}

TEST_F(LogicalTests, ORAZeroPageX_LoadValue)
{
    using namespace m6502;
    testLogicalZeroPagePlusRegister(CPU::INS_ORA_ZPX, &CPU::X);
}

TEST_F(LogicalTests, ANDZeroPageX_LoadValue_WrapAround)
{
    using namespace m6502;
    testLogicalZeroPagePlusRegisterWrapAround(CPU::INS_AND_ZPX, &CPU::X);
}

TEST_F(LogicalTests, EORZeroPageX_LoadValue_WrapAround)
{
    using namespace m6502;
    testLogicalZeroPagePlusRegisterWrapAround(CPU::INS_EOR_ZPX, &CPU::X);
}

TEST_F(LogicalTests, ORAZeroPageX_LoadValue_WrapAround)
{
    using namespace m6502;
    testLogicalZeroPagePlusRegisterWrapAround(CPU::INS_ORA_ZPX, &CPU::X);
}

/**
 * LOGICAL ABSOLUTE TESTING
 */

TEST_F(LogicalTests, ANDAbsolute_LoadValue)
{
    using namespace m6502;
    testLogicalAbsolute(CPU::INS_AND_ABS);
}

TEST_F(LogicalTests, EORAbsolute_LoadValue)
{
    using namespace m6502;
    testLogicalAbsolute(CPU::INS_EOR_ABS);
}

TEST_F(LogicalTests, ORAAbsolute_LoadValue)
{
    using namespace m6502;
    testLogicalAbsolute(CPU::INS_ORA_ABS);
}

/**
 * LOGICAL ABSOLUTE + REGISTER
 */

TEST_F(LogicalTests, ANDAbsoluteX_LoadValue)
{
    using namespace m6502;
    testLogicalAbsolutePlusRegister(CPU::INS_AND_ABX, &CPU::X);
}

TEST_F(LogicalTests, ANDAbsoluteY_LoadValue)
{
    using namespace m6502;
    testLogicalAbsolutePlusRegister(CPU::INS_AND_ABY, &CPU::Y);
}

TEST_F(LogicalTests, EORAbsoluteX_LoadValue)
{
    using namespace m6502;
    testLogicalAbsolutePlusRegister(CPU::INS_EOR_ABX, &CPU::X);
}

TEST_F(LogicalTests, EORAbsoluteY_LoadValue)
{
    using namespace m6502;
    testLogicalAbsolutePlusRegister(CPU::INS_EOR_ABY, &CPU::Y);
}

TEST_F(LogicalTests, ORAAbsoluteX_LoadValue)
{
    using namespace m6502;
    testLogicalAbsolutePlusRegister(CPU::INS_ORA_ABX, &CPU::X);
}

TEST_F(LogicalTests, ORAAbsoluteY_LoadValue)
{
    using namespace m6502;
    testLogicalAbsolutePlusRegister(CPU::INS_ORA_ABY, &CPU::Y);
}

TEST_F(LogicalTests, ANDAbsoluteX_LoadValue_PageCrossed)
{
    using namespace m6502;
    testLogicalAbsolutePlusRegisterPageCrossed(CPU::INS_AND_ABX, &CPU::X);
}

TEST_F(LogicalTests, ANDAbsoluteY_LoadValue_PageCrossed)
{
    using namespace m6502;
    testLogicalAbsolutePlusRegisterPageCrossed(CPU::INS_AND_ABY, &CPU::Y);
}

TEST_F(LogicalTests, EORAbsoluteX_LoadValue_PageCrossed)
{
    using namespace m6502;
    testLogicalAbsolutePlusRegisterPageCrossed(CPU::INS_EOR_ABX, &CPU::X);
}

TEST_F(LogicalTests, EORAbsoluteY_LoadValue_PageCrossed)
{
    using namespace m6502;
    testLogicalAbsolutePlusRegisterPageCrossed(CPU::INS_EOR_ABY, &CPU::Y);
}

TEST_F(LogicalTests, ORAAbsoluteX_LoadValue_PageCrossed)
{
    using namespace m6502;
    testLogicalAbsolutePlusRegisterPageCrossed(CPU::INS_ORA_ABX, &CPU::X);
}

TEST_F(LogicalTests, ORAAbsoluteY_LoadValue_PageCrossed)
{
    using namespace m6502;
    testLogicalAbsolutePlusRegisterPageCrossed(CPU::INS_ORA_ABY, &CPU::Y);
}

/**
 * LOGICAL INDEXED INDIRECT TESTING
 */

TEST_F(LogicalTests, ANDIndirectX_LoadValue)
{
    using namespace m6502;
    testLogicalIndexedIndirect(CPU::INS_AND_INX);
}

TEST_F(LogicalTests, EORIndirectX_LoadValue)
{
    using namespace m6502;
    testLogicalIndexedIndirect(CPU::INS_EOR_INX);
}

TEST_F(LogicalTests, ORAIndirectX_LoadValue)
{
    using namespace m6502;
    testLogicalIndexedIndirect(CPU::INS_ORA_INX);
}

/**
 * LOGICAL INDIRECT INDEXED TESTING
 */

TEST_F(LogicalTests, ANDIndirectY_LoadValue)
{
    using namespace m6502;
    testLogicalIndirectIndexed(CPU::INS_AND_INY);
}

TEST_F(LogicalTests, ANDIndirectY_LoadValue_PageCrossed)
{
    using namespace m6502;
    testLogicalIndirectIndexedPageCrossed(CPU::INS_AND_INY);
}

TEST_F(LogicalTests, EORIndirectY_LoadValue)
{
    using namespace m6502;
    testLogicalIndirectIndexed(CPU::INS_EOR_INY);
}

TEST_F(LogicalTests, EORIndirectY_LoadValue_PageCrossed)
{
    using namespace m6502;
    testLogicalIndirectIndexedPageCrossed(CPU::INS_EOR_INY);
}

TEST_F(LogicalTests, ORAIndirectY_LoadValue)
{
    using namespace m6502;
    testLogicalIndirectIndexed(CPU::INS_ORA_INY);
}

TEST_F(LogicalTests, ORAIndirectY_LoadValue_PageCrossed)
{
    using namespace m6502;
    testLogicalIndirectIndexedPageCrossed(CPU::INS_ORA_INY);
}

/**
 * BIT TEST TESTING
 */

TEST_F(LogicalTests, BITTestZeroPage_AllFlagsSet)
{
    // arrange:
    using namespace m6502;
    testLogicalBITTestZeroPage(0xC0, true);

    // assert:
    EXPECT_TRUE(cpu.P.bits.Z);
    EXPECT_TRUE(cpu.P.bits.V);
    EXPECT_TRUE(cpu.P.bits.N);
}

TEST_F(LogicalTests, BITTestZeroPage_ZeroNotSet)
{
    using namespace m6502;
    testLogicalBITTestZeroPage(0xC2, false);

    EXPECT_FALSE(cpu.P.bits.Z);
    EXPECT_TRUE(cpu.P.bits.V);
    EXPECT_TRUE(cpu.P.bits.N);
}

TEST_F(LogicalTests, BITTestZeroPage_OverflowNotSet)
{
    using namespace m6502;
    testLogicalBITTestZeroPage(0x9B, false);

    // assert:
    EXPECT_FALSE(cpu.P.bits.Z);
    EXPECT_FALSE(cpu.P.bits.V);
    EXPECT_TRUE(cpu.P.bits.N);
}

TEST_F(LogicalTests, BITTestZeroPage_NegativeNotSet)
{
    using namespace m6502;
    testLogicalBITTestZeroPage(0x1B, false);

    // assert:
    EXPECT_FALSE(cpu.P.bits.Z);
    EXPECT_FALSE(cpu.P.bits.V);
    EXPECT_FALSE(cpu.P.bits.N);
}

TEST_F(LogicalTests, BITTestAbsolute_AllFlagsSet)
{
    // arrange
    using namespace m6502;
    testLogicalBITTestAbsolute(0xC0, true);

    // assert
    EXPECT_TRUE(cpu.P.bits.Z);
    EXPECT_TRUE(cpu.P.bits.V);
    EXPECT_TRUE(cpu.P.bits.N);
}

TEST_F(LogicalTests, BITTestAbsolute_ZeroNotSet)
{
    using namespace m6502;
    testLogicalBITTestAbsolute(0xC2, false);

    EXPECT_FALSE(cpu.P.bits.Z);
    EXPECT_TRUE(cpu.P.bits.V);
    EXPECT_TRUE(cpu.P.bits.N);
}

TEST_F(LogicalTests, BITTestAbsolute_OverflowNotSet)
{
    using namespace m6502;
    testLogicalBITTestAbsolute(0x9B, false);

    // assert:
    EXPECT_FALSE(cpu.P.bits.Z);
    EXPECT_FALSE(cpu.P.bits.V);
    EXPECT_TRUE(cpu.P.bits.N);
}

TEST_F(LogicalTests, BITTestAbsolute_NegativeNotSet)
{
    using namespace m6502;
    testLogicalBITTestAbsolute(0x1B, false);

    // assert:
    EXPECT_FALSE(cpu.P.bits.Z);
    EXPECT_FALSE(cpu.P.bits.V);
    EXPECT_FALSE(cpu.P.bits.N);
}
