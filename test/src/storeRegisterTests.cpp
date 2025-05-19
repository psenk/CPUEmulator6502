#include <gtest/gtest.h>
#include "m6502.h"

class StoreRegisterTests : public ::testing::Test
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

    // load zero page
    void testStoreZeroPage(m6502::Byte opcode,
                          m6502::Byte m6502::CPU::*reg);

    // load zero page + register
    void testStoreZeroPagePlusRegister(m6502::Byte opcode,
                                      m6502::Byte m6502::CPU::*addedReg,
                                      m6502::Byte m6502::CPU::*reg);
    void testStoreZeroPagePlusRegisterWrapAround(m6502::Byte opcode,
                                                m6502::Byte m6502::CPU::*addedReg,
                                                m6502::Byte m6502::CPU::*reg);

    // load absolute
    void testStoreAbsolute(m6502::Byte opcode,
                          m6502::Byte m6502::CPU::*reg);

    // load absolute + register
    void testStoreAbsolutePlusRegister(m6502::Byte opcode,
                                      m6502::Byte m6502::CPU::*addedReg,
                                      m6502::Byte m6502::CPU::*reg);
    void testLoadAbsolutePlusRegisterPageCrossed(m6502::Byte opcode,
                                                 m6502::Byte m6502::CPU::*addedReg,
                                                 m6502::Byte m6502::CPU::*reg);

    // test flags
    static void testStoreFlagsUnchanged(const m6502::CPU &cpuCopy,
                                        const m6502::CPU &cpu)
    {
        EXPECT_EQ(cpuCopy.P.value, cpu.P.value);
    }
};

/**
 * STORE ZERO PAGE TESTING
 */

void StoreRegisterTests::testStoreZeroPage(
    m6502::Byte opcode,
    m6502::Byte m6502::CPU::*reg)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s32 NUM_CYCLES = 3;

    cpu.*reg = 0x42;
    mem[0xFFFC] = opcode;
    mem[0xFFFD] = 0x73;
    
    // act:
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(mem[0x0073], 0x42);
    EXPECT_EQ(cyclesExecuted, 3);
    testStoreFlagsUnchanged(cpuCopy, cpu);
}

TEST_F(StoreRegisterTests, STAZeroPage_StoreValue)
{
    using namespace m6502;
    testStoreZeroPage(CPU::INS_STA_ZPG, &CPU::A);
}

TEST_F(StoreRegisterTests, STXZeroPage_StoreValue)
{
    using namespace m6502;
    testStoreZeroPage(CPU::INS_STX_ZPG, &CPU::X);
}

TEST_F(StoreRegisterTests, STYZeroPage_StoreValue)
{
    using namespace m6502;
    testStoreZeroPage(CPU::INS_STY_ZPG, &CPU::Y);
}

/**
 * STORE ZERO PAGE + REGISTER TESTING
 */

void StoreRegisterTests::testStoreZeroPagePlusRegister(
    m6502::Byte opcode,
    m6502::Byte m6502::CPU::*addedReg,
    m6502::Byte m6502::CPU::*reg)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s32 NUM_CYCLES = 4;
    cpu.*reg = 0x42;
    cpu.*addedReg = 0x05;

    mem[0xFFFC] = opcode;
    mem[0xFFFD] = 0x73;

    // act:
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(mem[0x0078], 0x42);
    EXPECT_EQ(cyclesExecuted, 4);
    testStoreFlagsUnchanged(cpuCopy, cpu);
}

void StoreRegisterTests::testStoreZeroPagePlusRegisterWrapAround(
    m6502::Byte opcode,
    m6502::Byte m6502::CPU::*addedReg,
    m6502::Byte m6502::CPU::*reg)
{
    // arrange:
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s32 NUM_CYCLES = 4;
    cpu.*reg = 0x42;
    cpu.*addedReg = 0xFF;

    mem[0xFFFC] = opcode;
    mem[0xFFFD] = 0x80;

    // act:
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert:
    EXPECT_EQ(mem[0x007F], 0x42);
    EXPECT_EQ(cyclesExecuted, 4);
    testStoreFlagsUnchanged(cpuCopy, cpu);
}

TEST_F(StoreRegisterTests, STAZeroPageX_StoreValue)
{
    using namespace m6502;
    testStoreZeroPagePlusRegister(CPU::INS_STA_ZPX, &CPU::X, &CPU::A);
}

TEST_F(StoreRegisterTests, STXZeroPageY_StoreValue)
{
    using namespace m6502;
    testStoreZeroPagePlusRegister(CPU::INS_STX_ZPY, &CPU::Y, &CPU::X);
}

TEST_F(StoreRegisterTests, STYZeroPageX_StoreValue)
{
    using namespace m6502;
    testStoreZeroPagePlusRegister(CPU::INS_STY_ZPX, &CPU::X, &CPU::Y);
}

TEST_F(StoreRegisterTests, STAZeroPageX_StoreValue_WrapAround)
{
    using namespace m6502;
    testStoreZeroPagePlusRegisterWrapAround(CPU::INS_STA_ZPX, &CPU::X, &CPU::A);
}

TEST_F(StoreRegisterTests, STXZeroPageY_StoreValue_WrapAround)
{
    using namespace m6502;
    testStoreZeroPagePlusRegister(CPU::INS_STX_ZPY, &CPU::Y, &CPU::X);
}

TEST_F(StoreRegisterTests, STYZeroPageX_StoreValue_WrapAround)
{
    using namespace m6502;
    testStoreZeroPagePlusRegister(CPU::INS_STY_ZPX, &CPU::X, &CPU::Y);
}

/**
 * STORE ABSOLUTE TESTING
 */

void StoreRegisterTests::testStoreAbsolute(
    m6502::Byte opcode,
    m6502::Byte m6502::CPU::*reg)
{
    // arrange
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s32 NUM_CYCLES = 4;
    cpu.*reg = 0x42;

    mem[0xFFFC] = opcode;
    mem[0xFFFD] = 0x20;
    mem[0xFFFE] = 0x04;

    // act
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert
    EXPECT_EQ(mem[0x0420], 0x42);
    EXPECT_EQ(cyclesExecuted, 4);
    testStoreFlagsUnchanged(cpuCopy, cpu);
}

TEST_F(StoreRegisterTests, STAAbsolute_StoreValue)
{
    using namespace m6502;
    testStoreAbsolute(CPU::INS_STA_ABS, &CPU::A);
}

TEST_F(StoreRegisterTests, STXAbsolute_StoreValue)
{
    using namespace m6502;
    testStoreAbsolute(CPU::INS_STX_ABS, &CPU::X);
}

TEST_F(StoreRegisterTests, STYAbsolute_StoreValue)
{
    using namespace m6502;
    testStoreAbsolute(CPU::INS_STY_ABS, &CPU::Y);
}

/**
 * STORE ABSOLUTE + REGISTER
 */

void StoreRegisterTests::testStoreAbsolutePlusRegister(
    m6502::Byte opcode,
    m6502::Byte m6502::CPU::*addedReg,
    m6502::Byte m6502::CPU::*reg)
{
    // arrange
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s32 NUM_CYCLES = 5;
    cpu.*reg = 0x42;
    cpu.*addedReg = 0x05;

    mem[0xFFFC] = opcode;
    mem[0xFFFD] = 0x20;
    mem[0xFFFE] = 0x04;

    // act
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert
    EXPECT_EQ(cpu.*reg, 0x42);
    EXPECT_EQ(cyclesExecuted, 5);
    testStoreFlagsUnchanged(cpuCopy, cpu);
}

TEST_F(StoreRegisterTests, STAAbsoluteX_StoreValue)
{
    using namespace m6502;
    testStoreAbsolutePlusRegister(CPU::INS_STA_ABX, &CPU::X, &CPU::A);
}

TEST_F(StoreRegisterTests, STAAbsoluteY_StoreValue)
{
    using namespace m6502;
    testStoreAbsolutePlusRegister(CPU::INS_STA_ABY, &CPU::Y, &CPU::A);
}

/**
 * STORE INDEXED INDIRECT TESTING
 */

TEST_F(StoreRegisterTests, STAIndirectX_LoadValue)
{
    // arrange
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s32 NUM_CYCLES = 6;
    cpu.A = 0x42;
    cpu.X = 0x04;

    mem[0xFFFC] = CPU::INS_STA_INX;
    mem[0xFFFD] = 0x73;
    mem[0x0077] = 0x20;
    mem[0x0078] = 0x04;

    // act
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert
    EXPECT_EQ(mem[0x0420], 0x42);
    EXPECT_EQ(cyclesExecuted, 6);
    testStoreFlagsUnchanged(cpuCopy, cpu);
}

/**
 * STORE INDIRECT INDEXED TESTING
 */

TEST_F(StoreRegisterTests, STAIndirectY_LoadValue)
{
    // arrange
    using namespace m6502;
    CPU cpuCopy = cpu;
    static constexpr s32 NUM_CYCLES = 6;
    cpu.A = 0x42;
    cpu.Y = 0x05;

    mem[0xFFFC] = CPU::INS_STA_INY;
    mem[0xFFFD] = 0x73;
    mem[0x0073] = 0x20;
    mem[0x0074] = 0x04;

    // act
    s32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

    // assert
    EXPECT_EQ(mem[0x0425], 0x42);
    EXPECT_EQ(cyclesExecuted, 6);
    testStoreFlagsUnchanged(cpuCopy, cpu);
}