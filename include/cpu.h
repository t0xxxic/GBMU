#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint32_t mask;
    uint32_t value;
} opcode_type_t;

struct PSR {
    union {
        struct {
            uint8_t mode : 5;
            bool t : 1;
            bool fiq_disable : 1;
            bool irq_disable : 1;
            uint32_t reserved : 20;
            bool v : 1; // overflow
            bool c : 1; // carry / borrow / extend
            bool z : 1; // zero
            bool n : 1; // negative / less than
        };

        uint32_t value;
    };
};

struct registers {
    uint32_t gprs[16];
    struct PSR cpsr;
    struct PSR spsr;
} __attribute__((packed));

enum {
	REG_SP = 13,
	REG_LR = 14,
	REG_PC = 15
};

typedef enum {
    USER_MODE       = 0b10000,
    FIQ_MODE        = 0b10001,
    IRQ_MODE        = 0b10010,
    SVC_MODE        = 0b10011,
    ABOERT_MODE     = 0b10111,
    UNDEFINED_MODE  = 0b11011,
    SYSTEM_MODE     = 0b11111,
} cpu_mode_t;

typedef enum {
    COND_EQ = 0b0000,
    COND_NE = 0b0001,
    COND_CS = 0b0010,
    COND_CC = 0b0011,
    COND_MI = 0b0100,
    COND_PL = 0b0101,
    COND_VS = 0b0110,
    COND_VC = 0b0111,
    COND_HI = 0b1000,
    COND_LS = 0b1001,
    COND_GE = 0b1010,
    COND_LT = 0b1011,
    COND_GT = 0b1100,
    COND_LE = 0b1101,
    COND_AL = 0b1110,
    COND_XX = 0b1111,
} condition_t;

typedef enum {
    OPCODE_AND = 0,
    OPCODE_EOR,
    OPCODE_SUB,
    OPCODE_RSB,
    OPCODE_ADD,
    OPCODE_ADC,
    OPCODE_SBC,
    OPCODE_RSC,
    OPCODE_TST,
    OPCODE_TEQ,
    OPCODE_CMP,
    OPCODE_CMN,
    OPCODE_ORR,
    OPCODE_MOV,
    OPCODE_BIC,
    OPCODE_MVN
} data_opcode_t;

typedef union {
    struct {
        struct {
            uint8_t rn      : 4;
            uint32_t unused : 24;
        } bx;

        struct {
            int32_t offset  : 24;
            bool link       : 1;
            uint32_t unused : 3;
        } b;

        struct {
            uint16_t operand2       : 12;
            uint8_t rd              : 4;
            uint8_t rn              : 4;
            bool s                  : 1;
            data_opcode_t opcode    : 4;
            bool immediate          : 1;
        } data_processing;

        uint8_t cond : 4;
    };

    uint32_t value;
} arm_instruction_t __attribute__((packed));

static const opcode_type_t opcode_types[] = {
    {   // Multiply
        .mask   =   0x0FC000F0,
        .value  =   0x00000090
    },
    {   // Multiply Long
        .mask   =   0x0F8000F0,
        .value  =   0x00800090
    }, 
    {   // Single Data Swap
        .mask   =   0x0FB00FF0,
        .value  =   0x01000090
    },
    {   // Branch and Exchange
        .mask   =   0x0FFFFFF0,
        .value  =   0x012FFF10,
    },
    {   // Halfword Data Transfer: register offset
        .mask   =   0x0E400F90,
        .value  =   0x00000090,
    },
    {   // Halfword Data Transfer: immediate offset
        .mask   =   0x0E400090,
        .value  =   0x00400090,
    },
    {   // Single Data Transfer
        .mask   =   0x0C000000,
        .value  =   0x04000000,
    },
    {   // Block Data Transfer
        .mask   =   0x0E000000,
        .value  =   0x08000000,
    },
    {   // Branch
        .mask   =   0x0E000000,
        .value  =   0x0A000000,
    },
    {   // Coprocessor Data Transfer
        .mask   =   0x0E000000,
        .value  =   0x0C000000,
    },
    {   // Coprocessor Data Operation
        .mask   =   0x0F000010,
        .value  =   0x0E000000,
    },
    {   // Coprocessor Register Transfer
        .mask   =   0x0F000010,
        .value  =   0x0E000010,
    },
    {   // Software Interrupt
        .mask   =   0x0F000000,
        .value  =   0x0F000000,
    },
    {   // Data Processing / PSR Transfer
        .mask   =   0x0C000000,
        .value  =   0x00000000,
    },
};