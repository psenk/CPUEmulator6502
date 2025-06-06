#include <gtest/gtest.h>
#include "m6502.h"

class RegisterTransferTests : public ::testing::Test
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

    void testRegisterTransfer(const m6502::Byte opcode,
                              m6502::Byte value,
                              m6502::Byte m6502::CPU::*inputRegister,
                              m6502::Byte m6502::CPU::*outputRegister)
    {
        // arrange:
        using namespace m6502;
        static constexpr s_int32 NUM_CYCLES = 2;
        cpu.*inputRegister = value;

        mem[0xFFFC] = opcode;

        // act:
        s_int32 cyclesExecuted = cpu.execute(NUM_CYCLES, mem);

        // assert:
        EXPECT_EQ(cpu.*outputRegister, value);
        EXPECT_EQ(cyclesExecuted, NUM_CYCLES);
    }

    // test flags
    static void testTransferFlagsUnchanged(const m6502::CPU &cpuCopy,
                                           const m6502::CPU &cpu)
    {
        EXPECT_EQ(cpuCopy.P.bits.C, cpu.P.bits.C);
        EXPECT_EQ(cpuCopy.P.bits.I, cpu.P.bits.I);
        EXPECT_EQ(cpuCopy.P.bits.D, cpu.P.bits.D);
        EXPECT_EQ(cpuCopy.P.bits.B, cpu.P.bits.B);
        EXPECT_EQ(cpuCopy.P.bits.V, cpu.P.bits.V);
    }

    static void testAllFlagsUnchanged(const m6502::CPU &cpuCopy,
                                      const m6502::CPU &cpu)
    {
        EXPECT_EQ(cpuCopy.P.value, cpu.P.value);
    }
};

TEST_F(RegisterTransferTests, TAX_LoadValue)
{
    using namespace m6502;
    CPU cpuCopy = cpu;
    testRegisterTransfer(CPU::INS_TAX, 0x42, &CPU::A, &CPU::X);
    testAllFlagsUnchanged(cpuCopy, cpu);
}

TEST_F(RegisterTransferTests, TAX_LoadValue_ZeroValue)
{
    using namespace m6502;
    CPU cpuCopy = cpu;
    testRegisterTransfer(CPU::INS_TAX, 0x00, &CPU::A, &CPU::X);
    EXPECT_TRUE(cpu.P.bits.Z);
    testTransferFlagsUnchanged(cpuCopy, cpu);
}

TEST_F(RegisterTransferTests, TAX_LoadValue_NegativeValue)
{
    using namespace m6502;
    CPU cpuCopy = cpu;
    testRegisterTransfer(CPU::INS_TAX, 0x84, &CPU::A, &CPU::X);
    EXPECT_TRUE(cpu.P.bits.N);
    testTransferFlagsUnchanged(cpuCopy, cpu);
}

TEST_F(RegisterTransferTests, TAY_LoadValue)
{
    using namespace m6502;
    CPU cpuCopy = cpu;
    testRegisterTransfer(CPU::INS_TAY, 0x42, &CPU::A, &CPU::Y);
    testAllFlagsUnchanged(cpuCopy, cpu);
}

TEST_F(RegisterTransferTests, TAY_LoadValue_ZeroValue)
{
    using namespace m6502;
    CPU cpuCopy = cpu;
    testRegisterTransfer(CPU::INS_TAY, 0x00, &CPU::A, &CPU::Y);
    EXPECT_TRUE(cpu.P.bits.Z);
    testTransferFlagsUnchanged(cpuCopy, cpu);
}

TEST_F(RegisterTransferTests, TAY_LoadValue_NegativeValue)
{
    using namespace m6502;
    CPU cpuCopy = cpu;
    testRegisterTransfer(CPU::INS_TAY, 0x84, &CPU::A, &CPU::Y);
    EXPECT_TRUE(cpu.P.bits.N);
    testTransferFlagsUnchanged(cpuCopy, cpu);
}

TEST_F(RegisterTransferTests, TXA_LoadValue)
{
    using namespace m6502;
    CPU cpuCopy = cpu;
    testRegisterTransfer(CPU::INS_TXA, 0x42, &CPU::X, &CPU::A);
    testAllFlagsUnchanged(cpuCopy, cpu);
}

TEST_F(RegisterTransferTests, TXA_LoadValue_ZeroValue)
{

    using namespace m6502;
    CPU cpuCopy = cpu;
    testRegisterTransfer(CPU::INS_TXA, 0x00, &CPU::X, &CPU::A);
    EXPECT_TRUE(cpu.P.bits.Z);
    testTransferFlagsUnchanged(cpuCopy, cpu);
}

TEST_F(RegisterTransferTests, TXA_LoadValue_NegativeValue)
{

    using namespace m6502;
    CPU cpuCopy = cpu;
    testRegisterTransfer(CPU::INS_TXA, 0x84, &CPU::X, &CPU::A);
    EXPECT_TRUE(cpu.P.bits.N);
    testTransferFlagsUnchanged(cpuCopy, cpu);
}

TEST_F(RegisterTransferTests, TYA_LoadValue)
{

    using namespace m6502;
    CPU cpuCopy = cpu;
    testRegisterTransfer(CPU::INS_TYA, 0x42, &CPU::Y, &CPU::A);
    testAllFlagsUnchanged(cpuCopy, cpu);
}

TEST_F(RegisterTransferTests, TYA_LoadValue_ZeroValue)
{

    using namespace m6502;
    CPU cpuCopy = cpu;
    testRegisterTransfer(CPU::INS_TYA, 0x00, &CPU::Y, &CPU::A);
    EXPECT_TRUE(cpu.P.bits.Z);
    testTransferFlagsUnchanged(cpuCopy, cpu);
}

TEST_F(RegisterTransferTests, TYA_LoadValue_NegativeValue)
{

    using namespace m6502;
    CPU cpuCopy = cpu;
    testRegisterTransfer(CPU::INS_TYA, 0x84, &CPU::Y, &CPU::A);
    EXPECT_TRUE(cpu.P.bits.N);
    testTransferFlagsUnchanged(cpuCopy, cpu);
}
