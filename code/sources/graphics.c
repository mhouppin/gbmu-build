/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   graphics.c                                       .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: mhouppin <mhouppin@le-101.fr>              +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/05 01:48:43 by mhouppin     #+#   ##    ##    #+#       */
/*   Updated: 2019/08/08 14:16:47 by mhouppin    ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "graphics.h"
#include "cartridge.h"

extern uint32_t	g_ticks;

SDL_Window		*g_window;
SDL_Renderer	*g_render;
SDL_Texture		*g_texture;
int32_t			*g_pixels;
int				g_pitch;

SDL_Window		*g_dwindow;
SDL_Renderer	*g_drender;
SDL_Texture		*g_dtexture;
int32_t			*g_dpixels;

void	gr_init_window(void)
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS) != 0)
	{
		fprintf(stderr, "SDL_Init() failed: %s", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	g_window = SDL_CreateWindow(g_cart.game_title,
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 160, 144,
		SDL_WINDOW_RESIZABLE);

	g_dwindow = SDL_CreateWindow("RAM dump",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 256, 32 + 32 * g_cart.n_ram_banks,
		SDL_WINDOW_RESIZABLE);
	if (g_window == NULL || g_dwindow == NULL)
	{
		fprintf(stderr, "SDL_CreateWindow() failed: %s", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	g_render = SDL_CreateRenderer(g_window, -1, 0);
	g_drender = SDL_CreateRenderer(g_dwindow, -1, 0);
	if (g_render == NULL || g_drender == NULL)
	{
		fprintf(stderr, "SDL_CreateRenderer() failed: %s", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	SDL_SetRenderDrawColor(g_render, 0, 0, 0, 255);
	SDL_SetRenderDrawColor(g_drender, 0, 0, 0, 255);

	g_texture = SDL_CreateTexture(g_render, SDL_PIXELFORMAT_ARGB8888,
			SDL_TEXTUREACCESS_STREAMING, 160, 144);
	g_dtexture = SDL_CreateTexture(g_drender, SDL_PIXELFORMAT_ARGB8888,
			SDL_TEXTUREACCESS_STREAMING, 256, 32 + 32 * g_cart.n_ram_banks);
	if (g_texture == NULL || g_dtexture == NULL)
	{
		fprintf(stderr, "SDL_CreateTexture() failed: %s", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	SDL_RenderSetLogicalSize(g_render, 160, 144);
	SDL_RenderSetLogicalSize(g_drender, 256, 32 + 32 * g_cart.n_ram_banks);

	SDL_LockTexture(g_texture, NULL, (void **)&g_pixels, &g_pitch);
	SDL_LockTexture(g_dtexture, NULL, (void **)&g_dpixels, &g_pitch);

	g_ticks = SDL_GetTicks();
}
