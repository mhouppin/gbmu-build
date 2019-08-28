/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   mbc_swap.c                                       .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: mhouppin <mhouppin@le-101.fr>              +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/05 01:56:39 by mhouppin     #+#   ##    ##    #+#       */
/*   Updated: 2019/08/13 14:24:15 by mhouppin    ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "memory_map.h"
#include "cartridge.h"
#include <stdio.h>

void	MBC1_swap(uint8_t address, uint8_t value)
{
	uint32_t	rom_bank_no;

	if (address < 0x20u)
	{
		if ((value & 0x0Fu) == 0x0Au)
		{
			g_memmap.cart_reg[0] = 1;
		}
		else
		{
			g_memmap.cart_reg[0] = 0;
		}
	}
	else if (address < 0x40u)
	{
		g_memmap.cart_reg[1] = (value & 0x1Fu);
		goto switch_rom_ram;
	}
	else if (address < 0x60u)
	{
		g_memmap.cart_reg[2] = (value & 0x03u);

switch_rom_ram:
		rom_bank_no = (uint32_t)(g_memmap.cart_reg[1]) | ((uint32_t)g_memmap.cart_reg[2] << 5);
		if ((rom_bank_no & 0x1Fu) == 0)
			rom_bank_no++;
		rom_bank_no %= g_cart.n_rom_banks;
		if (g_memmap.cart_reg[3] == 1)
		{
			if (g_cart.n_ram_banks == 0)
				g_memmap.extern_ram = g_memmap.complete_block + 0xA000u;
			else
				g_memmap.extern_ram = g_memmap.extern_ram_banks[g_memmap.cart_reg[2]];
			g_memmap.switch_rom = g_memmap.rom_banks[rom_bank_no & 0x1Fu];
		}
		else
		{
			if (g_cart.n_ram_banks == 0)
				g_memmap.extern_ram = g_memmap.complete_block + 0xA000u;
			else
				g_memmap.extern_ram = g_memmap.extern_ram_banks[0];
			g_memmap.switch_rom = g_memmap.rom_banks[rom_bank_no];
		}
		g_get_real_addr[0xA] = g_memmap.extern_ram;
		g_get_real_addr[0xB] = g_memmap.extern_ram + 0x1000u;
		g_get_real_addr[0x4] = g_memmap.switch_rom;
		g_get_real_addr[0x5] = g_memmap.switch_rom + 0x1000u;
		g_get_real_addr[0x6] = g_memmap.switch_rom + 0x2000u;
		g_get_real_addr[0x7] = g_memmap.switch_rom + 0x3000u;
	}
	else
	{
		g_memmap.cart_reg[3] = (value & 0x01u);
	}
}

void	MBC3_swap(uint8_t address, uint8_t value)
{
	(void)address;
	(void)value;
}

void	MBC5_swap(uint8_t address, uint8_t value)
{
	uint32_t	rom_bank_no;

	if (address < 0x20u)
	{
		if ((value & 0x0Fu) == 0x0Au)
		{
			g_memmap.cart_reg[0] = 1;
		}
		else
		{
			g_memmap.cart_reg[0] = 0;
		}
	}
	else if (address < 0x30u)
	{
		g_memmap.cart_reg[1] = value;
		goto switch_rom;
	}
	else if (address < 0x40u)
	{
		g_memmap.cart_reg[2] = (value & 0x01u);

switch_rom:
		rom_bank_no = (uint32_t)(g_memmap.cart_reg[1]) | ((uint32_t)g_memmap.cart_reg[2] << 8);
		rom_bank_no %= g_cart.n_rom_banks;
		g_memmap.switch_rom = g_memmap.rom_banks[rom_bank_no];
		g_get_real_addr[0x4] = g_memmap.switch_rom;
		g_get_real_addr[0x5] = g_memmap.switch_rom + 0x1000u;
		g_get_real_addr[0x6] = g_memmap.switch_rom + 0x2000u;
		g_get_real_addr[0x7] = g_memmap.switch_rom + 0x3000u;
	}
	else if (address < 0x60u)
	{
		g_memmap.cart_reg[3] = (value & 0x0Fu);
		if (g_cart.n_ram_banks == 0)
		{
			g_get_real_addr[0xA] = g_memmap.complete_block + 0xA000u;
			g_get_real_addr[0xB] = g_memmap.complete_block + 0xB000u;
		}
		else
		{
			g_get_real_addr[0xA] = g_memmap.extern_ram_banks[value & 0x0Fu];
			g_get_real_addr[0xB] = g_memmap.extern_ram_banks[value & 0x0Fu] + 0x1000;
		}
	}
}

void	mbc_swap(uint8_t address, uint8_t value)
{
	switch (g_cart.mbc)
	{
		case MBC1:
		case MBC2:
			MBC1_swap(address, value);
			break ;

		case MBC3:
			MBC3_swap(address, value);
			break ;

		case MBC5:
			MBC5_swap(address, value);
			break ;
	}
}
