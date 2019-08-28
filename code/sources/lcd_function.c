/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   lcd_function.c                                   .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: mhouppin <mhouppin@le-101.fr>              +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/05 07:38:00 by mhouppin     #+#   ##    ##    #+#       */
/*   Updated: 2019/08/06 14:31:08 by mhouppin    ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "bitmask.h"
#include "timing.h"
#include "lcd_driver.h"
#include "memory_map.h"
#include "cartridge.h"

void	load_oam(oam_t *oam)
{
	if ((LCDC_REGISTER & BIT_1) != BIT_1)
	{
		oam->active = false;
		return ;
	}
	oam->active = true;
	for (size_t i = 0; i < 40; i++)
	{
		oam->obj[i].lcd_y =		g_memmap.complete_block[0xFE00u + i * 4] - 16;
		oam->obj[i].lcd_x =		g_memmap.complete_block[0xFE01u + i * 4] - 8;
		oam->obj[i].code =		g_memmap.complete_block[0xFE02u + i * 4];
		oam->obj[i].attrib =	g_memmap.complete_block[0xFE03u + i * 4];
		if ((oam->obj[i].attrib & BIT_7) == BIT_7 && g_cart.cgb_mode)
			oam->obj[i].prior = OBJ_BG_PRIOR;
		else
			oam->obj[i].prior = OBJ_OBJ_PRIOR;
		oam->obj[i].type =		OBJ_TILE;
		if (!g_cart.cgb_mode)
			oam->obj[i].prior += (255 - oam->obj[i].lcd_x);
		oam->obj[i].next_prior = (255 - oam->obj[i].code);
	}
}

void	lcd_function(int line, int type)
{
	if (type == OAM_READ)
	{
		load_oam(&(g_timing.oam));
		LY_REGISTER = line;
		if (LYC_REGISTER == LY_REGISTER)
		{
			STAT_REGISTER |= BIT_2;
			if ((STAT_REGISTER & BIT_6) == BIT_6 && (IE_REGISTER & BIT_1) == BIT_1)
				IF_REGISTER |= BIT_1;
		}
		else
			STAT_REGISTER &= ~(BIT_2);
	}
	else
		draw_line(&(g_timing.oam), line);
}
