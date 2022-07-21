/* Linker script to create TI99/4A bankswitched cartridges */

/* Output to a flat binary format (i.e. not ELF) */
OUTPUT_FORMAT(binary)
OUTPUT(cartridge.bin)

/* TI memory layout */
MEMORY
{
	cart_rom (rx)   : ORIGIN = 0x6000, LENGTH = 0x2000 		/* cartridge ROM, read-only */
	lower_exp (wx)  : ORIGIN = 0x2080, LENGTH = 0x1f80 		/* 8k - 128 bytes       */
	higher_exp (wx) : ORIGIN = 0xa000, LENGTH = 0x6000
	scratchpad (wx) : ORIGIN = 0x8320, LENGTH = 0x00e0 		/* 32b is for workspace */
}

/* Where we put sections */
SECTIONS
{
	. = 0x6000;
	.header : { bank0/cart_header.o(.text) } >cart_rom																	/* Bank 0: Cart header */
	_persistent_src = 0x601a;
	.persistent	: AT ( _persistent_src ) {_persistent = . ; persistent/*:*(.text); persistent/*(.text); _persistent_end = . ;} >lower_exp /* Persistent: Code that never can get bankswitched out */
	.bank0 (LOADADDR(.persistent) + SIZEOF( .persistent )) : { _text = . ; bank0/*.o(.text); _text_end = . ;}			/* Bank 0: code */
	.bank1 0x6000 : AT ( 0x8000 ) { bank1/*.o(.text); }																	/* Bank 1: code */
	.bank2 0x6000 : AT ( 0xa000 ) { bank2/*.o(.text); }																	/* Bank 2: code */
	.data  0xa000 : AT ( 0xc000 ) { _data = . ; persistent/*.a:*(.data); persistent/*.o ( .data ) bank0/*.o( .data ) bank1/*.o( .data ) bank2/*.o( .data ); _data_end = . ;}																				  /* Bank 3: data */
	.bss (_data_end) : { _bss = . ; *( .bss ) ; _bss_end = . ;}
	.fill  0xdfff : AT ( 0xdfff) { BYTE(0x00); }
}

/* Ensure banks don't call each other's functions directly */
NOCROSSREFS( .bank0 .bank1 .bank2 .bank3)
