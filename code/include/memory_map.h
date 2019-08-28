/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   memory_map.h                                     .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: mhouppin <mhouppin@le-101.fr>              +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/05/23 11:44:01 by mhouppin     #+#   ##    ##    #+#       */
/*   Updated: 2019/08/09 15:42:03 by mhouppin    ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#ifndef MEMORY_MAP_H
# define MEMORY_MAP_H

# include <stdint.h>
# include <stdbool.h>
# include <stddef.h>

# define NORMAL_MODE	0
# define HALT_MODE		1
# define STOP_MODE		2

# define BGP_REGISTER	g_memmap.complete_block[0xFF47u]
# define DIV_REGISTER	g_memmap.complete_block[0xFF04u]
# define DMA_REGISTER	g_memmap.complete_block[0xFF46u]
# define IE_REGISTER	g_memmap.complete_block[0xFFFFu]
# define IF_REGISTER	g_memmap.complete_block[0xFF0Fu]
# define IME_REGISTER	g_memmap.ime
# define KEY1_REGISTER	g_memmap.complete_block[0xFF4Du]
# define LCDC_REGISTER	g_memmap.complete_block[0xFF40u]
# define LY_REGISTER	g_memmap.complete_block[0xFF44u]
# define LYC_REGISTER	g_memmap.complete_block[0xFF45u]
# define OBP0_REGISTER	g_memmap.complete_block[0xFF48u]
# define OBP1_REGISTER	g_memmap.complete_block[0xFF49u]
# define P1_REGISTER	g_memmap.complete_block[0xFF00u]
# define SCX_REGISTER	g_memmap.complete_block[0xFF43u]
# define SCY_REGISTER	g_memmap.complete_block[0xFF42u]
# define STAT_REGISTER	g_memmap.complete_block[0xFF41u]
# define SVBK_REGISTER	g_memmap.complete_block[0xFF70u]
# define TAC_REGISTER	g_memmap.complete_block[0xFF07u]
# define TIMA_REGISTER	g_memmap.complete_block[0xFF05u]
# define TMA_REGISTER	g_memmap.complete_block[0xFF06u]
# define VBK_REGISTER	g_memmap.complete_block[0xFF4Fu]
# define WX_REGISTER	g_memmap.complete_block[0xFF4Bu]
# define WY_REGISTER	g_memmap.complete_block[0xFF4Au]

typedef struct	memory_map_s
{

	uint8_t		*complete_block;
	uint8_t		*fixed_rom;
	uint8_t		*switch_rom;
	uint8_t		*rom_banks[512];
	uint8_t		*vram;
	uint8_t		*vram_banks[2];
	uint8_t		*vram_bg[2];
	uint8_t		*extern_ram;
	uint8_t		*extern_ram_banks[16];
	uint8_t		*fixed_ram;
	uint8_t		*switch_ram;
	uint8_t		*ram_banks[7];
	uint8_t		*redzone;
	uint8_t		*oam;
	uint8_t		*cpu_redzone;
	uint8_t		*hardware_regs;
	uint8_t		*stack_ram;
	uint8_t		*int_flags;
	_Bool		ime;
//	uint32_t	cur_extern_ram;
	uint8_t		cart_reg[8];
	char		*save_name;
	uint32_t	save_size;

}				memory_map_t;

extern uint8_t			*g_get_real_addr[16];
extern memory_map_t		g_memmap;
extern uint32_t			GAMEBOY_STATUS;
extern unsigned long	GAMEBOY_SPEED;

# define GET_REAL_ADDR(virtual_addr)	\
		 (\
			(g_get_real_addr[((virtual_addr) >> 12)]) ?\
				g_get_real_addr[((virtual_addr) >> 12)] + ((virtual_addr) & 0xfff) :\
				(virtual_addr) + g_memmap.complete_block\
		 )

uint32_t	get_external_ram_size(void);

#endif
