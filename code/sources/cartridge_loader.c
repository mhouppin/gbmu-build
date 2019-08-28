/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   cartridge_loader.c                               .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: mhouppin <mhouppin@le-101.fr>              +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/05 01:48:16 by mhouppin     #+#   ##    ##    #+#       */
/*   Updated: 2019/07/05 04:48:45 by mhouppin    ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mbc_load.h"
#include "cartridge.h"
#include "file_read.h"
#include "memory_map.h"

cartridge_t		g_cart;

static void	load_cartridge(uint8_t *mem, const char *path)
{
	if (set_cartridge_info(mem) == -1)
	{
		fprintf(stderr, "Invalid cartridge\n");
		exit(EXIT_FAILURE);
	}

	switch (g_cart.type)
	{
		case 0x00:
			load_rom_only_cartridge(mem);
			break ;

		case 0x01:
		case 0x02:
		case 0x03:
			load_MBC1_cartridge(mem);
			break ;

		case 0x05:
		case 0x06:
			load_MBC2_cartridge(mem);
			break ;

		case 0x0f:
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
			load_MBC3_cartridge(mem);
			break ;

		case 0x19:
		case 0x1a:
		case 0x1b:
		case 0x1c:
		case 0x1d:
		case 0x1e:
			load_MBC5_cartridge(mem);
			break ;

		default:
			fprintf(stderr, "Unsupported cartridge type\n");
			exit(EXIT_FAILURE);
	}
	
	for (size_t i = 0; i < 8; i++)
		g_memmap.cart_reg[i] = 0;
	
	switch (g_cart.type)
	{
		case 0x03:
		case 0x06:
		case 0x09:
		case 0x10:
		case 0x13:
		case 0x1b:
		case 0x1e:
			load_saved_external_ram(path);
	}
}

void		open_cartridge(const char *filename)
{
	uint8_t		*content;

	bzero(&g_cart, sizeof(cartridge_t));

	content = (uint8_t *)get_file_contents(filename, &(g_cart.size));
	if (content == NULL)
		exit(EXIT_FAILURE);
	load_cartridge(content, filename);
	free(content);
}
