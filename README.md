# libmalloc99
Memory efficient implementation of a heap allocator for tms9900-gcc

This is a rather simple but memory efficient heap memory allocator for the tms9900 port of gcc.

## The library assumes the following:
	* The .bss segment lives in the higher memory expansion
	* The .bss segment is the /last/ segment in memory, everything beyond that up to 0xffff is free
	* The \_bss\_end symbol is provided by the linker script (it should be, most crt0.c's rely on it, I believe)

## Known limitations & behavior:
	* It uses a single word chunk_info header to keep track of the heap, this means that every call to malloc(size) will result in size + 2 bytes of memory being reserved.
	* malloc(), calloc(), realloc() and free() are implemented
	* freeing incorrect pointers will mess up the heap immediately
	* the free() function coalesces where adjacent free chunks of memory are created to avoid too much fragmentation, but it does so in a rather inefficient way. So while malloc() is quite fast, free() can take a bit more time.

