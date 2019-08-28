/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   graphics.h                                       .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: mhouppin <mhouppin@le-101.fr>              +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/08 16:10:15 by mhouppin     #+#   ##    ##    #+#       */
/*   Updated: 2019/08/08 14:10:10 by mhouppin    ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#ifndef GRAPHICS_H
# define GRAPHICS_H

#include <stdint.h>
#include "SDL.h"

void	gr_init_window(void);

extern SDL_Window	*g_window;
extern SDL_Texture	*g_texture;
extern SDL_Renderer	*g_render;
extern int32_t		*g_pixels;
extern int			g_pitch;

extern SDL_Window	*g_dwindow;
extern SDL_Texture	*g_dtexture;
extern SDL_Renderer	*g_drender;
extern int32_t		*g_dpixels;

#endif
