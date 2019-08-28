/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   lcd_driver.h                                     .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: mhouppin <mhouppin@le-101.fr>              +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/05 05:24:42 by mhouppin     #+#   ##    ##    #+#       */
/*   Updated: 2019/07/08 12:12:07 by mhouppin    ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#ifndef LCD_DRIVER_H
# define LCD_DRIVER_H

# include "processor.h"
# include <stdbool.h>

# define OAM_READ					0
# define OAM_VRAM_READ				1
# define HZ_BLANK					2

# define OAM_READ_CYCLES			80
# define OAM_VRAM_READ_CYCLES		192
# define HZ_BLANK_CYCLES			204
# define VT_BLANK_WAITING_CYCLES	2879

#define BG_TILE			1
#define WINDOW_TILE		2
#define OBJ_TILE		3

#define BG_BG_PRIOR		49152u
#define OBJ_OBJ_PRIOR	32768u
#define BG_OBJ_PRIOR	16384u
#define OBJ_BG_PRIOR	0u

typedef struct	object_s
{
	uint8_t		lcd_y;
	uint8_t		lcd_x;
	uint8_t		code;
	uint8_t		attrib;
	uint8_t		type;
	uint8_t		next_prior;
	uint16_t	prior;
}				object_t;

typedef struct	oam_s
{
	object_t	obj[104];
	bool		active;
}				oam_t;

void	update_lcd(cycle_count_t cycles);
void	lcd_function(int line, int type);
void	update_display(void);
void	draw_line(oam_t *oam, int line);

#endif
