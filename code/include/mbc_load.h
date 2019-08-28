#ifndef MBC_LOAD_H
# define MBC_LOAD_H

# include <stdint.h>

int		set_cartridge_info(uint8_t *mem);
void	load_rom_only_cartridge(uint8_t *mem);
void	load_MBC1_cartridge(uint8_t *mem);
void	load_MBC2_cartridge(uint8_t *mem);
void	load_MBC3_cartridge(uint8_t *mem);
void	load_MBC5_cartridge(uint8_t *mem);

void	malloc_blocks(void);
void	load_cartridge_on_memory(uint8_t *mem);

#endif
