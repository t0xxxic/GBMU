#include "cpu.h"
#include <assert.h>

struct cpu cpu = {0};

define_field_from_type(uint32_t, u32)
define_field_from_type(uint64_t, u64)

bool check_condition(condition_t cond) {
        switch (cond) {
        case COND_EQ:
            return cpu.regs.cpsr.z;

        case COND_NE:
            return !cpu.regs.cpsr.z;

        case COND_CS:
            return cpu.regs.cpsr.c;

        case COND_CC:
            return !cpu.regs.cpsr.c;

        case COND_MI:
            return cpu.regs.cpsr.n;

        case COND_PL:
            return !cpu.regs.cpsr.n;

        case COND_VS:
            return cpu.regs.cpsr.v;

        case COND_VC:
            return !cpu.regs.cpsr.v;

        case COND_HI:
            return cpu.regs.cpsr.c && !cpu.regs.cpsr.z;

        case COND_LS:
            return !cpu.regs.cpsr.c || cpu.regs.cpsr.z;

        case COND_GE:
            return cpu.regs.cpsr.n == cpu.regs.cpsr.v;

        case COND_LT:
            return cpu.regs.cpsr.n != cpu.regs.cpsr.v;

        case COND_GT:
            return !cpu.regs.cpsr.z && cpu.regs.cpsr.n == cpu.regs.cpsr.v;

        case COND_LE:
            return cpu.regs.cpsr.z || cpu.regs.cpsr.n != cpu.regs.cpsr.v;

        case COND_AL:
            return true;

        default:
            assert(false);
            return false;
    }
}

#define LOGICAL_SHIFT_HANDLER(operand)     \
switch (shift_amount)               \
{                                   \
    case 0:                         \
        *carry_out = cpu.regs.cpsr.c;   \
        return rm;                  \
    case 1 ... 32:                  \
        *carry_out = (rm operand (shift_amount - 1)) & 1; \
        return rm operand shift_amount;  \
    default:                        \
        *carry_out = false;         \
        return 0;                   \
}    

uint32_t get_operand(uint32_t insn, bool *carry_out) { 
    bool shift_from_reg = field_from_u32(insn, 4, 1);
    uint8_t rs_amt = field_from_u32(insn, 8, 4);
    uint8_t shift_amount = 0;


    if (field_from_u32(insn, 25, 1)) {
        return field_from_u32(insn, 0, 8) << (rs_amt * 2);
    }

    if (shift_from_reg) { // is reg shift
        assert(field_from_u32(insn, 7, 1) == 0);
        assert(rs_amt != REG_PC);
        shift_amount = cpu.regs.gprs[rs_amt] & 0xFF;
    }
    else { 
        shift_amount = field_from_u32(insn, 7, 5);
    }

    uint32_t rm = cpu.regs.gprs[field_from_u32(insn, 0, 4)];
    switch (field_from_u32(insn, 5, 2)) {

        case LSL_SHIFT: 
            LOGICAL_SHIFT_HANDLER(<<)

        case LSR_SHIFT:
            LOGICAL_SHIFT_HANDLER(>>)

        case ASR_SHIFT:
            switch(shift_amount) {
                case 0:
                    *carry_out = cpu.regs.cpsr.c;
                    return rm;
                case 1 ... 31:
                    *carry_out = (rm >> (shift_amount - 1)) & 1;
                    return ((int32_t) rm) >> shift_amount;
                default:
                    *carry_out = rm & 0x80000000;
                    return ((int32_t) rm) >> shift_amount;      
            }
            
        case ROR_SHIFT:
            rm = rm & 0x1F;
            switch(shift_amount) {
                case 0: 
                    if (shift_from_reg) {
                        *carry_out = cpu.regs.cpsr.c;
                        return rm;
                    }
                    // RRX
                    *carry_out = rm & 1;
                    return (cpu.regs.cpsr.c << 31) | (rm >> 1);

                case 1 ... 32:
                    *carry_out = ror32(rm, shift_amount - 1) & 1;
                    return ror32(rm, shift_amount);

                default:
                    assert(false && "rm can't be more than 32");
            }

        default:
            assert(false && "invalid shift type");
    }
}

void arm_mrs(uint32_t insn) {
    uint8_t rd = field_from_u32(insn, 12, 4);
    union PSR *src_psr = field_from_u32(insn, 22, 1) ? cpu.regs.spsr : &cpu.regs.cpsr;

    assert(rd != REG_PC && "MRS PC is illegal");
    assert(src_psr && "usermode MRS from SPSR is illegal");

    cpu.regs.gprs[rd] = src_psr->value;
}

void arm_msr(uint32_t insn) {
    bool imm_op = field_from_u32(insn, 25, 1);
    bool carry_out = false;
    union PSR source_op = { .value = get_operand(insn, &carry_out) };
    union PSR *dst_psr = field_from_u32(insn, 22, 1) ? cpu.regs.spsr : &cpu.regs.cpsr;

    assert(dst_psr && "usermode MSR from SPSR is illegal");

    dst_psr->value = (dst_psr->value & 0x0FFFFFFF) | (source_op.value & 0xF0000000); 

    if (field_from_u32(insn, 17, 1)) {
        // full PSR
        assert(!imm_op && "full MSR only from register");

        dst_psr->mode = source_op.mode;
        dst_psr->f = source_op.f;
        dst_psr->i = source_op.i;
    }
}

void arm_mul(uint32_t insn) {
    uint8_t rm = field_from_u32(insn, 0, 4),
            rs = field_from_u32(insn, 8, 4),
            rn = field_from_u32(insn, 12, 4),
            rd = field_from_u32(insn, 16, 4);
    uint32_t result = 0;

    assert(rm != REG_PC);
    assert(rs != REG_PC);
    assert(rn != REG_PC);
    assert(rd != REG_PC);
    assert(rd != rm);

    if (field_from_u32(insn, 21, 1)) {
        // MUL
        result = cpu.regs.gprs[rm] * cpu.regs.gprs[rs];
    } else {
        // MULA
        result = cpu.regs.gprs[rm] * cpu.regs.gprs[rs] + cpu.regs.gprs[rn];
    }

    // set flags
    if (field_from_u32(insn, 20, 1)) {
        cpu.regs.cpsr.z = !result;
        cpu.regs.cpsr.n = field_from_u32(result, 31, 1);
    }

    cpu.regs.gprs[rd] = result;
}

void arm_mull(uint32_t insn) {
    uint8_t rm = field_from_u32(insn, 0, 4),
            rs = field_from_u32(insn, 8, 4),
            rdl = field_from_u32(insn, 12, 4),
            rdh = field_from_u32(insn, 16, 4);
    uint64_t result = 0;

    assert(rm != REG_PC);
    assert(rs != REG_PC);
    assert(rdl != REG_PC);
    assert(rdh != REG_PC);

    if (field_from_u32(insn, 21, 1)) {
        // MULL
        result = cpu.regs.gprs[rm] * cpu.regs.gprs[rs];
    } else {
        // MULLA
        result = cpu.regs.gprs[rm] * cpu.regs.gprs[rs] + (((uint64_t) cpu.regs.gprs[rdh] << 32) | cpu.regs.gprs[rdl]);
    }

    // set flags
    if (field_from_u32(insn, 20, 1)) {
        cpu.regs.cpsr.z = !result;
        cpu.regs.cpsr.n = field_from_u64(result, 63, 1);
    }

    cpu.regs.gprs[rdl] = field_from_u64(result, 0, 32);
    cpu.regs.gprs[rdh] = field_from_u64(result, 32, 32);
}

void arm_data_processing(uint32_t insn) {
    uint8_t rd = field_from_u32(insn, 12, 4), 
            rn = field_from_u32(insn, 16, 4);


    uint32_t op1 = cpu.regs.gprs[rn],
             op2 = 0;
    uint64_t result = 0;
    bool carry_out = false;

    op2 = get_operand(insn, &carry_out);

    bool set_flags = field_from_u32(insn, 20, 1);
    bool arithm_op = false;
    switch(field_from_u32(insn, 21, 4)) { // data opcode
        case OPCODE_AND:
        case OPCODE_TST:
            assert(set_flags);
            result = op1 & op2;
            break;

        case OPCODE_EOR:
        case OPCODE_TEQ:
            assert(set_flags);
            result = op1 ^ op2;
            break;
        
        case OPCODE_SUB:
        case OPCODE_CMP:
            assert(set_flags);
            arithm_op = true;
            result = op1 - op2;
            break;
        
        case OPCODE_RSB:
            arithm_op = true;
            result = op2 - op1;
            break;
        
        case OPCODE_ADD:
        case OPCODE_CMN:
            assert(set_flags);
            arithm_op = true;
            result = op1 + op2;
            break;
        
        case OPCODE_ADC:
            arithm_op = true;
            result = op1 + op2 + cpu.regs.cpsr.c;
            break;
        
        case OPCODE_SBC:
            arithm_op = true;
            result = op1 - op2 + cpu.regs.cpsr.c - 1;
            break;
        
        case OPCODE_RSC:
            arithm_op = true;
            result = op2 - op1 + cpu.regs.cpsr.c - 1;
            break;
        
        case OPCODE_ORR:
            result = op1 | op2;
            break;
        
        case OPCODE_MOV:
            result = op2;
            break;
        
        case OPCODE_BIC:
            result = op1 & ~op2;
            break;
        
        case OPCODE_MVN:
            result = ~op2;
            break;
    }

    if  (set_flags) {  
        if (rd == REG_PC) {
            assert(cpu.regs.spsr && "can't transfer SPSR in usermode");
            cpu.regs.cpsr.value = cpu.regs.spsr->value; 
        } else {
            if (arithm_op) {
                cpu.regs.cpsr.v = !!field_from_u64(result, 32, 32);
                cpu.regs.cpsr.c = !!field_from_u64(result, 32, 1);
            } else {
                cpu.regs.cpsr.c = carry_out;
            }

            cpu.regs.cpsr.z = !(uint32_t)result;
            cpu.regs.cpsr.n = field_from_u32(result, 31, 1);
        }
    }

    
}

void arm_bx(uint32_t insn) {
    uint32_t rn = cpu.regs.gprs[field_from_u32(insn, 0, 4)];
    cpu.regs.cpsr.t = cpu.regs.gprs[rn] & 0x1;
    cpu.regs.gprs[REG_PC] = cpu.regs.gprs[rn] & (~0x1);
}

void arm_b(uint32_t insn) {
    int32_t offset = (field_from_u32(insn, 0, 24) << 8) >> 6;

    if (field_from_u32(insn, 24, 1)) {
        cpu.regs.gprs[REG_LR] = cpu.regs.gprs[REG_PC] - sizeof(insn);
    }

    cpu.regs.gprs[REG_PC] += offset;
}

void decode_arm(uint32_t opcode) {
    if (check_condition(field_from_u32(opcode, 28, 4))) {

    }

}