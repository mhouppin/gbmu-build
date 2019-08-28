/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   draw_line.c                                      .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: mhouppin <mhouppin@le-101.fr>              +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/05 07:56:44 by mhouppin     #+#   ##    ##    #+#       */
/*   Updated: 2019/08/06 14:32:22 by mhouppin    ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "bitmask.h"
#include "lcd_driver.h"
#include "memory_map.h"
#include "graphics.h"
#include "cartridge.h"

int		cmp_prior(const void *l, const void *r)
{
	const object_t	*left = l;
	const object_t	*right = r;

	if (left->prior - right->prior != 0)
		return (left->prior - right->prior);
	else if (left->next_prior - right->next_prior != 0)
		return (left->next_prior - right->next_prior);
	else
		return (left->type - right->type);
}

void	draw_dmg_line(object_t *obj, uint8_t size, uint8_t line)
{
	int32_t	bgp[4];
	int32_t	obp[8];

	uint8_t	*vram = g_memmap.vram;

	bgp[0] = (int32_t)(BGP_REGISTER & (BIT_0 | BIT_1));
	bgp[1] = (int32_t)(BGP_REGISTER & (BIT_2 | BIT_3)) >> 2;
	bgp[2] = (int32_t)(BGP_REGISTER & (BIT_4 | BIT_5)) >> 4;
	bgp[3] = (int32_t)(BGP_REGISTER & (BIT_6 | BIT_7)) >> 6;
	obp[0] = (int32_t)(OBP0_REGISTER & (BIT_0 | BIT_1));
	obp[1] = (int32_t)(OBP0_REGISTER & (BIT_2 | BIT_3)) >> 2;
	obp[2] = (int32_t)(OBP0_REGISTER & (BIT_4 | BIT_5)) >> 4;
	obp[3] = (int32_t)(OBP0_REGISTER & (BIT_6 | BIT_7)) >> 6;
	obp[4] = (int32_t)(OBP1_REGISTER & (BIT_0 | BIT_1));
	obp[5] = (int32_t)(OBP1_REGISTER & (BIT_2 | BIT_3)) >> 2;
	obp[6] = (int32_t)(OBP1_REGISTER & (BIT_4 | BIT_5)) >> 4;
	obp[7] = (int32_t)(OBP1_REGISTER & (BIT_6 | BIT_7)) >> 6;

	for (size_t i = 0; i < 4; i++)
	{
		bgp[i] = 0xffffff - (bgp[i] * 0x555555);
	}
	for (size_t i = 0; i < 8; i++)
	{
		obp[i] = 0xffffff - (obp[i] * 0x555555);
	}

	for (uint8_t i = 0; i < size; i++)
	{
		if (obj[i].type == OBJ_TILE)
		{
			uint8_t obj_ypos = obj[i].lcd_y;
			uint8_t	obj_xpos = obj[i].lcd_x;
			uint8_t	tile_size = (LCDC_REGISTER & BIT_2) ? 15 : 7;

			obj_ypos = line - obj_ypos;

			if (obj_ypos > tile_size)
				continue;

			uint8_t attrib = obj[i].attrib;

			if ((attrib & BIT_6) == BIT_6)
				obj_ypos = tile_size - obj_ypos;
			for (uint8_t x = 0; x < 8; x++)
			{
				uint8_t rx = obj_xpos;

				if ((attrib & BIT_5) == BIT_5)
					rx += x;
				else
					rx += 7 - x;

				if (rx < 160)
				{

					uint8_t	dot1 = (vram[(uint16_t)obj[i].code * 16 + (uint16_t)obj_ypos * 2] >> x) & 1;
					uint8_t	dot2 = (vram[(uint16_t)obj[i].code * 16 + (uint16_t)obj_ypos * 2 + 1] >> x) & 1;
					int32_t	pxl;
					if (dot1 + dot2 + dot2 != 0)
					{
						if ((attrib & BIT_4) == BIT_4)
							pxl = obp[4 + dot1 + dot2 + dot2];
						else
							pxl = obp[dot1 + dot2 + dot2];
						g_pixels[(uint16_t)rx + (uint16_t)line * 160] = pxl;
					}
				}
			}
		}
		else if (obj[i].type == WINDOW_TILE)
		{
			uint8_t	obj_ypos = obj[i].lcd_y;
			uint8_t	obj_xpos = obj[i].lcd_x;

			obj_ypos -= WY_REGISTER;
			obj_ypos &= 7;
			obj_xpos += WX_REGISTER;

			for (uint8_t x = 0; x < 8; x++)
			{
				uint8_t rx = obj_xpos - x;
				if (rx >= 160 || rx <= WX_REGISTER - 7)
					continue;

				uint8_t dot1, dot2;
				if (obj[i].code >= 0x80u)
				{
					dot1 = (vram[(uint16_t)obj[i].code * 16 + (uint16_t)obj_ypos * 2] >> x) & 1;
					dot2 = (vram[(uint16_t)obj[i].code * 16 + (uint16_t)obj_ypos * 2 + 1] >> x) & 1;
				}
				else
				{
					dot1 = (vram[(uint16_t)obj[i].code * 16 + 4096 + (uint16_t)obj_ypos * 2] >> x) & 1;
					dot2 = (vram[(uint16_t)obj[i].code * 16 + 4097 + (uint16_t)obj_ypos * 2] >> x) & 1;
				}
				int32_t pxl = bgp[dot1 + dot2 + dot2];
				g_pixels[(uint16_t)rx + (uint16_t)line * 160] = pxl;
			}
		}
		else
		{
			uint8_t obj_ypos = obj[i].lcd_y;
			uint8_t	obj_xpos = obj[i].lcd_x - SCX_REGISTER;

			obj_ypos -= (SCY_REGISTER + line) & ~(7);

			for (uint8_t x = 0; x < 8; x++)
			{
				uint8_t	rx = obj_xpos + 7 - x;
				if (rx >= 160)
					continue;

				uint8_t dot1, dot2;
				if ((LCDC_REGISTER & BIT_4) == BIT_4 || obj[i].code >= 0x80u)
				{
					dot1 = (vram[(uint16_t)obj[i].code * 16 + (uint16_t)obj_ypos * 2] >> x) & 1;
					dot2 = (vram[(uint16_t)obj[i].code * 16 + (uint16_t)obj_ypos * 2 + 1] >> x) & 1;
				}
				else
				{
					dot1 = (vram[(uint16_t)obj[i].code * 16 + 4096 + (uint16_t)obj_ypos * 2] >> x) & 1;
					dot2 = (vram[(uint16_t)obj[i].code * 16 + 4097 + (uint16_t)obj_ypos * 2] >> x) & 1;
				}
				int32_t pxl = bgp[dot1 + dot2 + dot2];
				g_pixels[(uint16_t)rx + (uint16_t)line * 160] = pxl;
			}
		}
	}
}

void	draw_line(oam_t *oam, int line)
{
	if ((LCDC_REGISTER & BIT_7) != BIT_7)
		return ;
	if (!g_cart.cgb_mode)
		VBK_REGISTER = 0;
	else
		VBK_REGISTER &= BIT_0;

	uint8_t	offset = (oam->active) ? 40 : 0;
	if ((LCDC_REGISTER & BIT_5) == BIT_5 && WY_REGISTER <= (uint8_t)line)
	{
		uint16_t	address = ((LCDC_REGISTER & BIT_6) == BIT_6) ?
			0x400u : 0x0u;

		uint8_t align_wy = ((uint8_t)line - WY_REGISTER) & ~(7);
		for (uint8_t x = 0; x < 32; x++)
		{
			oam->obj[offset + x].lcd_y = (uint8_t)line;
			oam->obj[offset + x].lcd_x = x << 3;

			oam->obj[offset + x].code =
				g_memmap.vram_bg[0][address + (uint16_t)align_wy * 4 + x];

			if (!g_cart.cgb_mode)
			{
				oam->obj[offset + x].attrib = 0;
			}
			else
			{
				oam->obj[offset + x].attrib =
					g_memmap.vram_bg[1][address + align_wy * 4 + x];
			}

			if ((oam->obj[offset + x].attrib & BIT_7) == BIT_7)
				oam->obj[offset + x].prior = BG_BG_PRIOR;
			else
				oam->obj[offset + x].prior = BG_OBJ_PRIOR;

			oam->obj[offset + x].type = WINDOW_TILE;
			oam->obj[offset + x].next_prior = 0;
		}
		offset += 32;
	}
	if ((LCDC_REGISTER & BIT_0) == BIT_0 || g_cart.cgb_mode)
	{
		uint16_t	address = ((LCDC_REGISTER & BIT_3) == BIT_3) ?
			0x400u : 0x0u;

		uint8_t		align_scy = (SCY_REGISTER + (uint8_t)line) & ~(7);
		for (uint8_t x = 0; x < 32; x++)
		{
			oam->obj[offset + x].lcd_y = (SCY_REGISTER + (uint8_t)line);
			oam->obj[offset + x].lcd_x = x << 3;

			oam->obj[offset + x].code =
				g_memmap.vram_bg[0][address + (uint16_t)align_scy * 4 + x];

			if (!g_cart.cgb_mode)
				oam->obj[offset + x].attrib = 0;
			else
				oam->obj[offset + x].attrib =
					g_memmap.vram_bg[1][address + (uint16_t)align_scy * 4 + x];

			if ((oam->obj[offset + x].attrib & BIT_7) == BIT_7)
				oam->obj[offset + x].prior = BG_BG_PRIOR;
			else
				oam->obj[offset + x].prior = BG_OBJ_PRIOR;

			oam->obj[offset + x].type = BG_TILE;
			oam->obj[offset + x].next_prior = 0;
		}
		offset += 32;
	}

	qsort((object_t *)oam->obj, offset, sizeof(object_t), &cmp_prior);

	if (!g_cart.cgb_mode)
		draw_dmg_line((object_t *)oam->obj, offset, (uint8_t)line);
}
