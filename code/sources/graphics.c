/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   graphics.c                                       .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: mhouppin <mhouppin@le-101.fr>              +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/05 01:48:43 by mhouppin     #+#   ##    ##    #+#       */
/*   Updated: 2019/08/29 11:24:28 by mhouppin    ###    #+. /#+    ###.fr     */
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
