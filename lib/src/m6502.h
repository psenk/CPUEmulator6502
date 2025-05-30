#include <iostream>
#include <stdlib.h>

// http://6502.org/users/obelisk/

// register = small area of memory for computer to access
// - 16 bit addresses

namespace m6502
{
    // custom types
    using Byte = unsigned char;  // 8-bit
    using Word = unsigned short; // 16-bit

    using u32 = unsigned int;
    using s32 = signed int;
    using s8 = signed char;

    struct Mem;
    union StatusFlags;
    struct CPU;
}

/**
 * MEMORY
 */

struct m6502::Mem
{
    static constexpr u32 MAX_MEM = 1024 * 64;
    Byte data[MAX_MEM];

    /* initialize memory */
    void initialize()
    {
        for (u32 i = 0; i < MAX_MEM; i++)
        {
            data[i] = 0;
        }
    }

    /* read one byte from memory
       defining custom [] operator */
    Byte operator[](u32 address) const
    {
        return data[address];
    }

    /* write one byte to memory
       defining custom [] operator */
    Byte &operator[](u32 address)
    {
        return data[address];
    }
};

union m6502::StatusFlags
{
    struct
    {
        Byte C : 1; // carry flag
        Byte Z : 1; // zero flag
        Byte I : 1; // interrupt disable
        Byte D : 1; // decimal mode
        Byte B : 1; // break command
        Byte U : 1; // unused
        Byte V : 1; // overflow flag
        Byte N : 1; // negative flag
    } bits;
    Byte value;
};

/**
 * CPU
 */

struct m6502::CPU
{
    /*
    Zero Page: $0000 - $00FF - 256 bytes
    Stack: $0100 - $01FF - 256 bytes
    Non-maskable Interrupt Handler: $FFFA/B
    Power on Reset: $FFFC/D
    BRK/Interrupt Request Handler: $FFFE/F
    */

    Word PC; // program counter
    Byte SP; // stack pointer

    Byte A, X, Y; // registers
    enum RegisterType;

    StatusFlags P;

    /**
     * RESET COMMAND
     */

    void reset(Mem &memory, Word vector = 0xFFFC)
    {
        PC = vector;
        SP = 0xFF;
        A = X = Y = 0;
        P.value = 0x24;
        memory.initialize();
    }

    /**
     * READ FUNCTIONS
     */

    /* read next byte from memory */
    Byte readNextByte(s32 &cycles, const Mem &memory)
    {
        Byte data = memory[PC];
        PC++;
        cycles--;
        return data;
    }

    /* read next word from memory
       little endian */
    Word readNextWord(s32 &cycles, const Mem &memory)
    {
        Word data = memory[PC];
        PC++;

        data |= (memory[PC] << 8);
        PC++;

        // handle endianness?

        cycles -= 2;
        return data;
    }

    /* read byte from memory */
    Byte readByteFromAddress(s32 &cycles, Word address, const Mem &memory)
    {
        cycles--;
        return memory[address];
    }

    /* read one word from address
       little endian */
    Word readWordFromAddress(s32 &cycles, Word address, const Mem &memory)
    {

        Word data = memory[address];
        address++;

        data |= (memory[address] << 8);

        // handle endianness?

        cycles -= 2;
        return data;
    }

    /**
     * WRITE FUNCTIONS
     */

    /* write one byte to memory */
    void writeByte(s32 &cycles, Byte value, Word address, Mem &memory)
    {
        memory[address] = value;
        cycles -= 1;
    }

    /* write one word to memory */
    void writeWord(s32 &cycles, Word value, Word address, Mem &memory)
    {
        memory[address] = value & 0xFF;
        memory[address + 1] = value >> 8;
        cycles -= 2;
    }

    /**
     * STACK FUNCTIONS
     */

    // convert the 8 bit stack pointer to a 16 bit address
    // the 0x01 high byte is assumed because 
    // the full stack is on the $01 page
    Word spToAddress() const
    {
        return 0x0100 | SP;
    }

    // push byte to the stack
    void pushByteToStack(s32 &cycles, Byte value, Mem &memory)
    {
        writeByte(cycles, value, spToAddress(), memory);
        SP--;
    }

    // push word to the stack
    // little endian
    void pushWordToStack(s32 &cycles, Word value, Mem &memory)
    {
        pushByteToStack(cycles, (value >> 8) & 0xFF, memory);
        pushByteToStack(cycles, value & 0xFF, memory);
    }

    // pop byte off the stack
    Byte popByteFromStack(s32 &cycles, Mem &memory)
    {
        SP++;
        cycles--; // for SP incrementing
        return readByteFromAddress(cycles, spToAddress(), memory);
    }

    // pop word off the stack
    Word popWordFromStack(s32 &cycles, Mem &memory)
    {
        Byte lowByte = popByteFromStack(cycles, memory);
        Byte highByte = popByteFromStack(cycles, memory);
        return (highByte << 8) | lowByte;
    }

    static constexpr Byte CARRY_FLAG_BIT = 0x01;
    static constexpr Byte ZERO_FLAG_BIT = 0x02;
    static constexpr Byte INTERRUPT_FLAG_BIT = 0x04;
    static constexpr Byte DECIMAL_FLAG_BIT = 0x08;
    static constexpr Byte BREAK_FLAG_BIT = 0x10;
    static constexpr Byte OVERFLOW_FLAG_BIT = 0x40;
    static constexpr Byte NEGATIVE_FLAG_BIT = 0x80;

    static constexpr Word IRQ_VECTOR_LOW = 0xFFFE;
    static constexpr Word IRQ_VECTOR_HIGH = 0xFFFF;

    /**
     * INSTRUCTION OPCODES
     */

    // LDA instructions
    static constexpr Byte INS_LDA_IMM = 0xA9,
                          INS_LDA_ZPG = 0xA5,
                          INS_LDA_ZPX = 0xB5,
                          INS_LDA_ABS = 0xAD,
                          INS_LDA_ABX = 0xBD,
                          INS_LDA_ABY = 0xB9,
                          INS_LDA_INX = 0xA1,
                          INS_LDA_INY = 0xB1;

    // LDX instructions
    static constexpr Byte INS_LDX_IMM = 0xA2,
                          INS_LDX_ZPG = 0xA6,
                          INS_LDX_ZPY = 0xB6,
                          INS_LDX_ABS = 0xAE,
                          INS_LDX_ABY = 0xBE;

    // LDY instructions
    static constexpr Byte INS_LDY_IMM = 0xA0,
                          INS_LDY_ZPG = 0xA4,
                          INS_LDY_ZPX = 0xB4,
                          INS_LDY_ABS = 0xAC,
                          INS_LDY_ABX = 0xBC;

    // STA instructions
    static constexpr Byte INS_STA_ZPG = 0x85,
                          INS_STA_ZPX = 0x95,
                          INS_STA_ABS = 0x8D,
                          INS_STA_ABX = 0x9D,
                          INS_STA_ABY = 0x99,
                          INS_STA_INX = 0x81,
                          INS_STA_INY = 0x91;

    // STX instructions
    static constexpr Byte INS_STX_ZPG = 0x86,
                          INS_STX_ZPY = 0x96,
                          INS_STX_ABS = 0x8E;

    // STY instructions
    static constexpr Byte INS_STY_ZPG = 0x84,
                          INS_STY_ZPX = 0x94,
                          INS_STY_ABS = 0x8C;

    // jump and call instructions
    static constexpr Byte INS_JMP_ABS = 0x4C,
                          INS_JMP_IND = 0x6C,
                          INS_JSR_ABS = 0x20,
                          INS_RTS = 0x60;

    // stack operation instructions
    static constexpr Byte INS_TSX = 0xBA,
                          INS_TXS = 0x9A,
                          INS_PHA = 0x48,
                          INS_PHP = 0x08,
                          INS_PLA = 0x68,
                          INS_PLP = 0x28;

    // logical instructions
    static constexpr Byte INS_AND_IMM = 0x29,
                          INS_AND_ZPG = 0x25,
                          INS_AND_ZPX = 0x35,
                          INS_AND_ABS = 0x2D,
                          INS_AND_ABX = 0x3D,
                          INS_AND_ABY = 0x39,
                          INS_AND_INX = 0x21,
                          INS_AND_INY = 0x31,
                          INS_EOR_IMM = 0x49,
                          INS_EOR_ZPG = 0x45,
                          INS_EOR_ZPX = 0x55,
                          INS_EOR_ABS = 0x4D,
                          INS_EOR_ABX = 0x5D,
                          INS_EOR_ABY = 0x59,
                          INS_EOR_INX = 0x41,
                          INS_EOR_INY = 0x51,
                          INS_ORA_IMM = 0x09,
                          INS_ORA_ZPG = 0x05,
                          INS_ORA_ZPX = 0x15,
                          INS_ORA_ABS = 0x0D,
                          INS_ORA_ABX = 0x1D,
                          INS_ORA_ABY = 0x19,
                          INS_ORA_INX = 0x01,
                          INS_ORA_INY = 0x11,
                          INS_BIT_ZPG = 0x24,
                          INS_BIT_ABS = 0x2C;

    // register transfer instructions
    static constexpr Byte INS_TAX = 0xAA,
                          INS_TAY = 0xA8,
                          INS_TXA = 0x8A,
                          INS_TYA = 0x98;

    // increment and decrement instructions
    static constexpr Byte INS_INC_ZPG = 0x56,
                          INS_INC_ZPX = 0xF6,
                          INS_INC_ABS = 0xEE,
                          INS_INC_ABX = 0xFE,
                          INS_INX = 0xE8,
                          INS_INY = 0xC8,
                          INS_DEC_ZPG = 0xC6,
                          INS_DEC_ZPX = 0xD6,
                          INS_DEC_ABS = 0xCE,
                          INS_DEC_ABX = 0xDE,
                          INS_DEX = 0xCA,
                          INS_DEY = 0x88;

    // branch instructions
    static constexpr Byte INS_BCC = 0x90,
                          INS_BCS = 0xB0,
                          INS_BNE = 0xD0,
                          INS_BEQ = 0xF0,
                          INS_BPL = 0x10,
                          INS_BMI = 0x30,
                          INS_BVC = 0x50,
                          INS_BVS = 0x70;

    // status flag change instructions
    static constexpr Byte INS_CLC = 0x18,
                          INS_CLD = 0xD8,
                          INS_CLI = 0x58,
                          INS_CLV = 0xB8,
                          INS_SEC = 0x38,
                          INS_SED = 0xF8,
                          INS_SEI = 0x78;

    // system function instructions
    static constexpr Byte INS_BRK = 0x00,
                          INS_NOP = 0xEA;

    /**
     * ADDRESSING MODES
     */

    s32 fetchAddressZeroPage(s32 &cycles, const Mem &memory);
    s32 fetchAddressZeroPagePlusRegister(s32 &cycles, RegisterType reg, const Mem &memory);
    s32 fetchAddressAbsolute(s32 &cycles, const Mem &memory);
    s32 fetchAddressAbsolutePlusRegister(s32 &cycles, RegisterType reg, const Mem &memory);
    s32 fetchAddressIndexedIndirect(s32 &cycles, const Mem &memory);
    s32 fetchAddressIndirectIndexed(s32 &cycles, const Mem &memory, bool extraCycle);

    /**
     * SET FLAGS
     */

    void setFlagStatus_NZ(Byte reg);
    void setFlagStatus_BIT(Byte value);

    /**
     * CPU FUNCTIONS
     */

    s32 execute(s32 cycles, Mem &memory);
    Word loadProgram(Byte *program, u32 numBytes, Mem &memory);
};