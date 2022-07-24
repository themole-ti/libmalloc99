#ifndef MALLOC_H
#define MALLOC_H

#undef DEBUG_MALLOC
#define null 0

void*			malloc(unsigned int size);
void* 			calloc(unsigned int count, unsigned int size);
void*  			realloc(void* source, unsigned int size);
void 			free(void*);
void			malloc_debug_print();
unsigned int 	get_free_heap();

#endif