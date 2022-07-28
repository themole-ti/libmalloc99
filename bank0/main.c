/***************************/
/* libmalloc99 test suite  */
/* 2022 - Danny Lousberg   */
/***************************/

#include "conio.h"
#include "vdp.h"
#include "malloc.h"

#define SIZEOF_CHUNK_INFO 2
#define NUM_ITERATIONS    350
#define VINT_COUNTER 	  *((volatile unsigned char*)0x8379)

unsigned char spinner[4] = {'-', '\\', '|', '/'};

unsigned int rnd_xorshift()
{
	static unsigned int 	x = 1, y = 1;
	unsigned int			t = ( x^( x<<5 ) );

	x = y;

	return y = ( y^( y>>1) )^( t^( t>>3 ) );
}

void shuffle(unsigned int *array, unsigned int n)
{
    if (n > 1) 
    {
        unsigned int i;
        for (i = 0; i < n - 1; i++) 
        {
          unsigned int j = (i + rnd_xorshift()) % n;
          unsigned int t = array[j];
          array[j] = array[i];
          array[i] = t;
        }
    }
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
		sizes[i] = (rnd_xorshift() % (total_free / NUM_ITERATIONS));
		cputc(spinner[i % 4]); cputc('\b'); 
	}
	cprintf(".");

	// Allocate some pointers
	for (int i = 0; i < NUM_ITERATIONS; i++)
	{
		ptrs[i] = malloc(sizes[i]);
		cputc(spinner[i % 4]); cputc('\b'); 
	}
	cprintf(".");

	for (int i = 0; i < NUM_ITERATIONS; i++)
	{
		cputc(spinner[i % 4]); cputc('\b'); 
		if (ptrs[i] == null)
			return -1;
	}
	cprintf(".");

	// Try to use all of allocated memory for one pointer
	ptrs[1][0] = (unsigned int)0xDEAD;
	for (int i = 0; i < (sizes[0]>>1); i++)
		ptrs[0][i] = 0xBEEF;
	if (ptrs[1][0] != (unsigned int)0xDEAD)
		return -4;
	cprintf(".");

	// Try to allocated too much memory
	if (malloc(get_free_heap() + 20))
		return -2;
	cprintf(".");

	// test freeing
	// We shuffle the ptrs array to test coalescing
	shuffle((unsigned int*)ptrs, NUM_ITERATIONS);
	cprintf(".");
	for (int i = 0; i < NUM_ITERATIONS; i++)
	{
		cputc(spinner[i % 4]); cputc('\b'); 
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
	unsigned char* ptrs[NUM_ITERATIONS];
	unsigned int total_free = get_free_heap();
	unsigned int sizes[NUM_ITERATIONS];

	// Assign some random size values
	for (int i = 0; i < NUM_ITERATIONS; i++)
	{
		cputc(spinner[i % 4]); cputc('\b'); 
		sizes[i] = (rnd_xorshift() % (total_free / NUM_ITERATIONS));
	}
	cprintf(".");

	// Allocate some pointers
	for (int i = 0; i < NUM_ITERATIONS; i++)
	{
		cputc(spinner[i % 4]); cputc('\b'); 
		ptrs[i] = calloc(sizes[i], sizeof(unsigned char));
	}
	cprintf(".");

	for (int i = 0; i < NUM_ITERATIONS; i++)
	{
		cputc(spinner[i % 4]); cputc('\b'); 
		if (ptrs[i] == null)
			return -1;
	}
	cprintf(".");

	// Check if everything was zeroed out correctly
	for (int i = 0; i < NUM_ITERATIONS; i++)
	{
		cputc(spinner[i % 4]); cputc('\b'); 
		for (int j = 0; j < sizes[i]; j++)
		{
			if (ptrs[i][j])
			{
				return -2;
			}
		}
	}
	cprintf(".");

	// Try to allocated too much memory
	if (calloc(get_free_heap() + 20, sizeof(unsigned char)))
		return -2;
	cprintf(".");

	// test freeing
	// We shuffle the ptrs array to test coalescing
	shuffle((unsigned int*)ptrs, NUM_ITERATIONS);
	cprintf(".");
	for (int i = 0; i < NUM_ITERATIONS; i++)
	{
		cputc(spinner[i % 4]); cputc('\b'); 
		free(ptrs[i]);
	}
	cprintf(".");

	// Check for leaked memory
	if (total_free != get_free_heap())
		return -3;

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
