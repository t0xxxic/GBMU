#include "cpu.h"
#include <assert.h>

struct registers regs = { 0 };

bool check_condition(condition_t cond) {
        switch (cond) {
        case COND_EQ:
            return regs.cpsr.z;

        case COND_NE:
            return !regs.cpsr.z;

        case COND_CS:
            return regs.cpsr.c;

        case COND_CC:
            return !regs.cpsr.c;

        case COND_MI:
            return regs.cpsr.n;

        case COND_PL:
            return !regs.cpsr.n;

        case COND_VS:
            return regs.cpsr.v;

        case COND_VC:
            return !regs.cpsr.v;

        case COND_HI:
            return regs.cpsr.c && !regs.cpsr.z;

        case COND_LS:
            return !regs.cpsr.c || regs.cpsr.z;

        case COND_GE:
            return regs.cpsr.n == regs.cpsr.v;

        case COND_LT:
            return regs.cpsr.n != regs.cpsr.v;

        case COND_GT:
            return !regs.cpsr.z && regs.cpsr.n == regs.cpsr.v;

        case COND_LE:
            return regs.cpsr.z || regs.cpsr.n != regs.cpsr.v;

        case COND_AL:
            return true;

        default:
            assert(false);
            return false;
    }
}

void arm_data_processing(arm_instruction_t insn) {
    switch(insn.data_processing.opcode) {
        case OPCODE_AND:
        
    }
}

void arm_bx(arm_instruction_t insn) {
    regs.cpsr.t = regs.gprs[insn.bx.rn] & 0x1;
    regs.gprs[REG_PC] = regs.gprs[insn.bx.rn] & (~0x1);
}

void arm_b(arm_instruction_t insn) {
    int32_t offset = (insn.b.offset << 8) >> 6;

    if (insn.b.link) {
        regs.gprs[REG_LR] = regs.gprs[REG_PC] - sizeof(insn);
    }

    regs.gprs[REG_PC] += offset;
}

void decode_arm(uint32_t opcode) {
    uint16_t index = (opcode >> 16) & 0xFF0 | ((opcode & 0xF0) >> 4);
    arm_instruction_t insn = {.value = opcode};    

    if (check_condition(insn.cond)) {

    }

}