/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   check_events.h                                   .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: mhouppin <mhouppin@le-101.fr>              +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/08 09:56:06 by mhouppin     #+#   ##    ##    #+#       */
/*   Updated: 2019/07/08 09:58:03 by mhouppin    ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#ifndef CHECK_EVENTS_H
# define CHECK_EVENTS_H

# include "processor.h"

void	check_cntrl_events(cycle_count_t cycles);
void	check_gb_events(void);

#endif
