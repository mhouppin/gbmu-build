/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   lcd_driver.c                                     .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: mhouppin <mhouppin@le-101.fr>              +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/05 05:27:34 by mhouppin     #+#   ##    ##    #+#       */
/*   Updated: 2019/07/05 07:34:26 by mhouppin    ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "lcd_driver.h"
#include "timing.h"
#include "memory_map.h"
#include "bitmask.h"

void	update_lcd(cycle_count_t cycles)
{
	if ((LCDC_REGISTER & BIT_7) == 0 || GAMEBOY_STATUS == STOP_MODE)
		return ;
	g_timing.lcd_cycles += cycles;
	if (g_timing.line_render < 144)
	{
		if (g_timing.render_status == OAM_READ &&
			g_timing.lcd_cycles >= OAM_READ_CYCLES)
		{
			g_timing.render_status = OAM_VRAM_READ;
			lcd_function(g_timing.line_render, OAM_VRAM_READ);
			g_timing.lcd_cycles -= OAM_READ_CYCLES;
			STAT_REGISTER |= (BIT_0 | BIT_1);
		}
		if (g_timing.render_status == OAM_VRAM_READ &&
			g_timing.lcd_cycles >= OAM_VRAM_READ_CYCLES)
		{
			g_timing.render_status = HZ_BLANK;
			g_timing.lcd_cycles -= OAM_VRAM_READ_CYCLES;
			if ((STAT_REGISTER & BIT_3) == BIT_3 && (IE_REGISTER & BIT_1) == BIT_1)
				IF_REGISTER |= BIT_1;
			STAT_REGISTER &= ~(BIT_0 | BIT_1);
		}
		if (g_timing.render_status == HZ_BLANK &&
			g_timing.lcd_cycles >= HZ_BLANK_CYCLES)
		{
			g_timing.line_render++;
			STAT_REGISTER &= ~(BIT_0);
			STAT_REGISTER |= (BIT_1);
			g_timing.render_status = OAM_READ;
			lcd_function(g_timing.line_render, OAM_READ);
			g_timing.lcd_cycles -= HZ_BLANK_CYCLES;
			if (g_timing.line_render == 144)
			{
				if ((STAT_REGISTER & BIT_4) == BIT_4 &&
					(IE_REGISTER & BIT_1) == BIT_1)
				{
					IF_REGISTER |= BIT_1;
				}
				if ((IE_REGISTER & BIT_0) == BIT_0)
					IF_REGISTER |= BIT_0;
				STAT_REGISTER &= ~(BIT_1);
				STAT_REGISTER |= (BIT_0);
				update_display();
			}
			else if ((STAT_REGISTER & BIT_5) == BIT_5 &&
					(IE_REGISTER & BIT_1) == BIT_1)
			{
				IF_REGISTER |= BIT_1;
			}
		}
	}
	if (g_timing.line_render >= 144 && g_timing.line_render < 153 &&
		g_timing.lcd_cycles >= HZ_BLANK_CYCLES)
	{
		g_timing.line_render++;
		LY_REGISTER = g_timing.line_render;
		g_timing.lcd_cycles -= HZ_BLANK_CYCLES;
		if (LYC_REGISTER == LY_REGISTER)
		{
			STAT_REGISTER |= BIT_2;
			if ((STAT_REGISTER & BIT_6) == BIT_6 && (IE_REGISTER & BIT_1) == BIT_1)
				IF_REGISTER |= BIT_1;
		}
		else
			STAT_REGISTER &= ~(BIT_2);
	}
	if (g_timing.line_render == 153 && g_timing.lcd_cycles >= VT_BLANK_WAITING_CYCLES)
	{
		g_timing.line_render = 0;
		g_timing.render_status = OAM_READ;
		lcd_function(0, OAM_READ);
		g_timing.lcd_cycles -= VT_BLANK_WAITING_CYCLES;
		if ((STAT_REGISTER & BIT_5) == BIT_5 && (IE_REGISTER & BIT_1) == BIT_1)
			IF_REGISTER |= BIT_1;
		STAT_REGISTER &= ~(BIT_0);
		STAT_REGISTER |= (BIT_1);
	}
}
