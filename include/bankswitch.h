#ifndef BANKSWITCH_H
#define BANKSWITCH_H

// Variable to store our current bank, used by all trampoline functions
volatile unsigned int currentbank = 0;

// Where to write to, to switch banks
// You can switch banks by addressing this as an array, and writing
// any value to it.
// 		e.g.	bankbase[2] = 1;
// This will switch to bank 2
volatile unsigned int *bankbase = (unsigned int*)0x6000;

#endif