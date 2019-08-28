/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   mbc_load.c                                       .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: mhouppin <mhouppin@le-101.fr>              +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/05 01:49:02 by mhouppin     #+#   ##    ##    #+#       */
/*   Updated: 2019/07/05 01:49:05 by mhouppin    ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include <ctype.h>
#include <string.h>
#include "mbc_load.h"
#include "cartridge.h"
#include "memory_map.h"

#define NINTENDO_LOGO		"\xCE\xED\x66\x66\xCC\x0D\x00\x0B\x03\x73\x00\x83"\
							"\x00\x0C\x00\x0D\x00\x08\x11\x1F\x88\x89\x00\x0E"\
							"\xDC\xCC\x6E\xE6\xDD\xDD\xD9\x99\xBB\xBB\x67\x63"\
							"\x6E\x0E\xEC\xCC\xDD\xDC\x99\x9F\xBB\xB9\x33\x3E"

int		set_cartridge_info(uint8_t *mem)
{
	uint8_t	*start = mem;
	uint8_t	sum;

	mem += 0x100;
	if (mem[0] != 0x00u || mem[1] != 0xC3u)
		return (-1);
	mem += 2;

#if (__BYTE_ORDER == __ORDER_LITTLE_ENDIAN)
	g_cart.jump_addr = *(uint16_t *)mem;
#else
	g_cart.jump_addr = (uint16_t)mem[0] | (((uint16_t)mem[1]) << 8);
#endif

	mem += 2;

	if (memcmp(mem, NINTENDO_LOGO, 48) != 0)
		return (-1);
	
	mem += 48;

	for (uint32_t i = 0; i < 11; i++)
		if (mem[i] != '\0' && !isalnum(mem[i]) && mem[i] != '_' && mem[i] != ' ')
			return (-1);
	
	strncpy((char *)g_cart.game_title, (char *)mem, 11);
	g_cart.game_title[11] = '\0';
	mem += 11;

	for (uint32_t i = 0; i < 4; i++)
		if (mem[i] && (mem[i] < 'A' || mem[i] > 'Z') && !isdigit(mem[i]))
			return (-1);
	
	strncpy((char *)g_cart.game_code, (char *)mem, 4);
	g_cart.game_code[4] = '\0';
	mem += 4;

	if (*mem > 0x80u && *mem != 0xC0u)
		return (-1);
	
	g_cart.cgb_mode = (*mem >= 0x80u);
	mem++;

	g_cart.maker_code[0] = *(mem++);
	g_cart.maker_code[1] = *(mem++);
	g_cart.maker_code[2] = 0;

	if (*mem == 0x03 && start[0x14B] != 0x33)
		return (-1);
	g_cart.sgb_support_code = *(mem++);

	g_cart.type = *(mem++);

	if (*mem > 0x08u)
		return (-1);
	g_cart.rom_size = *(mem++);

	if (*mem > 0x04u)
		return (-1);
	g_cart.extern_ram_size = *(mem++);

	if (*mem > 0x01u)
		return (-1);
	g_cart.destination_code = *mem;
	mem += 2;

	g_cart.rom_version = *(mem++);

	sum = 0;
	for (uint32_t i = 0x134; i < 0x14d; i++)
	{
		sum += start[i];
	}
	sum += 0x19u + *mem;
	if (sum != 0x0)
		return (-1);
	g_cart.sum_complement = *(mem++);

	return (g_cart.type);
}

void	load_rom_only_cartridge(uint8_t *mem)
{
	g_cart.mbc = ROM_ONLY;
	malloc_blocks();
	load_cartridge_on_memory(mem);
}

void	load_MBC1_cartridge(uint8_t *mem)
{
	g_cart.mbc = MBC1;
	malloc_blocks();
	load_cartridge_on_memory(mem);
}

void	load_MBC2_cartridge(uint8_t *mem)
{
	g_cart.mbc = MBC2;
	malloc_blocks();
	load_cartridge_on_memory(mem);
}

void	load_MBC3_cartridge(uint8_t *mem)
{
	g_cart.mbc = MBC3;
	malloc_blocks();
	load_cartridge_on_memory(mem);
}

void	load_MBC5_cartridge(uint8_t *mem)
{
	g_cart.mbc = MBC5;
	malloc_blocks();
	load_cartridge_on_memory(mem);
}
