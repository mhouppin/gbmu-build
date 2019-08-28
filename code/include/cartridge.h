/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   cartridge.h                                      .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: mhouppin <mhouppin@le-101.fr>              +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/05 02:06:02 by mhouppin     #+#   ##    ##    #+#       */
/*   Updated: 2019/07/05 02:06:03 by mhouppin    ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#ifndef CARTRIDGE_H
# define CARTRIDGE_H

#include <stdint.h>
#include <stdbool.h>

enum	cartridge_types
{
	ROM_ONLY, MBC1, MBC2, MBC3, MBC5
};

typedef struct	cartridge_s
{
	int32_t		jump_addr;
	char		game_title[16];
	char		game_code[8];
	bool		cgb_mode;
	char		maker_code[8];
	uint8_t		sgb_support_code;
	int32_t		type;
	int32_t		rom_size;
	int32_t		extern_ram_size;
	int32_t		destination_code;
	int32_t		rom_version;
	int32_t		sum_complement;
	int32_t		hi_check_sum;
	int32_t		lo_check_sum;

	uint32_t	size;
	uint32_t	n_rom_banks;
	uint32_t	n_ram_banks;
	uint32_t	mbc;
}				cartridge_t;

extern cartridge_t	g_cart;

void	open_cartridge(const char *filename);

#endif
