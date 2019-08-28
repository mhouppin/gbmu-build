/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   cpu_specs.h                                      .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: mhouppin <mhouppin@le-101.fr>              +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/05 04:53:42 by mhouppin     #+#   ##    ##    #+#       */
/*   Updated: 2019/07/05 04:54:28 by mhouppin    ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#ifndef CPU_SPECS_H
# define CPU_SPECS_H

# include "registers.h"

void	update_timer_values(unsigned long cycles);
void	check_interrupts(registers_t *regs);

#endif
