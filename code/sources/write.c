/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   write.c                                          .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: mhouppin <mhouppin@le-101.fr>              +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/05 01:48:05 by mhouppin     #+#   ##    ##    #+#       */
/*   Updated: 2019/08/13 14:02:54 by mhouppin    ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "check_events.h"
#include "mbc_swap.h"
#include "cartridge.h"
#include "registers.h"
#include "memory_map.h"

void	launch_dma(uint8_t value)
{
	if (!g_cart.cgb_mode && value < 0x80u)
		return ;
	if (value >= 0xE0u)
		return ;

	uint8_t *address = GET_REAL_ADDR((uint16_t)value << 8);
	__builtin_memcpy(g_memmap.complete_block + 0xFE00u, address, 160);
}

void	write_end(uint8_t addr, uint8_t value)
{
	switch (addr)
	{
		case 0x68u: // BCPS register
		case 0x6Au: // OCPS register
		case 0x70u: // SVBK register
			g_memmap.complete_block[0xFF00u + addr] = value | 0x40u;
			break ;

		case 0x4Du: // KEY1 register
			g_memmap.complete_block[0xFF00u + addr] = (value & 0x7Fu) | 0x7Eu;
			break ;

		case 0x41u: // STAT register
			g_memmap.complete_block[0xFF00u + addr] = value | 0x80u;
			break ;

		case 0x00u: // P1 register
			g_memmap.complete_block[0xFF00u + addr] = value | 0xC0u;
			check_gb_events();
			break ;

		case 0x0Fu: // IF register
		case 0xFFu: // IE register
			g_memmap.complete_block[0xFF00u + addr] = value | 0xE0u;
			break ;

		case 0x07u: // TAC register
			g_memmap.complete_block[0xFF00u + addr] = value | 0xF8u;
			break ;

		case 0x4Fu: // VBK register
			g_memmap.complete_block[0xFF00u + addr] = value | 0xFEu;
			break ;

		case 0x04u: // DIV register
			g_memmap.complete_block[0xFF00u + addr] = 0;
			break ;

		case 0x46u: // DMA register
			launch_dma(value);
			break ;

		default: // Stack and unmasked registers
			g_memmap.complete_block[0xFF00u + addr] = value;
			break ;
	}
}

void	write_maybe(uint16_t address, uint8_t value)
{
	if (address >= 0xFF00u)
	{
		write_end((uint8_t)address, value);
		return ;
	}
	if (address < 0x8000u)
	{
		mbc_swap((uint8_t)(address >> 8), value);
		return ;
	}
	if (g_memmap.cart_reg[0] == 0 && address >= 0xA000u && address <= 0xBFFFu)
		return ;
	uint8_t	*where = GET_REAL_ADDR(address);
	*where = value;
}

void	write_bytes(registers_t *regs)
{
	if (regs->wbytes == 2)
		write_maybe(regs->waddr_2, regs->wval_2);
	write_maybe(regs->waddr_1, regs->wval_1);
}
