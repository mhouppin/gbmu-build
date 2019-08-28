/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   gameboy_core.c                                   .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: mhouppin <mhouppin@le-101.fr>              +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/05 01:48:34 by mhouppin     #+#   ##    ##    #+#       */
/*   Updated: 2019/07/09 10:12:21 by mhouppin    ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "check_events.h"
#include "cpu_specs.h"
#include "lcd_driver.h"
#include "write.h"
#include "bitmask.h"
#include "registers.h"
#include "processor.h"
#include "memory_map.h"
#include "cartridge.h"
#include <stdlib.h>

cycle_count_t	execute_once(registers_t *regs)
{
	cycle_count_t	cycles;
	cycle_count_t	rcycles;

	regs->wbytes = 0;
	if (GAMEBOY_STATUS == NORMAL_MODE)
		cycles = execute(regs);
	else
		cycles = 4;

	rcycles = cycles;
	if (KEY1_REGISTER & BIT_7)
		rcycles /= 2;

	if (regs->wbytes)
		write_bytes(regs);

	update_lcd(rcycles);
	if (GAMEBOY_STATUS != STOP_MODE)
		update_timer_values(cycles);
	check_cntrl_events(cycles);
	check_interrupts(regs);
	return (rcycles);
}
