/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   cpu_specs.c                                      .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: mhouppin <mhouppin@le-101.fr>              +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/05 04:58:27 by mhouppin     #+#   ##    ##    #+#       */
/*   Updated: 2019/08/13 11:41:06 by mhouppin    ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "bitmask.h"
#include "cpu_specs.h"
#include "timing.h"
#include "memory_map.h"
#include "write.h"

void	update_timer_values(cycle_count_t cycles)
{
	uint8_t	shift;

	g_timing.div_cycles += cycles;
	if (g_timing.div_cycles >= 256)
	{
		g_timing.div_cycles -= 256;
		DIV_REGISTER += 1;
	}
	if ((TAC_REGISTER & BIT_2) == BIT_2)
	{
		shift = TAC_REGISTER & (BIT_0 | BIT_1);
		if (shift == 0)
			g_timing.timer_cycles += cycles * 1024;
		else
			g_timing.timer_cycles += cycles * (1024 << (4 - shift));
		if (g_timing.timer_cycles > 4194304)
		{
			TIMA_REGISTER += 1;
			if (TIMA_REGISTER == 0)
			{
				TIMA_REGISTER = TMA_REGISTER;
				if ((IE_REGISTER & BIT_2) == BIT_2)
					IF_REGISTER |= BIT_2;
			}
		}
	}
}

void	auto_sri(registers_t *regs, uint16_t new_pc, uint8_t if_mask)
{
	GAMEBOY_STATUS = NORMAL_MODE;
	g_memmap.ime = false;
	IF_REGISTER &= ~(if_mask);
	regs->reg_sp -= 2;
	regs->wbytes = 2;
	regs->waddr_1 = regs->reg_sp;
	regs->wval_1 = (uint8_t)(regs->reg_pc);
	regs->waddr_2 = regs->reg_sp + 1;
	regs->wval_2 = (uint8_t)((regs->reg_pc) >> 8);
	regs->reg_pc = new_pc;
	write_bytes(regs);
}

void	check_interrupts(registers_t *regs)
{
	if (g_memmap.ime == false && GAMEBOY_STATUS == NORMAL_MODE)
		return ;
	if ((IF_REGISTER & BIT_0) == BIT_0 && GAMEBOY_STATUS != STOP_MODE)
		auto_sri(regs, 0x40, BIT_0);
	else if ((IF_REGISTER & BIT_1) == BIT_1 && GAMEBOY_STATUS != STOP_MODE)
		auto_sri(regs, 0x48, BIT_1);
	else if ((IF_REGISTER & BIT_2) == BIT_2 && GAMEBOY_STATUS != STOP_MODE)
		auto_sri(regs, 0x50, BIT_2);
	else if ((IF_REGISTER & BIT_4) == BIT_4)
		auto_sri(regs, 0x60, BIT_4);
}
