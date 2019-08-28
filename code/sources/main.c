/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   main.c                                           .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: mhouppin <mhouppin@le-101.fr>              +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/05 01:48:55 by mhouppin     #+#   ##    ##    #+#       */
/*   Updated: 2019/07/05 01:48:56 by mhouppin    ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "cartridge.h"
#include "graphics.h"
#include "launcher.h"
#include <stdio.h>

int		main(int argc, char **argv)
{
	if (argc != 2)
	{
		fprintf(stderr, "%s cartridge_file\n", *argv);
		return (1);
	}

	open_cartridge(argv[1]);

	gr_init_window();

	start_game();
	return (0);
}
