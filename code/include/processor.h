#ifndef PROCESSOR_H
# define PROCESSOR_H

# include "registers.h"

typedef unsigned long	cycle_count_t;

cycle_count_t	execute(registers_t *regs);

#endif
