/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   timing.h                                         .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: mhouppin <mhouppin@le-101.fr>              +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/05 05:04:07 by mhouppin     #+#   ##    ##    #+#       */
/*   Updated: 2019/07/05 05:24:37 by mhouppin    ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#ifndef TIMING_H
# define TIMING_H

# include "processor.h"
# include "lcd_driver.h"

typedef struct	timings_s
{
	oam_t			oam;
	cycle_count_t	lcd_cycles;
	int				line_render;
	int				render_status;
	cycle_count_t	timer_cycles;
	cycle_count_t	div_cycles;
}				timings_t;

extern timings_t	g_timing;

#endif
