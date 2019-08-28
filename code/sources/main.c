/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   main.c                                           .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: mhouppin <mhouppin@le-101.fr>              +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/05 01:48:55 by mhouppin     #+#   ##    ##    #+#       */
/*   Updated: 2019/08/28 16:46:49 by mhouppin    ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "cartridge.h"
#include "graphics.h"
#include "launcher.h"
#include <stdio.h>

int		main(void)
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS) != 0)
	{
		fprintf(stderr, "SDL_Init() failed: %s", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	g_window = SDL_CreateWindow(g_cart.game_title,
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 160, 144,
		SDL_WINDOW_RESIZABLE);
	g_render = SDL_CreateRenderer(g_window, -1, 0);
	SDL_SetRenderDrawColor(g_render, 0, 0, 0, 255);
	g_texture = SDL_CreateTexture(g_render, SDL_PIXELFORMAT_ARGB8888,
			SDL_TEXTUREACCESS_STREAMING, 160, 144);
	SDL_RenderSetLogicalSize(g_render, 160, 144);
	SDL_LockTexture(g_texture, NULL, (void **)&g_pixels, &g_pitch);

	SDL_Event ev;
	while (SDL_PollEvent(&ev))
	{
		if (ev.type == SDL_QUIT || (ev.type == SDL_WINDOWEVENT &&
					ev.window.type == SDL_WINDOWEVENT_CLOSE))
			exit(EXIT_SUCCESS);
		else if (ev.type == SDL_DROPFILE)
		{
			open_cartridge(ev.drop.file);
			gr_init_window();
			SDL_free(ev.drop.file);
			break ;
		}
	}

	start_game();
	return (0);
}
