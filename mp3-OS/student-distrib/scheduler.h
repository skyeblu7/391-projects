#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include "lib.h"
#include "types.h"

term_t terminals[3];
int currentTerminal;

void scheduler();
void switch_terminal(int terminal_num);

#endif

