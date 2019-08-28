#ifndef REGISTERS_H
# define REGISTERS_H

# include <stdint.h>

# if (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)

#  define reg_a		bytes[0]
#  define reg_f		bytes[1]
#  define reg_b		bytes[2]
#  define reg_c		bytes[3]
#  define reg_d		bytes[4]
#  define reg_e		bytes[5]
#  define reg_h		bytes[6]
#  define reg_l		bytes[7]

# else

#  define reg_a		bytes[1]
#  define reg_f		bytes[0]
#  define reg_b		bytes[3]
#  define reg_c		bytes[2]
#  define reg_d		bytes[5]
#  define reg_e		bytes[4]
#  define reg_h		bytes[7]
#  define reg_l		bytes[6]

# endif

# define reg_af		packs[0]
# define reg_bc		packs[1]
# define reg_de		packs[2]
# define reg_hl		packs[3]
# define reg_pc		packs[4]
# define reg_sp		packs[5]

# define wbytes		bytes[12]
# define wval_1		bytes[14]
# define wval_2		bytes[15]
# define waddr_1	packs[8]
# define waddr_2	packs[9]

# define FLAG_Z		0x80u
# define FLAG_N		0x40u
# define FLAG_H		0x20u
# define FLAG_CY	0x10u

typedef union	registers_u
{
	uint8_t		bytes[20];
	uint16_t	packs[10];
}				registers_t;

#endif
