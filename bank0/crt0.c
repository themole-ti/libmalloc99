// crt0.c - C runtime initialization code
#include "trampolines.h"

extern unsigned int  _text;
extern unsigned int  _text_end;
extern unsigned int  _bss;
extern unsigned int  _bss_end;
extern unsigned int  _persistent;
extern unsigned int  _persistent_src;
extern unsigned int  _persistent_end;
extern unsigned int  _data_bank;

// Linker will look for _start symbol as the entry point of our program
void _start()
{
	// Start by turning off interupts, and setting the workspace pointer
	__asm__
	(
		"limi	0	\n\t"
		"lwpi	0x8300\n\t"
	);

	// The symbols starting with '_' are defined in the linker script
	// They point to the ROM locations for each section
	unsigned int *src = &_persistent_src;
	unsigned int *dst = &_persistent;

	// Copy persistent code (ie, non-bankswitchable) to lower memory expansion
	while (dst < &_persistent_end)
		*dst++ = *src++;

	// Copy initial data from ROM to higher memory expansion
	init_data_section();

	// Zero BSS
	for (dst = &_bss; dst < &_bss_end; dst++)
		*dst = 0;

	// Set stack
	__asm__
	(
		"li	sp, 0x4000\n\t"
	);

	// Start executing C program from main function
	__asm__
	(
		"b	@main\n\t"
	);
}
