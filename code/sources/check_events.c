/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   check_events.c                                   .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: mhouppin <mhouppin@le-101.fr>              +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/08 09:53:23 by mhouppin     #+#   ##    ##    #+#       */
/*   Updated: 2019/08/08 14:09:12 by mhouppin    ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "check_events.h"
#include "bitmask.h"
#include "graphics.h"
#include "memory_map.h"

void	check_cntrl_events(cycle_count_t cycles)
{
	static cycle_count_t	ev_cycles = 0;

	ev_cycles += cycles;
	if (ev_cycles > 10000)
	{
		ev_cycles -= 10000;

		if (GAMEBOY_STATUS != NORMAL_MODE)
			check_gb_events();

		SDL_Event ev;

		while (SDL_PollEvent(&ev))
			if (ev.type == SDL_WINDOWEVENT && ev.window.event == SDL_WINDOWEVENT_CLOSE)
			{
				if (ev.window.windowID == SDL_GetWindowID(g_window))
					exit(EXIT_SUCCESS);
				else if (ev.window.windowID == SDL_GetWindowID(g_dwindow))
				{
					SDL_DestroyTexture(g_dtexture);
					SDL_DestroyRenderer(g_drender);
					SDL_DestroyWindow(g_dwindow);
					g_dwindow = NULL;
				}
			}
	}
}

void	check_gb_events(void)
{
	bool			kint = false;
	const uint8_t	*keystate = SDL_GetKeyboardState(NULL);

	P1_REGISTER |= 0xFu;
	if ((P1_REGISTER & BIT_4) == 0 && (P1_REGISTER & BIT_5) == BIT_5)
	{
		if (keystate[SDL_SCANCODE_S] || keystate[SDL_SCANCODE_DOWN])
		{
			P1_REGISTER &= ~(BIT_3);
			kint = true;
		}
		if (keystate[SDL_SCANCODE_W] || keystate[SDL_SCANCODE_UP])
		{
			P1_REGISTER &= ~(BIT_2);
			kint = true;
		}
		if (keystate[SDL_SCANCODE_A] || keystate[SDL_SCANCODE_LEFT])
		{
			P1_REGISTER &= ~(BIT_1);
			kint = true;
		}
		if (keystate[SDL_SCANCODE_D] || keystate[SDL_SCANCODE_RIGHT])
		{
			P1_REGISTER &= ~(BIT_0);
			kint = true;
		}
	}
	if ((P1_REGISTER & BIT_4) == BIT_4 && (P1_REGISTER & BIT_5) == 0)
	{
		if (keystate[SDL_SCANCODE_SPACE])
		{
			P1_REGISTER &= ~(BIT_3);
			kint = true;
		}
		if (keystate[SDL_SCANCODE_RALT] || keystate[SDL_SCANCODE_LALT])
		{
			P1_REGISTER &= ~(BIT_2);
			kint = true;
		}
		if (keystate[SDL_SCANCODE_BACKSPACE])
		{
			P1_REGISTER &= ~(BIT_1);
			kint = true;
		}
		if (keystate[SDL_SCANCODE_RETURN])
		{
			P1_REGISTER &= ~(BIT_0);
			kint = true;
		}
	}
	if (kint && (IE_REGISTER & BIT_4) == BIT_4)
	{
		IF_REGISTER |= BIT_4;
	}
}
