/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   settings.h                                       .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: mhouppin <mhouppin@le-101.fr>              +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/05 07:18:17 by mhouppin     #+#   ##    ##    #+#       */
/*   Updated: 2019/07/05 07:19:31 by mhouppin    ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#ifndef SETTINGS_H
# define SETTINGS_H

# include <stdbool.h>

typedef struct	settings_s
{
	bool	debug_mode;
	bool	uspeed_mode;
}				settings_t;

extern settings_t	g_settings;

#endif
