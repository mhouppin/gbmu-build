/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   memory_map.c                                     .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: mhouppin <mhouppin@le-101.fr>              +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/08/09 12:16:21 by mhouppin     #+#   ##    ##    #+#       */
/*   Updated: 2019/08/09 12:16:22 by mhouppin    ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "memory_map.h"
#include "cartridge.h"

uint8_t			*g_get_real_addr[16];
memory_map_t	g_memmap;
uint32_t		GAMEBOY_STATUS;
unsigned long	GAMEBOY_SPEED;

uint32_t	get_external_ram_size(void)
{
	switch (g_cart.extern_ram_size)
	{
		case 0x02: g_cart.n_ram_banks = 1; return (0x2000);
		case 0x03: g_cart.n_ram_banks = 4; return (0x8000);
		case 0x04: g_cart.n_ram_banks = 16; return (0x20000);
		case 0x05: g_cart.n_ram_banks = 8; return (0x10000);
	}
	return (0);
}

void		malloc_blocks(void)
{
	bzero(&g_memmap, sizeof(memory_map_t));

	g_memmap.complete_block = valloc(0x10000);

	g_memmap.vram = valloc(0x4000);
	g_memmap.vram_banks[0] = g_memmap.vram;
	g_memmap.vram_banks[1] = g_memmap.vram + 0x2000;
	g_memmap.vram_bg[0] = g_memmap.vram + 0x1800;
	g_memmap.vram_bg[1] = g_memmap.vram + 0x3800;

	if ((g_memmap.save_size = get_external_ram_size()) == 0)
	{
		g_memmap.fixed_ram = valloc(0x8000);
	}
	else
	{
		g_memmap.extern_ram = valloc(g_memmap.save_size + 0x8000);
		for (unsigned int i = 0; i < g_cart.n_ram_banks; i++)
		{
			g_memmap.extern_ram_banks[i] = g_memmap.extern_ram + (0x2000 * i);
		}
		g_memmap.fixed_ram = g_memmap.extern_ram + g_memmap.save_size;
	}

	g_memmap.switch_ram = g_memmap.fixed_ram + 0x1000;
	for (unsigned int i = 0; i < 7; i++)
	{
		g_memmap.ram_banks[i] = g_memmap.switch_ram + (0x1000 * i);
	}

	g_memmap.redzone = g_memmap.complete_block + 0xE000;
	g_memmap.oam = g_memmap.complete_block + 0xFE00;
	g_memmap.cpu_redzone = g_memmap.complete_block + 0xFEA0;
	g_memmap.hardware_regs = g_memmap.complete_block + 0xFF00;
	g_memmap.stack_ram = g_memmap.complete_block + 0xFF80;
	g_memmap.int_flags = g_memmap.complete_block + 0xFFFF;

	g_get_real_addr[0x8] = g_memmap.vram;
	g_get_real_addr[0x9] = g_memmap.vram + 0x1000;
	if (g_memmap.extern_ram == NULL)
	{
		g_get_real_addr[0xA] = g_memmap.complete_block + 0xA000;
		g_get_real_addr[0xB] = g_memmap.complete_block + 0xB000;
	}
	else
	{
		g_get_real_addr[0xA] = g_memmap.extern_ram;
		g_get_real_addr[0xB] = g_memmap.extern_ram + 0x1000;
	}
	g_get_real_addr[0xC] = g_memmap.fixed_ram;
	g_get_real_addr[0xD] = g_memmap.switch_ram;
}

void		load_cartridge_on_memory(uint8_t *mem)
{
	uint32_t	cartridge_size;
	uint8_t		*rom;

	cartridge_size = (0x8000 << g_cart.rom_size);
	g_cart.n_rom_banks = (2 << g_cart.rom_size);

	if (cartridge_size != g_cart.size)
	{
		fprintf(stderr, "Invalid cartridge size (%lu, should be %lu)\n",
			(unsigned long)g_cart.size, (unsigned long)cartridge_size);
		exit(EXIT_FAILURE);
	}

	rom = valloc(cartridge_size);
	memcpy(rom, mem, cartridge_size);

	g_memmap.fixed_rom = rom;
	for (unsigned int i = 0; i < g_cart.n_rom_banks; i++)
	{
		g_memmap.rom_banks[i] = rom + (0x4000 * i);
	}

	g_memmap.switch_rom = rom + 0x4000;

	for (unsigned int i = 0; i < 8; i++)
	{
		g_get_real_addr[i] = rom + (0x1000 * i);
	}
}
