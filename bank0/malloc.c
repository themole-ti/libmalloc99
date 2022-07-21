/*************************************/
/* tms9900-gcc malloc implementation */
/* 2022 - Danny Lousberg             */
/*************************************/

#include "malloc.h"
#ifdef  DEBUG_MALLOC
extern int cprintf(const char *fmt, ...);
extern void clrscr();
#endif

#define MAX_HEAP 0xfffe

extern unsigned int _bss_end;
unsigned int 		heap_free = 0;
unsigned int*  		heap_start = &_bss_end;
unsigned int*       heap_end = (unsigned int*)MAX_HEAP;
unsigned int  		heap_size;
unsigned int 		heap_initialized = 0;

#define B2W(a) ((a) >> 1)
#define W2B(a) ((a) << 1)

typedef struct
{
	unsigned int  free : 1;
	unsigned int  size : 15;
} chunk_info;

void initialize_heap()
{
	// Make sure it's zeroed out, we store metadata in unused space
	unsigned int* ptr;
	for (ptr = heap_start; ptr < heap_end; ptr++)
		*ptr = 0;

 	heap_size = (((MAX_HEAP - (unsigned int)heap_start)) << 1) >> 1;
 	heap_free = heap_size;

 	chunk_info *chunk = (void*)heap_start;
 	chunk->free = 1;
 	chunk->size = B2W(heap_free);

 	heap_initialized = 1;
}

chunk_info* get_next_chunk(chunk_info* current)
{
	return current + current->size;
}

void* malloc(unsigned int size)
{
	if (!heap_initialized)
		initialize_heap();

	// Make sure we stay aligned to word boundaries
	if (size % 2)
		size++;

	// return immediately if heap_free is smaller than requested size
	if (heap_free < (size + sizeof(chunk_info))) 
		return null;

	// iterate over the heap until we find a spot that is free and large enough
	unsigned int *ptr = heap_start;
	while (ptr < heap_end)
	{
		chunk_info *chunk = (void*)ptr;
		chunk_info *nextchunk = get_next_chunk(chunk);
		if ((chunk->free) && (chunk->size >= (size + B2W(sizeof(chunk_info)))))
		{
			// Mark changes to chunk
			unsigned int tempsize = chunk->size;
			chunk->free = 0;
			chunk->size = (size + sizeof(chunk_info)) >> 1;

			nextchunk = get_next_chunk(chunk);
			nextchunk->free = 1;
			nextchunk->size = tempsize - chunk->size;

			heap_free  -= (size + sizeof(chunk_info));

			return ptr + (sizeof(chunk_info) >> 1);
		}

		ptr = (void*)(nextchunk);
	}

	// Could not find a suitably sized chunk, return null
	return null;
}

void* calloc(unsigned int count, unsigned int size)
{
	unsigned char* tempptr = malloc(count * size);

	if (tempptr == 0)
		return null;

	// Initialize to zero
	for (unsigned int i = 0; i < (count * size); i++)
		tempptr[i] = 0;

	return tempptr;
}

void free(void* ptr)
{
	if (!heap_initialized)
		return;

	if (ptr == null)
		return;

	// Mark current chunk as free and keep a free counter
	chunk_info *chunk = ptr - sizeof(chunk_info);

	if (chunk->free != 0)
		return;

	chunk->free = 1;
	unsigned int freed = W2B(chunk->size);

	// Coalesce adjacent free chunks
	chunk = (void*)heap_start; 
	while ((unsigned int*)chunk < heap_end)
	{
		chunk_info *nextchunk = get_next_chunk(chunk);
		if ((chunk->free) && (nextchunk->free))
		{
			chunk->size += nextchunk->size;
			nextchunk->size = 0;
		}
		else
			chunk = get_next_chunk(chunk);
	}

	heap_free += freed;
	return;
}

// TODO: re-implement smarter algorithm that re-uses the existing pointer where possible
void* realloc(void* ptr, unsigned int size)
{
	if (!heap_initialized)
		return null;

	if (ptr == null)
		return null;

	// First, check if we can avoid a new malloc
	chunk_info* chunk = ptr - sizeof(chunk_info);
	chunk_info* nextchunk = get_next_chunk(chunk);
	unsigned int oldsize = W2B(chunk->size) + sizeof(chunk_info);

	if (size < oldsize)
	{
		// We can just resize the current chunk
		chunk->size = B2W(size + sizeof(chunk_info));
		nextchunk = get_next_chunk(chunk);
		nextchunk->free = 1;
		nextchunk->size = B2W((oldsize - size)) - sizeof(chunk_info);

		heap_free += W2B(nextchunk->size);

		return ptr;
	}
	if (nextchunk->free)
	{
		unsigned int oldfreesize = W2B(nextchunk->size);
		if (size < (oldsize + W2B(nextchunk->size)))
		{
			chunk->size = B2W(size + sizeof(chunk_info));
			nextchunk = get_next_chunk(chunk);
			nextchunk->free = 1;
			nextchunk->size = B2W((size - oldsize)) + sizeof(chunk_info);

			heap_free -= (oldfreesize - W2B(nextchunk->size));

			return ptr;
		}
	}

	// Can't grow or shrink in current location, let's try
	// to allocate a new location and copy stuff over
	void* newptr = malloc(size);
	if (newptr)
	{
		unsigned int* src = ptr;
		unsigned int* dst = newptr;

		for (int i = 0; i < B2W(oldsize); i++)
			dst[i] = src[i];

		free(ptr);
		return newptr;
	}

	return null;
}

unsigned int get_free_heap()
{
	if (!heap_initialized)
	{
		initialize_heap();
	}

	return heap_free;
}

void malloc_debug_print()
{
	if (!heap_initialized)
	{
		initialize_heap();
	}

#ifdef DEBUG_MALLOC
	cprintf("heap starts at 0x%x, ", heap_start);
	cprintf("%d bytes (%dkb) free\n", heap_free, heap_free >> 10 );
	cprintf("---------------------------------------\n");
	cprintf("| Chunk | Start Addr |   Size |  Free |\n");
	cprintf("---------------------------------------\n");


	// iterate over the heap and print the chunks
	unsigned int chunk_counter = 0;
	unsigned int *ptr = heap_start;
	while ((ptr < heap_end) && (chunk_counter < 15))
	{
		chunk_info *chunk = (void*)ptr; 
		cprintf(
			"|    %2d\t|    0x%4x\t |  %5d |   %s   |\n", 
			chunk_counter, 
			ptr, 
			W2B(chunk->size), 
			chunk->free ? "Y" : " "
		);

		ptr += chunk->size;
		chunk_counter++;
	}
	if (chunk_counter >= 15)
		cprintf("|   (more chunks remaining...)        |\n");
	cprintf("---------------------------------------\n");
#endif
}
