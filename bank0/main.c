/***************************/
/* libmalloc99 test suite  */
/* 2022 - Danny Lousberg   */
/***************************/

#include "conio.h"
#include "vdp.h"
#include "malloc.h"

#define SIZEOF_CHUNK_INFO 2
#define NUM_ITERATIONS    10
#define VINT_COUNTER 	  *((volatile unsigned char*)0x8379)

unsigned int rnd_xorshift()
{
	static unsigned int 	x = 1, y = 1;
	unsigned int			t = ( x^( x<<5 ) );

	x = y;

	return y = ( y^( y>>1) )^( t^( t>>3 ) );
}

void randomize()
{
	unsigned char seed = VINT_COUNTER;
	for (int i = 0; i < seed; i++)
	{
		rnd_xorshift();
	}
}

unsigned int test_malloc()
{
	unsigned int* ptrs[NUM_ITERATIONS];
	unsigned int total_free = get_free_heap();
	unsigned int sizes[NUM_ITERATIONS];

	// Assign some random size values
	for (int i = 0; i < NUM_ITERATIONS; i++)
	{
		sizes[i] = (rnd_xorshift() % 128);
	}

	// Allocate some pointers
	for (int i = 0; i < NUM_ITERATIONS; i++)
	{
		ptrs[i] = malloc(sizes[i]);
	}

	for (int i = 0; i < NUM_ITERATIONS; i++)
	{
		if (ptrs[i] == null)
			return -1;
	}
	cprintf(".");

	// Try to use all of allocated memory for one pointer
	ptrs[1][0] = (unsigned int)0xDEAD;
	for (int i = 0; i < (sizes[0]>>1); i++)
		ptrs[0][i] = i;
	if (ptrs[1][0] != (unsigned int)0xDEAD)
		return -4;
	cprintf(".");

	// Try to allocated too much memory
	if (malloc(get_free_heap() + 20))
		return -2;
	cprintf(".");

	// test freeing
	// We interleave the free calls to test coalescing
	for (int i = 0; i < NUM_ITERATIONS; i +=2)
	{
		free(ptrs[i + 1]);
		free(ptrs[i]);
	}
	cprintf(".");

	// Check for leaked memory
	if (total_free != get_free_heap())
		return -3;

	cprintf(".");

	return 0;
}

unsigned int test_calloc()
{
	unsigned int total_free = get_free_heap();	

	unsigned int* ptr1 = calloc(100, sizeof(unsigned int));
	unsigned int* ptr2 = calloc(100, sizeof(unsigned int));
	unsigned int* ptr3 = calloc(100, sizeof(unsigned int));
	cprintf(".");

	// Put some data on heap and free the associated pointer
	for (int i = 0; i < (100 >> 1); i++)
		ptr2[i] = i;
	free(ptr2);
	cprintf(".");

	// Reallocate space in free chunk with 0 initiallization
	ptr2 = calloc(50, sizeof(unsigned int));
	for (int i = 0; i < (50 >> 1); i++)
	{
		// Check for zeroes
		if (ptr2[i])
			return -1;
	}
	cprintf(".");

	free(ptr2);
	free(ptr3);
	free(ptr1);

	// Check for leaked memory
	if (total_free != get_free_heap())
		return -4;

	cprintf(".");

	return 0;
}

unsigned int test_realloc()
{
	unsigned int total_free = get_free_heap();	

	unsigned int* ptr1 = malloc(200);
	unsigned int* ptr2 = malloc(200);
	unsigned int* ptr3 = malloc(200);
	cprintf(".");

	// Test reallocating smaller size;
	unsigned int* newptr = realloc(ptr2, 100);
	if (newptr != ptr2)
		return -1;
	cprintf(".");

	// Test reallocating larger size with free chunk after current
	newptr = realloc(ptr2, 150);
	if (newptr != ptr2)
		return -2;
	cprintf(".");

	// Test reallocating larger size with need for memcpy
	for (int i = 0; i < (150 >> 1); i++)
		ptr2[i] = i;
	cprintf(".");

	newptr = realloc(ptr2, 250);
	if (newptr == ptr2)
		return -3;
	if (!newptr)
		return -3;
	cprintf(".");

	for (int i = 0; i < (150 >> 1); i++)
	{
		if (newptr[i] != i)
			return -4;
	}
	cprintf(".");

	free(newptr);
	free(ptr3);
	free(ptr1);

	// Check for leaked memory
	if (total_free != get_free_heap())
		return -4;

	cprintf(".");


	return 0;
}

inline void print_fail()
{
		gotox(74); cprintf("[FAIL]");
}

inline void print_ok()
{
		gotox(74); cprintf("[ OK ]");
}


int main(int argc, char *argv[])
{
	bgcolor(COLOR_BLACK);
	set_text80();
	charsetlc();

	unsigned int total_free = get_free_heap();
	cprintf("%d bytes of heap free, press any key to start malloc tests\n\n", total_free); cgetc();
	randomize();

	cputs("Testing malloc and free");
	(test_malloc())?print_fail():print_ok();

	cputs("Testing calloc");
	(test_calloc())?print_fail():print_ok();

	cputs("Testing realloc");
	(test_realloc())?print_fail():print_ok();

	cputs("\n");
	malloc_debug_print();		
	cputs("\n* done *\n");

	while(1)
	{
	}

	return 0;
}
