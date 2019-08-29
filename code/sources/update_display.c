/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   update_display.c                                 .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: mhouppin <mhouppin@le-101.fr>              +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/08 12:16:51 by mhouppin     #+#   ##    ##    #+#       */
/*   Updated: 2019/08/29 11:23:50 by mhouppin    ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "lcd_driver.h"
#include "graphics.h"
#include "settings.h"
#include "cartridge.h"
#include "memory_map.h"

cycle_count_t	g_ticks;

void	update_display(void)
{
	SDL_UnlockTexture(g_texture);
	SDL_RenderClear(g_render);
	SDL_RenderCopy(g_render, g_texture, NULL, NULL);
	SDL_RenderPresent(g_render);
	SDL_LockTexture(g_texture, NULL, (void **)&g_pixels, &g_pitch);

	if (!g_settings.debug_mode && !g_settings.uspeed_mode)
	{
		uint32_t cur_ticks = SDL_GetTicks();
		if (cur_ticks - g_ticks < 17)
			SDL_Delay(17 - (cur_ticks - g_ticks));
		g_ticks = SDL_GetTicks();
	}
}
