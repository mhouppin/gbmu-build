/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   processor.c                                      .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: mhouppin <mhouppin@le-101.fr>              +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/08 11:02:11 by mhouppin     #+#   ##    ##    #+#       */
/*   Updated: 2019/09/16 08:58:34 by mhouppin    ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include <unistd.h>
#include "memory_map.h"
#include "registers.h"
#include "bitmask.h"
#include "processor.h"

#define ADD_PC(offset) regs->reg_pc += (offset)
#define SET_PC(value) regs->reg_pc = (value)

#define WRITE_8(address, value) \
	regs->wbytes = 1; \
	regs->waddr_1 = (address); \
	regs->wval_1 = (value)

#define WRITE_16(address, value) \
	regs->wbytes = 2; \
	regs->waddr_1 = (address); \
	regs->wval_1 = (uint8_t)(value); \
	regs->waddr_2 = (address + 1); \
	regs->wval_2 = (uint8_t)((value) >> 8)

#define INC_REG_8(reg) \
	regs->reg_f &= FLAG_CY; \
	(reg) += 1; \
	if ((reg) == 0) \
		regs->reg_f |= FLAG_Z; \
	if (((reg) & 0xFu) == 0) \
		regs->reg_f |= FLAG_H

#define DEC_REG_8(reg) \
	regs->reg_f &= FLAG_CY; \
	regs->reg_f |= FLAG_N; \
	(reg) -= 1; \
	if ((reg) == 0) \
		regs->reg_f |= FLAG_Z; \
	if (((reg) & 0xFu) == 0xFu) \
		regs->reg_f |= FLAG_H

#define ADD_HL_REG_16(reg) \
	regs->reg_f &= FLAG_Z; \
	imm_16 = regs->reg_hl + (reg); \
	if (imm_16 < regs->reg_hl) \
		regs->reg_f |= FLAG_CY; \
	if ((imm_16 & 0xFFFu) < (regs->reg_hl & 0xFFFu)) \
		regs->reg_f |= FLAG_H; \
	regs->reg_hl = imm_16

#define LD_BLOCK(left, right) \
	ld_ ## left ## _ ## right: \
	ADD_PC(1); \
	regs->reg_ ## left = regs->reg_ ## right; \
	return (4)

#define LD_BLOCK_HL(left) \
	ld_ ## left ## _hl: \
	ADD_PC(1); \
	address = GET_REAL_ADDR(regs->reg_hl); \
	regs->reg_ ## left = *address; \
	return (8)

#define LD_HL_BLOCK(right) \
	ld_hl_ ## right: \
	ADD_PC(1); \
	WRITE_8(regs->reg_hl, regs->reg_ ## right); \
	return (8)

#define ADD_A_BLOCK(right) \
	add_a_ ## right: \
	ADD_PC(1); \
	regs->reg_f = 0; \
	imm_8 = regs->reg_a + regs->reg_ ## right; \
	if (imm_8 < regs->reg_a) \
		regs->reg_f |= FLAG_CY; \
	if ((imm_8 & 0xFu) < (regs->reg_a & 0xFu)) \
		regs->reg_f |= FLAG_H; \
	if (imm_8 == 0) \
		regs->reg_f |= FLAG_Z; \
	regs->reg_a = imm_8; \
	return (4)

#define ADC_A_BLOCK(right) \
	adc_a_ ## right: \
	ADD_PC(1); \
	imm_8 = regs->reg_a + regs->reg_ ## right; \
	opcode = (regs->reg_f & FLAG_CY) == FLAG_CY; \
	regs->reg_f = 0; \
	imm_8 += (opcode) ? 1 : 0; \
	if (imm_8 < regs->reg_a || (imm_8 == regs->reg_a && opcode)) \
		regs->reg_f |= FLAG_CY; \
	if ((imm_8 & 0xFu) < (regs->reg_a & 0xFu) || \
		((imm_8 & 0xFu) == (regs->reg_a & 0xFu) && opcode)) \
		regs->reg_f |= FLAG_H; \
	if (imm_8 == 0) \
		regs->reg_f |= FLAG_Z; \
	regs->reg_a = imm_8; \
	return (4)

#define SUB_A_BLOCK(right) \
	sub_a_ ## right: \
	ADD_PC(1); \
	regs->reg_f = FLAG_N; \
	imm_8 = regs->reg_a - regs->reg_ ## right; \
	if (imm_8 > regs->reg_a) \
		regs->reg_f |= FLAG_CY; \
	if ((imm_8 & 0xFu) > (regs->reg_a & 0xFu)) \
		regs->reg_f |= FLAG_H; \
	if (imm_8 == 0) \
		regs->reg_f |= FLAG_Z; \
	regs->reg_a = imm_8; \
	return (4)

#define SBC_A_BLOCK(right) \
	sbc_a_ ## right: \
	ADD_PC(1); \
	imm_8 = regs->reg_a - regs->reg_ ## right; \
	opcode = (regs->reg_f & FLAG_CY) == FLAG_CY; \
	regs->reg_f = FLAG_N; \
	imm_8 -= (opcode) ? 1 : 0; \
	if (imm_8 > regs->reg_a || (imm_8 == regs->reg_a && opcode)) \
		regs->reg_f |= FLAG_CY; \
	if ((imm_8 & 0xFu) > (regs->reg_a & 0xFu) || \
		((imm_8 & 0xFu) == (regs->reg_a & 0xFu) && opcode)) \
		regs->reg_f |= FLAG_H; \
	if (imm_8 == 0) \
		regs->reg_f |= FLAG_Z; \
	regs->reg_a = imm_8; \
	return (4)

#define AND_A_BLOCK(right) \
	and_a_ ## right: \
	ADD_PC(1); \
	regs->reg_f = FLAG_H; \
	regs->reg_a &= regs->reg_ ## right; \
	if (regs->reg_a == 0) \
		regs->reg_f |= FLAG_Z; \
	return (4)

#define XOR_A_BLOCK(right) \
	xor_a_ ## right: \
	ADD_PC(1); \
	regs->reg_f = 0; \
	regs->reg_a ^= regs->reg_ ## right; \
	if (regs->reg_a == 0) \
		regs->reg_f |= FLAG_Z; \
	return (4)

#define OR_A_BLOCK(right) \
	or_a_ ## right: \
	ADD_PC(1); \
	regs->reg_f = 0; \
	regs->reg_a |= regs->reg_ ## right; \
	if (regs->reg_a == 0) \
		regs->reg_f |= FLAG_Z; \
	return (4)

#define CP_A_BLOCK(right) \
	cp_a_ ## right: \
	ADD_PC(1); \
	regs->reg_f = FLAG_N; \
	if (regs->reg_a == regs->reg_ ## right) \
		regs->reg_f |= FLAG_Z; \
	if (regs->reg_a < regs->reg_ ## right) \
		regs->reg_f |= FLAG_CY; \
	if ((regs->reg_a & 0xFu) < (regs->reg_ ## right & 0xFu)) \
		regs->reg_f |= FLAG_H; \
	return (4)

#define RET_IF(condition) \
	if (condition) \
	{ \
		POP_REG_16(regs->reg_pc); \
		return (20); \
	} \
	ADD_PC(1); \
	return (8)

#define POP_REG_16(reg) \
	address = GET_REAL_ADDR(regs->reg_sp); \
	reg = *address; \
	regs->reg_sp += 1; \
	address = GET_REAL_ADDR(regs->reg_sp); \
	reg |= ((uint16_t)*address) << 8; \
	regs->reg_sp += 1

#define PUSH_REG_16(reg) \
	regs->reg_sp -= 2; \
	WRITE_16(regs->reg_sp, reg)

#define JP_IF(condition) \
	if (condition) \
	{ \
		SET_PC(imm_16); \
		return (16); \
	} \
	ADD_PC(3); \
	return (12)

#define CALL_IF(condition) \
	ADD_PC(3); \
	if (condition) \
	{ \
		regs->reg_sp -= 2; \
		WRITE_16(regs->reg_sp, regs->reg_pc); \
		SET_PC(imm_16); \
		return (24); \
	} \
	return (12)

#define CALL(value) \
	regs->reg_sp -= 2; \
	WRITE_16(regs->reg_sp, regs->reg_pc); \
	SET_PC(value)

void	dump(uint8_t *, uint8_t, registers_t *);

cycle_count_t	execute(registers_t *regs)
{
	static const void *const	insn_jumps[256] =
	{
		&&nop,				&&ld_bc_imm16,		&&ld_bc_a,			&&inc_bc,
		&&inc_b,			&&dec_b,			&&ld_b_imm8,		&&rlca,
		&&ld_addr16_sp,		&&add_hl_bc,		&&ld_a_bc,			&&dec_bc,
		&&inc_c,			&&dec_c,			&&ld_c_imm8,		&&rrca,
		&&stop,				&&ld_de_imm16,		&&ld_de_a,			&&inc_de,
		&&inc_d,			&&dec_d,			&&ld_d_imm8,		&&rla,
		&&jr_imm8,			&&add_hl_de,		&&ld_a_de,			&&dec_de,
		&&inc_e,			&&dec_e,			&&ld_e_imm8,		&&rra,
		&&jrnz_imm8,		&&ld_hl_imm16,		&&ld_hli_a,			&&inc_hl,
		&&inc_h,			&&dec_h,			&&ld_h_imm8,		&&daa,
		&&jrz_imm8,			&&add_hl_hl,		&&ld_a_hli,			&&dec_hl,
		&&inc_l,			&&dec_l,			&&ld_l_imm8,		&&cpl,
		&&jrnc_imm8,		&&ld_sp_imm16,		&&ld_hld_a,			&&inc_sp,
		&&inc_ahl,			&&dec_ahl,			&&ld_hl_imm8,		&&scf,
		&&jrc_imm8,			&&add_hl_sp,		&&ld_a_hld,			&&dec_sp,
		&&inc_a,			&&dec_a,			&&ld_a_imm8,		&&ccf,
		&&nop,				&&ld_b_c,			&&ld_b_d,			&&ld_b_e,
		&&ld_b_h,			&&ld_b_l,			&&ld_b_hl,			&&ld_b_a,
		&&ld_c_b,			&&nop,				&&ld_c_d,			&&ld_c_e,
		&&ld_c_h,			&&ld_c_l,			&&ld_c_hl,			&&ld_c_a,
		&&ld_d_b,			&&ld_d_c,			&&nop,				&&ld_d_e,
		&&ld_d_h,			&&ld_d_l,			&&ld_d_hl,			&&ld_d_a,
		&&ld_e_b,			&&ld_e_c,			&&ld_e_d,			&&nop,
		&&ld_e_h,			&&ld_e_l,			&&ld_e_hl,			&&ld_e_a,
		&&ld_h_b,			&&ld_h_c,			&&ld_h_d,			&&ld_h_e,
		&&nop,				&&ld_h_l,			&&ld_h_hl,			&&ld_h_a,
		&&ld_l_b,			&&ld_l_c,			&&ld_l_d,			&&ld_l_e,
		&&ld_l_h,			&&nop,				&&ld_l_hl,			&&ld_l_a,
		&&ld_hl_b,			&&ld_hl_c,			&&ld_hl_d,			&&ld_hl_e,
		&&ld_hl_h,			&&ld_hl_l,			&&halt,				&&ld_hl_a,
		&&ld_a_b,			&&ld_a_c,			&&ld_a_d,			&&ld_a_e,
		&&ld_a_h,			&&ld_a_l,			&&ld_a_hl,			&&nop,
		&&add_a_b,			&&add_a_c,			&&add_a_d,			&&add_a_e,
		&&add_a_h,			&&add_a_l,			&&add_a_hl,			&&add_a_a,
		&&adc_a_b,			&&adc_a_c,			&&adc_a_d,			&&adc_a_e,
		&&adc_a_h,			&&adc_a_l,			&&adc_a_hl,			&&adc_a_a,
		&&sub_a_b,			&&sub_a_c,			&&sub_a_d,			&&sub_a_e,
		&&sub_a_h,			&&sub_a_l,			&&sub_a_hl,			&&sub_a_a,
		&&sbc_a_b,			&&sbc_a_c,			&&sbc_a_d,			&&sbc_a_e,
		&&sbc_a_h,			&&sbc_a_l,			&&sbc_a_hl,			&&sbc_a_a,
		&&and_a_b,			&&and_a_c,			&&and_a_d,			&&and_a_e,
		&&and_a_h,			&&and_a_l,			&&and_a_hl,			&&and_a_a,
		&&xor_a_b,			&&xor_a_c,			&&xor_a_d,			&&xor_a_e,
		&&xor_a_h,			&&xor_a_l,			&&xor_a_hl,			&&xor_a_a,
		&&or_a_b,			&&or_a_c,			&&or_a_d,			&&or_a_e,
		&&or_a_h,			&&or_a_l,			&&or_a_hl,			&&or_a_a,
		&&cp_a_b,			&&cp_a_c,			&&cp_a_d,			&&cp_a_e,
		&&cp_a_h,			&&cp_a_l,			&&cp_a_hl,			&&cp_a_a,
		&&retnz,			&&pop_bc,			&&jpnz_imm16,		&&jp_imm16,
		&&callnz_imm16,		&&push_bc,			&&add_a_imm8,		&&rst_00h,
		&&retz,				&&ret,				&&jpz_imm16,		&&prefix_cb,
		&&callz_imm16,		&&call_imm16,		&&adc_a_imm8,		&&rst_08h,
		&&retnc,			&&pop_de,			&&jpnc_imm16,		&&illegal,
		&&callnc_imm16,		&&push_de,			&&sub_a_imm8,		&&rst_10h,
		&&retc,				&&reti,				&&jpc_imm16,		&&illegal,
		&&callc_imm16,		&&illegal,			&&sbc_a_imm8,		&&rst_18h,
		&&ldff_imm8_a,		&&pop_hl,			&&ldff_c_a,			&&illegal,
		&&illegal,			&&push_hl,			&&and_a_imm8,		&&rst_20h,
		&&add_sp_imm8,		&&jp_hl,			&&ld_imm16_a,		&&illegal,
		&&illegal,			&&illegal,			&&xor_a_imm8,		&&rst_28h,
		&&ldff_a_imm8,		&&pop_af,			&&ldff_a_c,			&&di,
		&&illegal,			&&push_af,			&&or_a_imm8,		&&rst_30h,
		&&ld_hl_sp_imm8,	&&ld_sp_hl,			&&ld_a_imm16,		&&ei,
		&&illegal,			&&illegal,			&&cp_a_imm8,		&&rst_38h
	};

	uint8_t		*address = GET_REAL_ADDR(regs->reg_pc);

	uint8_t		opcode = *address;

//	dump(address, opcode, regs);

	address = GET_REAL_ADDR(regs->reg_pc + 1);

	uint8_t		imm_8 = *address;
	uint16_t	imm_16 = imm_8;

	address = GET_REAL_ADDR(regs->reg_pc + 2);

	imm_16 |= ((uint16_t)*address) << 8;

	goto *insn_jumps[opcode];

nop:
	ADD_PC(1);
	return (4);

ld_bc_imm16:
	ADD_PC(3);
	regs->reg_bc = imm_16;
	return (12);

ld_bc_a:
	ADD_PC(1);
	WRITE_8(regs->reg_bc, regs->reg_a);
	return (8);

inc_bc:
	ADD_PC(1);
	regs->reg_bc += 1;
	return (8);

inc_b:
	ADD_PC(1);
	INC_REG_8(regs->reg_b);
	return (4);

dec_b:
	ADD_PC(1);
	DEC_REG_8(regs->reg_b);
	return (4);

ld_b_imm8:
	ADD_PC(2);
	regs->reg_b = imm_8;
	return (8);

rlca:
	ADD_PC(1);
	imm_8 = (regs->reg_a & BIT_7) >> 7;
	regs->reg_f = imm_8 << 4;
	regs->reg_a <<= 1;
	regs->reg_a |= imm_8;
	return (4);

ld_addr16_sp:
	ADD_PC(3);
	WRITE_16(imm_16, regs->reg_sp);
	return (20);

add_hl_bc:
	ADD_PC(1);
	ADD_HL_REG_16(regs->reg_bc);
	return (8);

ld_a_bc:
	ADD_PC(1);
	address = GET_REAL_ADDR(regs->reg_bc);
	regs->reg_a = *address;
	return (8);

dec_bc:
	ADD_PC(1);
	regs->reg_bc -= 1;
	return (8);

inc_c:
	ADD_PC(1);
	INC_REG_8(regs->reg_c);
	return (4);

dec_c:
	ADD_PC(1);
	DEC_REG_8(regs->reg_c);
	return (4);

ld_c_imm8:
	ADD_PC(2);
	regs->reg_c = imm_8;
	return (8);

rrca:
	ADD_PC(1);
	imm_8 = (regs->reg_a & BIT_0) << 7;
	regs->reg_f = imm_8 >> 3;
	regs->reg_a >>= 1;
	regs->reg_a |= imm_8;
	return (4);

stop:
	ADD_PC(1);
	GAMEBOY_STATUS = STOP_MODE;
	return (4);

ld_de_imm16:
	ADD_PC(3);
	regs->reg_de = imm_16;
	return (12);

ld_de_a:
	ADD_PC(1);
	WRITE_8(regs->reg_de, regs->reg_a);
	return (8);

inc_de:
	ADD_PC(1);
	regs->reg_de += 1;
	return (8);

inc_d:
	ADD_PC(1);
	INC_REG_8(regs->reg_d);
	return (4);

dec_d:
	ADD_PC(1);
	DEC_REG_8(regs->reg_d);
	return (4);

ld_d_imm8:
	ADD_PC(2);
	regs->reg_d = imm_8;
	return (8);

rla:
	ADD_PC(1);
	imm_8 = (regs->reg_f & FLAG_CY) >> 4;
	regs->reg_f = (regs->reg_a & BIT_7) >> 3;
	regs->reg_a <<= 1;
	regs->reg_a |= imm_8;
	return (4);

jr_imm8:
	ADD_PC(2);
	ADD_PC((int8_t)imm_8);
	return (12);

add_hl_de:
	ADD_PC(1);
	ADD_HL_REG_16(regs->reg_de);
	return (8);

ld_a_de:
	ADD_PC(1);
	address = GET_REAL_ADDR(regs->reg_de);
	regs->reg_a = *address;
	return (8);

dec_de:
	ADD_PC(1);
	regs->reg_de -= 1;
	return (8);

inc_e:
	ADD_PC(1);
	INC_REG_8(regs->reg_e);
	return (4);

dec_e:
	ADD_PC(1);
	DEC_REG_8(regs->reg_e);
	return (4);

ld_e_imm8:
	ADD_PC(2);
	regs->reg_e = imm_8;
	return (8);

rra:
	ADD_PC(1);
	imm_8 = (regs->reg_f & FLAG_CY) << 3;
	regs->reg_f = (regs->reg_a & BIT_0) << 4;
	regs->reg_a >>= 1;
	regs->reg_a |= imm_8;
	return (4);

jrnz_imm8:
	ADD_PC(2);
	if ((regs->reg_f & FLAG_Z) == 0)
	{
		ADD_PC((int8_t)imm_8);
		return (12);
	}
	return (8);

ld_hl_imm16:
	ADD_PC(3);
	regs->reg_hl = imm_16;
	return (12);

ld_hli_a:
	ADD_PC(1);
	WRITE_8(regs->reg_hl, regs->reg_a);
	regs->reg_hl += 1;
	return (8);

inc_hl:
	ADD_PC(1);
	regs->reg_hl += 1;
	return (8);

inc_h:
	ADD_PC(1);
	INC_REG_8(regs->reg_h);
	return (4);

dec_h:
	ADD_PC(1);
	DEC_REG_8(regs->reg_h);
	return (4);

ld_h_imm8:
	ADD_PC(2);
	regs->reg_h = imm_8;
	return (8);

#define old_a	opcode
#define old_f	imm_8

daa:
	ADD_PC(1);
	old_a = regs->reg_a;
	old_f = regs->reg_f;
	regs->reg_f &= FLAG_N;
	if (old_f & FLAG_N)
	{
		if ((old_a & 0xFu) > 0x9 || (old_f & FLAG_H))
		{
			regs->reg_a -= 0x6u;
			if (regs->reg_a >= 0xFAu)
				regs->reg_f |= FLAG_CY;
		}
		if ((old_a > 0x99u) || (old_f & FLAG_CY))
		{
			regs->reg_a -= 0x60u;
			regs->reg_f |= FLAG_CY;
		}
	}
	else
	{
		if ((old_a & 0xFu) > 0x9u || (old_f & FLAG_H))
		{
			regs->reg_a += 0x6u;
			if (regs->reg_a < 0x6u)
				regs->reg_f |= FLAG_CY;
		}
		if ((old_a > 0x99u) || (old_f & FLAG_CY))
		{
			regs->reg_a += 0x60u;
			regs->reg_f |= FLAG_CY;
		}
	}
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	return (4);

#undef old_a
#undef old_f

jrz_imm8:
	ADD_PC(2);
	if ((regs->reg_f & FLAG_Z) == FLAG_Z)
	{
		ADD_PC((int8_t)imm_8);
		return (12);
	}
	return (8);

add_hl_hl:
	ADD_PC(1);
	ADD_HL_REG_16(regs->reg_hl);
	return (8);

ld_a_hli:
	ADD_PC(1);
	address = GET_REAL_ADDR(regs->reg_hl);
	regs->reg_hl += 1;
	regs->reg_a = *address;
	return (8);

dec_hl:
	ADD_PC(1);
	regs->reg_hl -= 1;
	return (8);

inc_l:
	ADD_PC(1);
	INC_REG_8(regs->reg_l);
	return (4);

dec_l:
	ADD_PC(1);
	DEC_REG_8(regs->reg_l);
	return (4);

ld_l_imm8:
	ADD_PC(2);
	regs->reg_l = imm_8;
	return (8);

cpl:
	ADD_PC(1);
	regs->reg_f |= FLAG_N | FLAG_H;
	regs->reg_a = ~(regs->reg_a);
	return (4);

jrnc_imm8:
	ADD_PC(2);
	if ((regs->reg_f & FLAG_CY) == 0)
	{
		ADD_PC((int8_t)imm_8);
		return (12);
	}
	return (8);

ld_sp_imm16:
	ADD_PC(3);
	regs->reg_sp = imm_16;
	return (12);

ld_hld_a:
	ADD_PC(1);
	WRITE_8(regs->reg_hl, regs->reg_a);
	regs->reg_hl -= 1;
	return (8);

inc_sp:
	ADD_PC(1);
	regs->reg_sp += 1;
	return (8);

inc_ahl:
	ADD_PC(1);
	address = GET_REAL_ADDR(regs->reg_hl);
	imm_8 = *address;
	INC_REG_8(imm_8);
	WRITE_8(regs->reg_hl, imm_8);
	return (12);

dec_ahl:
	ADD_PC(1);
	address = GET_REAL_ADDR(regs->reg_hl);
	imm_8 = *address;
	DEC_REG_8(imm_8);
	WRITE_8(regs->reg_hl, imm_8);
	return (12);

ld_hl_imm8:
	ADD_PC(2);
	WRITE_8(regs->reg_hl, imm_8);
	return (12);

scf:
	ADD_PC(1);
	regs->reg_f &= FLAG_Z;
	regs->reg_f |= FLAG_CY;
	return (4);

jrc_imm8:
	ADD_PC(2);
	if ((regs->reg_f & FLAG_CY) == FLAG_CY)
	{
		ADD_PC((int8_t)imm_8);
		return (12);
	}
	return (8);

add_hl_sp:
	ADD_PC(1);
	ADD_HL_REG_16(regs->reg_sp);
	return (8);

ld_a_hld:
	ADD_PC(1);
	address = GET_REAL_ADDR(regs->reg_hl);
	regs->reg_hl -= 1;
	regs->reg_a = *address;
	return (8);

dec_sp:
	ADD_PC(1);
	regs->reg_sp -= 1;
	return (8);

inc_a:
	ADD_PC(1);
	INC_REG_8(regs->reg_a);
	return (4);

dec_a:
	ADD_PC(1);
	DEC_REG_8(regs->reg_a);
	return (4);

ld_a_imm8:
	ADD_PC(2);
	regs->reg_a = imm_8;
	return (8);

ccf:
	ADD_PC(1);
	regs->reg_f ^= FLAG_CY;
	regs->reg_f &= (FLAG_Z | FLAG_CY);
	return (4);

LD_BLOCK(b, c);
LD_BLOCK(b, d);
LD_BLOCK(b, e);
LD_BLOCK(b, h);
LD_BLOCK(b, l);
LD_BLOCK_HL(b);
LD_BLOCK(b, a);
LD_BLOCK(c, b);
LD_BLOCK(c, d);
LD_BLOCK(c, e);
LD_BLOCK(c, h);
LD_BLOCK(c, l);
LD_BLOCK_HL(c);
LD_BLOCK(c, a);
LD_BLOCK(d, b);
LD_BLOCK(d, c);
LD_BLOCK(d, e);
LD_BLOCK(d, h);
LD_BLOCK(d, l);
LD_BLOCK_HL(d);
LD_BLOCK(d, a);
LD_BLOCK(e, b);
LD_BLOCK(e, c);
LD_BLOCK(e, d);
LD_BLOCK(e, h);
LD_BLOCK(e, l);
LD_BLOCK_HL(e);
LD_BLOCK(e, a);
LD_BLOCK(h, b);
LD_BLOCK(h, c);
LD_BLOCK(h, d);
LD_BLOCK(h, e);
LD_BLOCK(h, l);
LD_BLOCK_HL(h);
LD_BLOCK(h, a);
LD_BLOCK(l, b);
LD_BLOCK(l, c);
LD_BLOCK(l, d);
LD_BLOCK(l, e);
LD_BLOCK(l, h);
LD_BLOCK_HL(l);
LD_BLOCK(l, a);
LD_HL_BLOCK(b);
LD_HL_BLOCK(c);
LD_HL_BLOCK(d);
LD_HL_BLOCK(e);
LD_HL_BLOCK(h);
LD_HL_BLOCK(l);

halt:
	ADD_PC(1);
	GAMEBOY_STATUS = HALT_MODE;
	return (4);

LD_HL_BLOCK(a);
LD_BLOCK(a, b);
LD_BLOCK(a, c);
LD_BLOCK(a, d);
LD_BLOCK(a, e);
LD_BLOCK(a, h);
LD_BLOCK(a, l);
LD_BLOCK_HL(a);

ADD_A_BLOCK(b);
ADD_A_BLOCK(c);
ADD_A_BLOCK(d);
ADD_A_BLOCK(e);
ADD_A_BLOCK(h);
ADD_A_BLOCK(l);

add_a_hl:
	ADD_PC(1);
	regs->reg_f = 0;
	address = GET_REAL_ADDR(regs->reg_hl);
	imm_8 = regs->reg_a + *address;
	if (imm_8 < regs->reg_a)
		regs->reg_f |= FLAG_CY;
	if ((imm_8 & 0xFu) < (regs->reg_a & 0xFu))
		regs->reg_f |= FLAG_H;
	if (imm_8 == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a = imm_8;
	return (8);

ADD_A_BLOCK(a);

ADC_A_BLOCK(b);
ADC_A_BLOCK(c);
ADC_A_BLOCK(d);
ADC_A_BLOCK(e);
ADC_A_BLOCK(h);
ADC_A_BLOCK(l);

adc_a_hl:
	ADD_PC(1);
	address = GET_REAL_ADDR(regs->reg_hl);
	imm_8 = regs->reg_a + *address;
	opcode = (regs->reg_f & FLAG_CY) == FLAG_CY;
	regs->reg_f = 0;
	imm_8 += (opcode) ? 1 : 0;
	if (imm_8 < regs->reg_a || (imm_8 == regs->reg_a && opcode))
		regs->reg_f |= FLAG_CY;
	if ((imm_8 & 0xFu) < (regs->reg_a & 0xFu) ||
		((imm_8 & 0xFu) == (regs->reg_a & 0xFu) && opcode))
		regs->reg_f |= FLAG_H;
	if (imm_8 == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a = imm_8;
	return (8);

ADC_A_BLOCK(a);

SUB_A_BLOCK(b);
SUB_A_BLOCK(c);
SUB_A_BLOCK(d);
SUB_A_BLOCK(e);
SUB_A_BLOCK(h);
SUB_A_BLOCK(l);

sub_a_hl:
	ADD_PC(1);
	regs->reg_f = FLAG_N;
	address = GET_REAL_ADDR(regs->reg_hl);
	imm_8 = regs->reg_a - *address;
	if (imm_8 > regs->reg_a)
		regs->reg_f |= FLAG_CY;
	if ((imm_8 & 0xFu) > (regs->reg_a & 0xFu))
		regs->reg_f |= FLAG_H;
	if (imm_8 == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a = imm_8;
	return (8);

SUB_A_BLOCK(a);

SBC_A_BLOCK(b);
SBC_A_BLOCK(c);
SBC_A_BLOCK(d);
SBC_A_BLOCK(e);
SBC_A_BLOCK(h);
SBC_A_BLOCK(l);

sbc_a_hl:
	ADD_PC(1);
	address = GET_REAL_ADDR(regs->reg_hl);
	imm_8 = regs->reg_a - *address;
	opcode = (regs->reg_f & FLAG_CY) == FLAG_CY;
	regs->reg_f = FLAG_N;
	imm_8 -= (opcode) ? 1 : 0;
	if (imm_8 > regs->reg_a || (imm_8 == regs->reg_a && opcode))
		regs->reg_f |= FLAG_CY;
	if ((imm_8 & 0xFu) > (regs->reg_a & 0xFu) ||
		((imm_8 & 0xFu) == (regs->reg_a & 0xFu) && opcode))
		regs->reg_f |= FLAG_H;
	if (imm_8 == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a = imm_8;
	return (8);

SBC_A_BLOCK(a);

AND_A_BLOCK(b);
AND_A_BLOCK(c);
AND_A_BLOCK(d);
AND_A_BLOCK(e);
AND_A_BLOCK(h);
AND_A_BLOCK(l);

and_a_hl:
	ADD_PC(1);
	regs->reg_f = FLAG_H;
	address = GET_REAL_ADDR(regs->reg_hl);
	regs->reg_a &= *address;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

AND_A_BLOCK(a);

XOR_A_BLOCK(b);
XOR_A_BLOCK(c);
XOR_A_BLOCK(d);
XOR_A_BLOCK(e);
XOR_A_BLOCK(h);
XOR_A_BLOCK(l);

xor_a_hl:
	ADD_PC(1);
	regs->reg_f = 0;
	address = GET_REAL_ADDR(regs->reg_hl);
	regs->reg_a ^= *address;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

XOR_A_BLOCK(a);

OR_A_BLOCK(b);
OR_A_BLOCK(c);
OR_A_BLOCK(d);
OR_A_BLOCK(e);
OR_A_BLOCK(h);
OR_A_BLOCK(l);

or_a_hl:
	ADD_PC(1);
	regs->reg_f = 0;
	address = GET_REAL_ADDR(regs->reg_hl);
	regs->reg_a |= *address;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

OR_A_BLOCK(a);

CP_A_BLOCK(b);
CP_A_BLOCK(c);
CP_A_BLOCK(d);
CP_A_BLOCK(e);
CP_A_BLOCK(h);
CP_A_BLOCK(l);

cp_a_hl:
	ADD_PC(1);
	regs->reg_f = FLAG_N;
	address = GET_REAL_ADDR(regs->reg_hl);
	if (regs->reg_a == *address)
		regs->reg_f |= FLAG_Z;
	if (regs->reg_a < *address)
		regs->reg_f |= FLAG_CY;
	if ((regs->reg_a & 0xFu) < (*address & 0xFu))
		regs->reg_f |= FLAG_H;
	return (8);

CP_A_BLOCK(a);

retnz:
	RET_IF((regs->reg_f & FLAG_Z) == 0);

pop_bc:
	ADD_PC(1);
	POP_REG_16(regs->reg_bc);
	return (12);

jpnz_imm16:
	JP_IF((regs->reg_f & FLAG_Z) == 0);

jp_imm16:
	SET_PC(imm_16);
	return (16);

callnz_imm16:
	CALL_IF((regs->reg_f & FLAG_Z) == 0);

push_bc:
	ADD_PC(1);
	PUSH_REG_16(regs->reg_bc);
	return (16);

add_a_imm8:
	ADD_PC(2);
	regs->reg_f = 0;
	imm_8 += regs->reg_a;
	if (imm_8 < regs->reg_a)
		regs->reg_f |= FLAG_CY;
	if ((imm_8 & 0xFu) < (regs->reg_a & 0xFu))
		regs->reg_f |= FLAG_H;
	if (imm_8 == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a = imm_8;
	return (8);

rst_00h:
	ADD_PC(1);
	CALL(0x00);
	return (16);

retz:
	RET_IF((regs->reg_f & FLAG_Z) == FLAG_Z);

ret:
	POP_REG_16(regs->reg_pc);
	return (16);

jpz_imm16:
	JP_IF((regs->reg_f & FLAG_Z) == FLAG_Z);

callz_imm16:
	CALL_IF((regs->reg_f & FLAG_Z) == FLAG_Z);

call_imm16:
	ADD_PC(3);
	PUSH_REG_16(regs->reg_pc);
	SET_PC(imm_16);
	return (24);

adc_a_imm8:
	ADD_PC(2);
	imm_8 += regs->reg_a;
	opcode = (regs->reg_f & FLAG_CY) == FLAG_CY;
	regs->reg_f = 0;
	imm_8 += (opcode) ? 1 : 0;
	if (imm_8 < regs->reg_a || (imm_8 == regs->reg_a && opcode))
		regs->reg_f |= FLAG_CY;
	if ((imm_8 & 0xFu) < (regs->reg_a & 0xFu) ||
		((imm_8 & 0xFu) == (regs->reg_a & 0xFu) && opcode))
		regs->reg_f |= FLAG_H;
	if (imm_8 == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a = imm_8;
	return (8);


rst_08h:
	ADD_PC(1);
	CALL(0x08);
	return (16);

retnc:
	RET_IF((regs->reg_f & FLAG_CY) == 0);

pop_de:
	ADD_PC(1);
	POP_REG_16(regs->reg_de);
	return (12);

jpnc_imm16:
	JP_IF((regs->reg_f & FLAG_CY) == 0);

callnc_imm16:
	CALL_IF((regs->reg_f & FLAG_CY) == 0);

push_de:
	ADD_PC(1);
	PUSH_REG_16(regs->reg_de);
	return (16);

sub_a_imm8:
	ADD_PC(2);
	regs->reg_f = FLAG_N;
	imm_8 = regs->reg_a - imm_8;
	if (imm_8 > regs->reg_a)
		regs->reg_f |= FLAG_CY;
	if ((imm_8 & 0xFu) > (regs->reg_a & 0xFu))
		regs->reg_f |= FLAG_H;
	if (imm_8 == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a = imm_8;
	return (8);

rst_10h:
	ADD_PC(1);
	CALL(0x10);
	return (16);

retc:
	RET_IF((regs->reg_f & FLAG_CY) == FLAG_CY);

reti:
	ADD_PC(1);
	POP_REG_16(regs->reg_pc);
	g_memmap.ime = true;
	return (16);

jpc_imm16:
	JP_IF((regs->reg_f & FLAG_CY) == FLAG_CY);

callc_imm16:
	CALL_IF((regs->reg_f & FLAG_CY) == FLAG_CY);

sbc_a_imm8:
	ADD_PC(2);
	imm_8 = regs->reg_a - imm_8;
	opcode = (regs->reg_f & FLAG_CY) == FLAG_CY;
	regs->reg_f = FLAG_N;
	imm_8 -= (opcode) ? 1 : 0;
	if (imm_8 > regs->reg_a || (imm_8 == regs->reg_a && opcode))
		regs->reg_f |= FLAG_CY;
	if ((imm_8 & 0xFu) > (regs->reg_a & 0xFu) ||
		((imm_8 & 0xFu) == (regs->reg_a & 0xFu) && opcode))
		regs->reg_f |= FLAG_H;
	if (imm_8 == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a = imm_8;
	return (8);

rst_18h:
	ADD_PC(1);
	CALL(0x18);
	return (16);

ldff_imm8_a:
	ADD_PC(2);
	WRITE_8(0xFF00u + imm_8, regs->reg_a);
	return (12);

pop_hl:
	ADD_PC(1);
	POP_REG_16(regs->reg_hl);
	return (12);

ldff_c_a:
	ADD_PC(1);
	WRITE_8(0xFF00u + regs->reg_c, regs->reg_a);
	return (8);

push_hl:
	ADD_PC(1);
	PUSH_REG_16(regs->reg_hl);
	return (16);

and_a_imm8:
	ADD_PC(2);
	regs->reg_f = FLAG_H;
	regs->reg_a &= imm_8;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

rst_20h:
	ADD_PC(1);
	CALL(0x20);
	return (16);

add_sp_imm8:
	ADD_PC(2);
	regs->reg_f = 0;
	imm_16 = regs->reg_sp + (int8_t)imm_8;
	if ((imm_8 < 0x80u) && (imm_16 & 0xFFu) < (regs->reg_sp & 0xFFu))
		regs->reg_f |= FLAG_CY;
//	else if ((imm_8 >= 0x80u) && (imm_16 & 0xFFu) > (regs->reg_sp & 0xFFu))
//		regs->reg_f |= FLAG_CY;
	if ((imm_8 < 0x80u) && (imm_16 & 0xFu) < (regs->reg_sp & 0xFu))
		regs->reg_f |= FLAG_H;
//	else if ((imm_8 >= 0x80u) && (imm_16 & 0xFu) > (regs->reg_sp & 0xFu))
//		regs->reg_f |= FLAG_H;
	regs->reg_sp = imm_16;
	return (16);

jp_hl:
	SET_PC(regs->reg_hl);
	return (4);

ld_imm16_a:
	ADD_PC(3);
	WRITE_8(imm_16, regs->reg_a);
	return (16);

xor_a_imm8:
	ADD_PC(2);
	regs->reg_f = 0;
	regs->reg_a ^= imm_8;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

rst_28h:
	ADD_PC(1);
	CALL(0x28);
	return (16);

ldff_a_imm8:
	ADD_PC(2);
	address = GET_REAL_ADDR(0xFF00u + imm_8);
	regs->reg_a = *address;
	return (8);

pop_af:
	ADD_PC(1);
	POP_REG_16(regs->reg_af);
	return (12);

ldff_a_c:
	ADD_PC(1);
	address = GET_REAL_ADDR(0xFF00u + regs->reg_c);
	regs->reg_a = *address;
	return (8);

di:
	ADD_PC(1);
	g_memmap.ime = false;
	return (4);

push_af:
	ADD_PC(1);
	PUSH_REG_16(regs->reg_af);
	return (16);

or_a_imm8:
	ADD_PC(2);
	regs->reg_f = 0;
	regs->reg_a |= imm_8;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

rst_30h:
	ADD_PC(1);
	CALL(0x30);
	return (16);

ld_hl_sp_imm8:
	ADD_PC(2);
	regs->reg_f = 0;
	imm_16 = regs->reg_sp + (int8_t)imm_8;
	if ((imm_8 < 0x80u) && (imm_16 & 0xFFu) < (regs->reg_sp & 0xFFu))
		regs->reg_f |= FLAG_CY;
//	else if ((imm_8 >= 0x80u) && (imm_16 & 0xFFu) > (regs->reg_sp & 0xFFu))
//		regs->reg_f |= FLAG_CY;
	if ((imm_8 < 0x80u) && (imm_16 & 0xFu) < (regs->reg_sp & 0xFu))
		regs->reg_f |= FLAG_H;
//	else if ((imm_8 >= 0x80u) && (imm_16 & 0xFu) > (regs->reg_sp & 0xFu))
//		regs->reg_f |= FLAG_H;
	regs->reg_hl = imm_16;
	return (12);

ld_sp_hl:
	ADD_PC(1);
	regs->reg_sp = regs->reg_hl;
	return (8);

ld_a_imm16:
	ADD_PC(3);
	address = GET_REAL_ADDR(imm_16);
	regs->reg_a = *address;
	return (16);

ei:
	ADD_PC(1);
	g_memmap.ime = true;
	return (4);

cp_a_imm8:
	ADD_PC(2);
	regs->reg_f = FLAG_N;
	if (regs->reg_a == imm_8)
		regs->reg_f |= FLAG_Z;
	if (regs->reg_a < imm_8)
		regs->reg_f |= FLAG_CY;
	if ((regs->reg_a & 0xFu) < (imm_8 & 0xFu))
		regs->reg_f |= FLAG_H;
	return (4);

rst_38h:
	ADD_PC(1);
	CALL(0x38);
	return (16);

illegal:
	ADD_PC(1);
	return (4);

prefix_cb:
	ADD_PC(2);

	static const void *const	bitboard_tables[256] = 
	{
		&&rlc_b,			&&rlc_c,			&&rlc_d,			&&rlc_e,
		&&rlc_h,			&&rlc_l,			&&rlc_hl,			&&rlc_a,
		&&rrc_b,			&&rrc_c,			&&rrc_d,			&&rrc_e,
		&&rrc_h,			&&rrc_l,			&&rrc_hl,			&&rrc_a,
		&&rl_b,				&&rl_c,				&&rl_d,				&&rl_e,
		&&rl_h,				&&rl_l,				&&rl_hl,			&&rl_a,
		&&rr_b,				&&rr_c,				&&rr_d,				&&rr_e,
		&&rr_h,				&&rr_l,				&&rr_hl,			&&rr_a,
		&&sla_b,			&&sla_c,			&&sla_d,			&&sla_e,
		&&sla_h,			&&sla_l,			&&sla_hl,			&&sla_a,
		&&sra_b,			&&sra_c,			&&sra_d,			&&sra_e,
		&&sra_h,			&&sra_l,			&&sra_hl,			&&sra_a,
		&&swap_b,			&&swap_c,			&&swap_d,			&&swap_e,
		&&swap_h,			&&swap_l,			&&swap_hl,			&&swap_a,
		&&srl_b,			&&srl_c,			&&srl_d,			&&srl_e,
		&&srl_h,			&&srl_l,			&&srl_hl,			&&srl_a,
		&&bit_0_b,			&&bit_0_c,			&&bit_0_d,			&&bit_0_e,
		&&bit_0_h,			&&bit_0_l,			&&bit_0_hl,			&&bit_0_a,
		&&bit_1_b,			&&bit_1_c,			&&bit_1_d,			&&bit_1_e,
		&&bit_1_h,			&&bit_1_l,			&&bit_1_hl,			&&bit_1_a,
		&&bit_2_b,			&&bit_2_c,			&&bit_2_d,			&&bit_2_e,
		&&bit_2_h,			&&bit_2_l,			&&bit_2_hl,			&&bit_2_a,
		&&bit_3_b,			&&bit_3_c,			&&bit_3_d,			&&bit_3_e,
		&&bit_3_h,			&&bit_3_l,			&&bit_3_hl,			&&bit_3_a,
		&&bit_4_b,			&&bit_4_c,			&&bit_4_d,			&&bit_4_e,
		&&bit_4_h,			&&bit_4_l,			&&bit_4_hl,			&&bit_4_a,
		&&bit_5_b,			&&bit_5_c,			&&bit_5_d,			&&bit_5_e,
		&&bit_5_h,			&&bit_5_l,			&&bit_5_hl,			&&bit_5_a,
		&&bit_6_b,			&&bit_6_c,			&&bit_6_d,			&&bit_6_e,
		&&bit_6_h,			&&bit_6_l,			&&bit_6_hl,			&&bit_6_a,
		&&bit_7_b,			&&bit_7_c,			&&bit_7_d,			&&bit_7_e,
		&&bit_7_h,			&&bit_7_l,			&&bit_7_hl,			&&bit_7_a,
		&&res_0_b,			&&res_0_c,			&&res_0_d,			&&res_0_e,
		&&res_0_h,			&&res_0_l,			&&res_0_hl,			&&res_0_a,
		&&res_1_b,			&&res_1_c,			&&res_1_d,			&&res_1_e,
		&&res_1_h,			&&res_1_l,			&&res_1_hl,			&&res_1_a,
		&&res_2_b,			&&res_2_c,			&&res_2_d,			&&res_2_e,
		&&res_2_h,			&&res_2_l,			&&res_2_hl,			&&res_2_a,
		&&res_3_b,			&&res_3_c,			&&res_3_d,			&&res_3_e,
		&&res_3_h,			&&res_3_l,			&&res_3_hl,			&&res_3_a,
		&&res_4_b,			&&res_4_c,			&&res_4_d,			&&res_4_e,
		&&res_4_h,			&&res_4_l,			&&res_4_hl,			&&res_4_a,
		&&res_5_b,			&&res_5_c,			&&res_5_d,			&&res_5_e,
		&&res_5_h,			&&res_5_l,			&&res_5_hl,			&&res_5_a,
		&&res_6_b,			&&res_6_c,			&&res_6_d,			&&res_6_e,
		&&res_6_h,			&&res_6_l,			&&res_6_hl,			&&res_6_a,
		&&res_7_b,			&&res_7_c,			&&res_7_d,			&&res_7_e,
		&&res_7_h,			&&res_7_l,			&&res_7_hl,			&&res_7_a,
		&&set_0_b,			&&set_0_c,			&&set_0_d,			&&set_0_e,
		&&set_0_h,			&&set_0_l,			&&set_0_hl,			&&set_0_a,
		&&set_1_b,			&&set_1_c,			&&set_1_d,			&&set_1_e,
		&&set_1_h,			&&set_1_l,			&&set_1_hl,			&&set_1_a,
		&&set_2_b,			&&set_2_c,			&&set_2_d,			&&set_2_e,
		&&set_2_h,			&&set_2_l,			&&set_2_hl,			&&set_2_a,
		&&set_3_b,			&&set_3_c,			&&set_3_d,			&&set_3_e,
		&&set_3_h,			&&set_3_l,			&&set_3_hl,			&&set_3_a,
		&&set_4_b,			&&set_4_c,			&&set_4_d,			&&set_4_e,
		&&set_4_h,			&&set_4_l,			&&set_4_hl,			&&set_4_a,
		&&set_5_b,			&&set_5_c,			&&set_5_d,			&&set_5_e,
		&&set_5_h,			&&set_5_l,			&&set_5_hl,			&&set_5_a,
		&&set_6_b,			&&set_6_c,			&&set_6_d,			&&set_6_e,
		&&set_6_h,			&&set_6_l,			&&set_6_hl,			&&set_6_a,
		&&set_7_b,			&&set_7_c,			&&set_7_d,			&&set_7_e,
		&&set_7_h,			&&set_7_l,			&&set_7_hl,			&&set_7_a,
	};

	goto *bitboard_tables[imm_8];

#define RLC_8(value) \
	imm_8 = ((value) & BIT_7) >> 7; \
	regs->reg_f = imm_8 << 4; \
	(value) <<= 1; \
	(value) |= imm_8; \
	if ((value) == 0) \
		regs->reg_f |= FLAG_Z

#define RRC_8(value) \
	imm_8 = ((value) & BIT_0) << 7; \
	regs->reg_f = imm_8 >> 3; \
	(value) >>= 1; \
	(value) |= imm_8; \
	if ((value) == 0) \
		regs->reg_f |= FLAG_Z

#define RL_8(value) \
	imm_8 = (regs->reg_f & FLAG_CY) >> 4; \
	regs->reg_f = ((value) & BIT_7) >> 3; \
	(value) <<= 1; \
	(value) |= imm_8; \
	if ((value) == 0) \
		regs->reg_f |= FLAG_Z

#define RR_8(value) \
	imm_8 = (regs->reg_f & FLAG_CY) << 3; \
	regs->reg_f = ((value) & BIT_0) << 4; \
	(value) >>= 1; \
	(value) |= imm_8; \
	if ((value) == 0) \
		regs->reg_f |= FLAG_Z

#define SLA_8(value) \
	regs->reg_f = (((value) & BIT_7) == BIT_7) ? FLAG_CY : 0; \
	(value) <<= 1; \
	if ((value) == 0) \
		regs->reg_f |= FLAG_Z

#define SRA_8(value) \
	regs->reg_f = (((value) & BIT_0) == BIT_0) ? FLAG_CY : 0; \
	imm_8 = (value) & BIT_7; \
	(value) >>= 1; \
	(value) |= imm_8; \
	if ((value) == 0) \
		regs->reg_f |= FLAG_Z

#define SWAP_8(value) \
	(value) = ((value) >> 4) | ((value) << 4); \
	regs->reg_f = ((value) == 0) ? FLAG_Z : 0

#define SRL_8(value) \
	regs->reg_f = (((value) & BIT_0) == BIT_0) ? FLAG_CY : 0; \
	(value) >>= 1; \
	if ((value) == 0) \
		regs->reg_f |= FLAG_Z

#define BIT_8(value, bit_number) \
	regs->reg_f &= FLAG_CY; \
	regs->reg_f |= FLAG_H; \
	if (((value) & BIT_ ## bit_number) == 0) \
		regs->reg_f |= FLAG_Z

#define RES_8(value, bit_number) \
	(value) &= ~(BIT_ ## bit_number)

#define SET_8(value, bit_number) \
	(value) |= (BIT_ ## bit_number)

rlc_b:
	RLC_8(regs->reg_b);
	return (8);

rlc_c:
	RLC_8(regs->reg_c);
	return (8);

rlc_d:
	RLC_8(regs->reg_d);
	return (8);

rlc_e:
	RLC_8(regs->reg_e);
	return (8);

rlc_h:
	RLC_8(regs->reg_h);
	return (8);

rlc_l:
	RLC_8(regs->reg_l);
	return (8);

rlc_hl:
	address = GET_REAL_ADDR(regs->reg_hl);
	RLC_8(*address);
	return (16);

rlc_a:
	RLC_8(regs->reg_a);
	return (8);

rrc_b:
	RRC_8(regs->reg_b);
	return (8);

rrc_c:
	RRC_8(regs->reg_c);
	return (8);

rrc_d:
	RRC_8(regs->reg_d);
	return (8);

rrc_e:
	RRC_8(regs->reg_e);
	return (8);

rrc_h:
	RRC_8(regs->reg_h);
	return (8);

rrc_l:
	RRC_8(regs->reg_l);
	return (8);

rrc_hl:
	address = GET_REAL_ADDR(regs->reg_hl);
	RRC_8(*address);
	return (16);

rrc_a:
	RLC_8(regs->reg_a);
	return (8);

rl_b:
	RL_8(regs->reg_b);
	return (8);

rl_c:
	RL_8(regs->reg_c);
	return (8);

rl_d:
	RL_8(regs->reg_d);
	return (8);

rl_e:
	RL_8(regs->reg_e);
	return (8);

rl_h:
	RL_8(regs->reg_h);
	return (8);

rl_l:
	RL_8(regs->reg_l);
	return (8);

rl_hl:
	address = GET_REAL_ADDR(regs->reg_hl);
	RL_8(*address);
	return (16);

rl_a:
	RL_8(regs->reg_a);
	return (8);

rr_b:
	RR_8(regs->reg_b);
	return (8);

rr_c:
	RR_8(regs->reg_c);
	return (8);

rr_d:
	RR_8(regs->reg_d);
	return (8);

rr_e:
	RR_8(regs->reg_e);
	return (8);

rr_h:
	RR_8(regs->reg_h);
	return (8);

rr_l:
	RR_8(regs->reg_l);
	return (8);

rr_hl:
	address = GET_REAL_ADDR(regs->reg_hl);
	RR_8(*address);
	return (16);

rr_a:
	RR_8(regs->reg_a);
	return (8);


sla_b:
	SLA_8(regs->reg_b);
	return (8);

sla_c:
	SLA_8(regs->reg_c);
	return (8);

sla_d:
	SLA_8(regs->reg_d);
	return (8);

sla_e:
	SLA_8(regs->reg_e);
	return (8);

sla_h:
	SLA_8(regs->reg_h);
	return (8);

sla_l:
	SLA_8(regs->reg_l);
	return (8);

sla_hl:
	address = GET_REAL_ADDR(regs->reg_hl);
	SLA_8(*address);
	return (16);

sla_a:
	SLA_8(regs->reg_a);
	return (8);

sra_b:
	SRA_8(regs->reg_b);
	return (8);

sra_c:
	SRA_8(regs->reg_c);
	return (8);

sra_d:
	SRA_8(regs->reg_d);
	return (8);

sra_e:
	SRA_8(regs->reg_e);
	return (8);

sra_h:
	SRA_8(regs->reg_h);
	return (8);

sra_l:
	SRA_8(regs->reg_l);
	return (8);

sra_hl:
	address = GET_REAL_ADDR(regs->reg_hl);
	SRA_8(*address);
	return (16);

sra_a:
	SRA_8(regs->reg_a);
	return (8);

swap_b:
	SWAP_8(regs->reg_b);
	return (8);

swap_c:
	SWAP_8(regs->reg_c);
	return (8);

swap_d:
	SWAP_8(regs->reg_d);
	return (8);

swap_e:
	SWAP_8(regs->reg_e);
	return (8);

swap_h:
	SWAP_8(regs->reg_h);
	return (8);

swap_l:
	SWAP_8(regs->reg_l);
	return (8);

swap_hl:
	address = GET_REAL_ADDR(regs->reg_hl);
	SWAP_8(*address);
	return (16);

swap_a:
	SWAP_8(regs->reg_a);
	return (8);

srl_b:
	SRL_8(regs->reg_b);
	return (8);

srl_c:
	SRL_8(regs->reg_c);
	return (8);

srl_d:
	SRL_8(regs->reg_d);
	return (8);

srl_e:
	SRL_8(regs->reg_e);
	return (8);

srl_h:
	SRL_8(regs->reg_h);
	return (8);

srl_l:
	SRL_8(regs->reg_l);
	return (8);

srl_hl:
	address = GET_REAL_ADDR(regs->reg_hl);
	SRL_8(*address);
	return (16);

srl_a:
	SRL_8(regs->reg_a);
	return (8);

bit_0_b:
	BIT_8(regs->reg_b, 0);
	return (8);

bit_0_c:
	BIT_8(regs->reg_c, 0);
	return (8);

bit_0_d:
	BIT_8(regs->reg_d, 0);
	return (8);

bit_0_e:
	BIT_8(regs->reg_e, 0);
	return (8);

bit_0_h:
	BIT_8(regs->reg_h, 0);
	return (8);

bit_0_l:
	BIT_8(regs->reg_l, 0);
	return (8);

bit_0_hl:
	address = GET_REAL_ADDR(regs->reg_hl);
	BIT_8(*address, 0);
	return (16);

bit_0_a:
	BIT_8(regs->reg_a, 0);
	return (8);

bit_1_b:
	BIT_8(regs->reg_b, 1);
	return (8);

bit_1_c:
	BIT_8(regs->reg_c, 1);
	return (8);

bit_1_d:
	BIT_8(regs->reg_d, 1);
	return (8);

bit_1_e:
	BIT_8(regs->reg_e, 1);
	return (8);

bit_1_h:
	BIT_8(regs->reg_h, 1);
	return (8);

bit_1_l:
	BIT_8(regs->reg_l, 1);
	return (8);

bit_1_hl:
	address = GET_REAL_ADDR(regs->reg_hl);
	BIT_8(*address, 1);
	return (16);

bit_1_a:
	BIT_8(regs->reg_a, 1);
	return (8);

bit_2_b:
	BIT_8(regs->reg_b, 2);
	return (8);

bit_2_c:
	BIT_8(regs->reg_c, 2);
	return (8);

bit_2_d:
	BIT_8(regs->reg_d, 2);
	return (8);

bit_2_e:
	BIT_8(regs->reg_e, 2);
	return (8);

bit_2_h:
	BIT_8(regs->reg_h, 2);
	return (8);

bit_2_l:
	BIT_8(regs->reg_l, 2);
	return (8);

bit_2_hl:
	address = GET_REAL_ADDR(regs->reg_hl);
	BIT_8(*address, 2);
	return (16);

bit_2_a:
	BIT_8(regs->reg_a, 2);
	return (8);

bit_3_b:
	BIT_8(regs->reg_b, 3);
	return (8);

bit_3_c:
	BIT_8(regs->reg_c, 3);
	return (8);

bit_3_d:
	BIT_8(regs->reg_d, 3);
	return (8);

bit_3_e:
	BIT_8(regs->reg_e, 3);
	return (8);

bit_3_h:
	BIT_8(regs->reg_h, 3);
	return (8);

bit_3_l:
	BIT_8(regs->reg_l, 3);
	return (8);

bit_3_hl:
	address = GET_REAL_ADDR(regs->reg_hl);
	BIT_8(*address, 3);
	return (16);

bit_3_a:
	BIT_8(regs->reg_a, 3);
	return (8);

bit_4_b:
	BIT_8(regs->reg_b, 4);
	return (8);

bit_4_c:
	BIT_8(regs->reg_c, 4);
	return (8);

bit_4_d:
	BIT_8(regs->reg_d, 4);
	return (8);

bit_4_e:
	BIT_8(regs->reg_e, 4);
	return (8);

bit_4_h:
	BIT_8(regs->reg_h, 4);
	return (8);

bit_4_l:
	BIT_8(regs->reg_l, 4);
	return (8);

bit_4_hl:
	address = GET_REAL_ADDR(regs->reg_hl);
	BIT_8(*address, 4);
	return (16);

bit_4_a:
	BIT_8(regs->reg_a, 4);
	return (8);

bit_5_b:
	BIT_8(regs->reg_b, 5);
	return (8);

bit_5_c:
	BIT_8(regs->reg_c, 5);
	return (8);

bit_5_d:
	BIT_8(regs->reg_d, 5);
	return (8);

bit_5_e:
	BIT_8(regs->reg_e, 5);
	return (8);

bit_5_h:
	BIT_8(regs->reg_h, 5);
	return (8);

bit_5_l:
	BIT_8(regs->reg_l, 5);
	return (8);

bit_5_hl:
	address = GET_REAL_ADDR(regs->reg_hl);
	BIT_8(*address, 5);
	return (16);

bit_5_a:
	BIT_8(regs->reg_a, 5);
	return (8);

bit_6_b:
	BIT_8(regs->reg_b, 6);
	return (8);

bit_6_c:
	BIT_8(regs->reg_c, 6);
	return (8);

bit_6_d:
	BIT_8(regs->reg_d, 6);
	return (8);

bit_6_e:
	BIT_8(regs->reg_e, 6);
	return (8);

bit_6_h:
	BIT_8(regs->reg_h, 6);
	return (8);

bit_6_l:
	BIT_8(regs->reg_l, 6);
	return (8);

bit_6_hl:
	address = GET_REAL_ADDR(regs->reg_hl);
	BIT_8(*address, 6);
	return (16);

bit_6_a:
	BIT_8(regs->reg_a, 6);
	return (8);

bit_7_b:
	BIT_8(regs->reg_b, 7);
	return (8);

bit_7_c:
	BIT_8(regs->reg_c, 7);
	return (8);

bit_7_d:
	BIT_8(regs->reg_d, 7);
	return (8);

bit_7_e:
	BIT_8(regs->reg_e, 7);
	return (8);

bit_7_h:
	BIT_8(regs->reg_h, 7);
	return (8);

bit_7_l:
	BIT_8(regs->reg_l, 7);
	return (8);

bit_7_hl:
	address = GET_REAL_ADDR(regs->reg_hl);
	BIT_8(*address, 7);
	return (16);

bit_7_a:
	BIT_8(regs->reg_a, 7);
	return (8);

res_0_b:
	RES_8(regs->reg_b, 0);
	return (8);

res_0_c:
	RES_8(regs->reg_c, 0);
	return (8);

res_0_d:
	RES_8(regs->reg_d, 0);
	return (8);

res_0_e:
	RES_8(regs->reg_e, 0);
	return (8);

res_0_h:
	RES_8(regs->reg_h, 0);
	return (8);

res_0_l:
	RES_8(regs->reg_l, 0);
	return (8);

res_0_hl:
	address = GET_REAL_ADDR(regs->reg_hl);
	RES_8(*address, 0);
	return (16);

res_0_a:
	RES_8(regs->reg_a, 0);
	return (8);

res_1_b:
	RES_8(regs->reg_b, 1);
	return (8);

res_1_c:
	RES_8(regs->reg_c, 1);
	return (8);

res_1_d:
	RES_8(regs->reg_d, 1);
	return (8);

res_1_e:
	RES_8(regs->reg_e, 1);
	return (8);

res_1_h:
	RES_8(regs->reg_h, 1);
	return (8);

res_1_l:
	RES_8(regs->reg_l, 1);
	return (8);

res_1_hl:
	address = GET_REAL_ADDR(regs->reg_hl);
	RES_8(*address, 1);
	return (16);

res_1_a:
	RES_8(regs->reg_a, 1);
	return (8);

res_2_b:
	RES_8(regs->reg_b, 2);
	return (8);

res_2_c:
	RES_8(regs->reg_c, 2);
	return (8);

res_2_d:
	RES_8(regs->reg_d, 2);
	return (8);

res_2_e:
	RES_8(regs->reg_e, 2);
	return (8);

res_2_h:
	RES_8(regs->reg_h, 2);
	return (8);

res_2_l:
	RES_8(regs->reg_l, 2);
	return (8);

res_2_hl:
	address = GET_REAL_ADDR(regs->reg_hl);
	RES_8(*address, 2);
	return (16);

res_2_a:
	RES_8(regs->reg_a, 2);
	return (8);

res_3_b:
	RES_8(regs->reg_b, 3);
	return (8);

res_3_c:
	RES_8(regs->reg_c, 3);
	return (8);

res_3_d:
	RES_8(regs->reg_d, 3);
	return (8);

res_3_e:
	RES_8(regs->reg_e, 3);
	return (8);

res_3_h:
	RES_8(regs->reg_h, 3);
	return (8);

res_3_l:
	RES_8(regs->reg_l, 3);
	return (8);

res_3_hl:
	address = GET_REAL_ADDR(regs->reg_hl);
	RES_8(*address, 3);
	return (16);

res_3_a:
	RES_8(regs->reg_a, 3);
	return (8);

res_4_b:
	RES_8(regs->reg_b, 4);
	return (8);

res_4_c:
	RES_8(regs->reg_c, 4);
	return (8);

res_4_d:
	RES_8(regs->reg_d, 4);
	return (8);

res_4_e:
	RES_8(regs->reg_e, 4);
	return (8);

res_4_h:
	RES_8(regs->reg_h, 4);
	return (8);

res_4_l:
	RES_8(regs->reg_l, 4);
	return (8);

res_4_hl:
	address = GET_REAL_ADDR(regs->reg_hl);
	RES_8(*address, 4);
	return (16);

res_4_a:
	RES_8(regs->reg_a, 4);
	return (8);

res_5_b:
	RES_8(regs->reg_b, 5);
	return (8);

res_5_c:
	RES_8(regs->reg_c, 5);
	return (8);

res_5_d:
	RES_8(regs->reg_d, 5);
	return (8);

res_5_e:
	RES_8(regs->reg_e, 5);
	return (8);

res_5_h:
	RES_8(regs->reg_h, 5);
	return (8);

res_5_l:
	RES_8(regs->reg_l, 5);
	return (8);

res_5_hl:
	address = GET_REAL_ADDR(regs->reg_hl);
	RES_8(*address, 5);
	return (16);

res_5_a:
	RES_8(regs->reg_a, 5);
	return (8);

res_6_b:
	RES_8(regs->reg_b, 6);
	return (8);

res_6_c:
	RES_8(regs->reg_c, 6);
	return (8);

res_6_d:
	RES_8(regs->reg_d, 6);
	return (8);

res_6_e:
	RES_8(regs->reg_e, 6);
	return (8);

res_6_h:
	RES_8(regs->reg_h, 6);
	return (8);

res_6_l:
	RES_8(regs->reg_l, 6);
	return (8);

res_6_hl:
	address = GET_REAL_ADDR(regs->reg_hl);
	RES_8(*address, 6);
	return (16);

res_6_a:
	RES_8(regs->reg_a, 6);
	return (8);

res_7_b:
	RES_8(regs->reg_b, 7);
	return (8);

res_7_c:
	RES_8(regs->reg_c, 7);
	return (8);

res_7_d:
	RES_8(regs->reg_d, 7);
	return (8);

res_7_e:
	RES_8(regs->reg_e, 7);
	return (8);

res_7_h:
	RES_8(regs->reg_h, 7);
	return (8);

res_7_l:
	RES_8(regs->reg_l, 7);
	return (8);

res_7_hl:
	address = GET_REAL_ADDR(regs->reg_hl);
	RES_8(*address, 7);
	return (16);

res_7_a:
	RES_8(regs->reg_a, 7);
	return (8);

set_0_b:
	SET_8(regs->reg_b, 0);
	return (8);

set_0_c:
	SET_8(regs->reg_c, 0);
	return (8);

set_0_d:
	SET_8(regs->reg_d, 0);
	return (8);

set_0_e:
	SET_8(regs->reg_e, 0);
	return (8);

set_0_h:
	SET_8(regs->reg_h, 0);
	return (8);

set_0_l:
	SET_8(regs->reg_l, 0);
	return (8);

set_0_hl:
	address = GET_REAL_ADDR(regs->reg_hl);
	SET_8(*address, 0);
	return (16);

set_0_a:
	SET_8(regs->reg_a, 0);
	return (8);

set_1_b:
	SET_8(regs->reg_b, 1);
	return (8);

set_1_c:
	SET_8(regs->reg_c, 1);
	return (8);

set_1_d:
	SET_8(regs->reg_d, 1);
	return (8);

set_1_e:
	SET_8(regs->reg_e, 1);
	return (8);

set_1_h:
	SET_8(regs->reg_h, 1);
	return (8);

set_1_l:
	SET_8(regs->reg_l, 1);
	return (8);

set_1_hl:
	address = GET_REAL_ADDR(regs->reg_hl);
	SET_8(*address, 1);
	return (16);

set_1_a:
	SET_8(regs->reg_a, 1);
	return (8);

set_2_b:
	SET_8(regs->reg_b, 2);
	return (8);

set_2_c:
	SET_8(regs->reg_c, 2);
	return (8);

set_2_d:
	SET_8(regs->reg_d, 2);
	return (8);

set_2_e:
	SET_8(regs->reg_e, 2);
	return (8);

set_2_h:
	SET_8(regs->reg_h, 2);
	return (8);

set_2_l:
	SET_8(regs->reg_l, 2);
	return (8);

set_2_hl:
	address = GET_REAL_ADDR(regs->reg_hl);
	SET_8(*address, 2);
	return (16);

set_2_a:
	SET_8(regs->reg_a, 2);
	return (8);

set_3_b:
	SET_8(regs->reg_b, 3);
	return (8);

set_3_c:
	SET_8(regs->reg_c, 3);
	return (8);

set_3_d:
	SET_8(regs->reg_d, 3);
	return (8);

set_3_e:
	SET_8(regs->reg_e, 3);
	return (8);

set_3_h:
	SET_8(regs->reg_h, 3);
	return (8);

set_3_l:
	SET_8(regs->reg_l, 3);
	return (8);

set_3_hl:
	address = GET_REAL_ADDR(regs->reg_hl);
	SET_8(*address, 3);
	return (16);

set_3_a:
	SET_8(regs->reg_a, 3);
	return (8);

set_4_b:
	SET_8(regs->reg_b, 4);
	return (8);

set_4_c:
	SET_8(regs->reg_c, 4);
	return (8);

set_4_d:
	SET_8(regs->reg_d, 4);
	return (8);

set_4_e:
	SET_8(regs->reg_e, 4);
	return (8);

set_4_h:
	SET_8(regs->reg_h, 4);
	return (8);

set_4_l:
	SET_8(regs->reg_l, 4);
	return (8);

set_4_hl:
	address = GET_REAL_ADDR(regs->reg_hl);
	SET_8(*address, 4);
	return (16);

set_4_a:
	SET_8(regs->reg_a, 4);
	return (8);

set_5_b:
	SET_8(regs->reg_b, 5);
	return (8);

set_5_c:
	SET_8(regs->reg_c, 5);
	return (8);

set_5_d:
	SET_8(regs->reg_d, 5);
	return (8);

set_5_e:
	SET_8(regs->reg_e, 5);
	return (8);

set_5_h:
	SET_8(regs->reg_h, 5);
	return (8);

set_5_l:
	SET_8(regs->reg_l, 5);
	return (8);

set_5_hl:
	address = GET_REAL_ADDR(regs->reg_hl);
	SET_8(*address, 5);
	return (16);

set_5_a:
	SET_8(regs->reg_a, 5);
	return (8);

set_6_b:
	SET_8(regs->reg_b, 6);
	return (8);

set_6_c:
	SET_8(regs->reg_c, 6);
	return (8);

set_6_d:
	SET_8(regs->reg_d, 6);
	return (8);

set_6_e:
	SET_8(regs->reg_e, 6);
	return (8);

set_6_h:
	SET_8(regs->reg_h, 6);
	return (8);

set_6_l:
	SET_8(regs->reg_l, 6);
	return (8);

set_6_hl:
	address = GET_REAL_ADDR(regs->reg_hl);
	SET_8(*address, 6);
	return (16);

set_6_a:
	SET_8(regs->reg_a, 6);
	return (8);

set_7_b:
	SET_8(regs->reg_b, 7);
	return (8);

set_7_c:
	SET_8(regs->reg_c, 7);
	return (8);

set_7_d:
	SET_8(regs->reg_d, 7);
	return (8);

set_7_e:
	SET_8(regs->reg_e, 7);
	return (8);

set_7_h:
	SET_8(regs->reg_h, 7);
	return (8);

set_7_l:
	SET_8(regs->reg_l, 7);
	return (8);

set_7_hl:
	address = GET_REAL_ADDR(regs->reg_hl);
	SET_8(*address, 7);
	return (16);

set_7_a:
	SET_8(regs->reg_a, 7);
	return (8);
}
