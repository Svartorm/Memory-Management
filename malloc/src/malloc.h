#ifndef MALLOC_H
#define MALLOC_H

/***** Includes *****/
#include <stddef.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>
#include <string.h>

/***** Macros *****/
#define MALLOC_ALIGN sizeof(long double)
#define PAGE_SIZE sysconf(_SC_PAGESIZE)

/***** Structs *****/
struct block 
{
    //? Block infos
    size_t size;
    int free; //$ 1 if free, 0 if not
    
    struct block *prev;
    struct block *next;
    struct block *next_free;
    void *data;

    //? Page infos
    size_t page_size;
};

/***** Globals *****/
//extern static void *heap_start;

/***** Functions *****/
void *my_malloc(size_t size);
void my_free(void *ptr);
void *my_realloc(void *ptr, size_t size);
void *my_calloc(size_t nmemb, size_t size);

size_t align(size_t size, size_t align);

#endif /* ! MALLOC_H */
