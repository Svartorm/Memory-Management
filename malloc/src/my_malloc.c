#include "malloc.h"

// Global variables
static struct block *free_list = NULL;

/*--------- Utils ---------*/
// Aligns the given size to the given alignment
size_t align(size_t size, size_t align)
{
    size_t res;
    size = size == 0 ? 1 : size;

    //? Check for overflow
    if (__builtin_add_overflow(size, (align - 1), &res))
        return 0;

    return (size + (align - 1)) & ~(align - 1);
}

/*--------- Block allocator ---------*/
// Allocates a new block of memory
static struct block *balloc(size_t size)
{
    size_t block_size = align(size, PAGE_SIZE);

    struct block *new = mmap(NULL, block_size, PROT_READ | PROT_WRITE,
                             MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (new == MAP_FAILED)
        return NULL;

    size_t header_size = align(sizeof(struct block), MALLOC_ALIGN);

    new->page_size = block_size;

    new->size = block_size - header_size;
    new->free = 1;

    new->next = NULL;
    new->prev = NULL;
    new->next_free = NULL;
    new->data = (char *)new + header_size;

    // printf("Allocated %zu bytes\n", block_size);

    return new;
}

// Returns the block that contains the given pointer
static void *block_begin(void *data)
{
    return (char *)data - align(sizeof(struct block), MALLOC_ALIGN);
}

/*
// Print the block
static void print_block(struct block *blk)
{
    printf("\033[0;33m---\n");
    printf("Block: %p\n", (void *)blk);
    printf("Size: %zu\n", blk->size);
    printf("Free: %s\n", blk->free == 1 ? "Available" : "Occupied");
    printf("Next: %p\n", (void *)blk->next);
    printf("Prev: %p\n", (void *)blk->prev);
    printf("Next free: %p\n", (void *)blk->next_free);
    printf("data: %p\n", (void *)blk->data);
    printf("\033[0m\n");
}

// Print the Free List
void print_free_list(void)
{
    printf("Free List:\n");
    struct block *blk = free_list;
    while (blk != NULL)
    {
        print_block(blk);
        blk = blk->next_free;
    }
}
*/

// Print the heap
void print_heap(struct block *blk)
{
    // Get the start of the heap
    struct block *start = blk;
    while (start->prev != NULL)
        start = start->prev;

    // Print the heap
    while (start != NULL)
    {
        print_block(start);
        start = start->next;
    }
}

/*--------- Free List ---------*/
void add_to_free_list(struct block *blk)
{
    blk->next_free = free_list;
    blk->free = 1;
    free_list = blk;
}

void remove_from_free_list(struct block *blk)
{
    struct block *curr = free_list;
    struct block *prev = NULL;
    while (curr != NULL && curr != blk)
    {
        prev = curr;
        curr = curr->next_free;
    }

    if (curr == NULL)
        return;

    if (prev == NULL)
        free_list = curr->next_free;
    else
        prev->next_free = curr->next_free;

    blk->free = 0;
}

void heap_init(void)
{
    //? Allocate a new block
    struct block *blk = balloc(1);
    if (blk == NULL)
        return;

    //? Set the free list
    add_to_free_list(blk);
}

/*
**  Merge the given block with all the previous free blocks
**  until it reaches an occupied block
**  Then merge the given block with all the next free blocks
**  until it reaches an occupied block
**  @param blk The block to merge
*/
void merge(struct block *blk)
{
    //? Merge with the previous blocks
    struct block *curr = blk->prev;
    while (curr != NULL && curr->free == 1)
    {
        curr->size += blk->size + sizeof(struct block);
        curr->next = blk->next;

        blk = curr;
        curr = curr->prev;
    }

    //? Merge with the next blocks
    curr = blk->next;
    while (curr != NULL && curr->free == 1)
    {
        blk->size += curr->size + sizeof(struct block);
        blk->next = curr->next;

        curr = curr->next;
    }
}

void merge_with_next(struct block *blk)
{
    if (blk->next == NULL)
        return;

    remove_from_free_list(blk->next);

    blk->size += blk->next->size + align(sizeof(struct block), MALLOC_ALIGN);
    blk->next = blk->next->next;
}

/*--------- My malloc ---------*/

void *my_malloc(size_t size)
{
    //? If the size is 0, return NULL
    if (size == 0)
        return NULL;

    //? Align the size
    size_t aligned_size = align(size, MALLOC_ALIGN);

    //? Find a free block that is big enough
    struct block *blk = free_list;
    while (blk != NULL && blk->size < aligned_size)
        blk = blk->next;

    //? If no block was found, allocate a new one
    if (blk == NULL)
    {
        blk = balloc(aligned_size);
        if (blk == NULL)
            return NULL;

        add_to_free_list(blk);
    }

    //? If the block is too big, split it
    if (blk->size > aligned_size)
    {
        struct block *new = (void *)((char *)blk->data + aligned_size);

        new->page_size = blk->page_size;

        size_t header_size = align(sizeof(struct block), MALLOC_ALIGN);

        new->size = blk->size - aligned_size - header_size;
        new->free = 1;

        new->next = blk->next;
        new->prev = blk;
        new->data = (char *)new + header_size;

        blk->size = aligned_size;
        blk->next = new;

        //? Add the new block to the free list
        add_to_free_list(new);
    }

    //? Remove the block from the free list
    remove_from_free_list(blk);

    //? Return the pointer to the allocated memory
    return blk->data;
}

void my_free(void *data)
{
    //? If the pointer is NULL, return
    if (data == NULL)
        return;

    //? Find the block that contains the pointer
    struct block *blk = block_begin(data);

    //? Free the block
    add_to_free_list(blk);

    //? Merge the block with the previous and next blocks
    merge(blk);

    //? Free the block if it is the last one on the page
    if (blk->next == NULL && blk->prev == NULL)
    {
        remove_from_free_list(blk);
        munmap(blk, blk->page_size);
    }
}

void *my_realloc(void *data, size_t size)
{
    if (data == NULL)
        return my_malloc(size);

    if (size == 0)
    {
        my_free(data);
        return NULL;
    }

    struct block *blk = block_begin(data);

    size_t realigned_size = align(size, MALLOC_ALIGN);

    if (blk->next && blk->next->free == 1)
    {
        size_t merged_size = blk->size + blk->next->size
            + align(sizeof(struct block), MALLOC_ALIGN);

        // Check if next block is free and large enough
        if (merged_size >= realigned_size)
        {
            merge_with_next(blk);
            return data;
        }
    }

    // Allocate a new block
    void *new_data = my_malloc(size);
    if (new_data == NULL)
        return NULL;

    // Copy the data
    memcpy(new_data, data, blk->size);

    // Free the old block
    my_free(data);

    // Return the pointer to the new block
    return new_data;
}

void *my_calloc(size_t nmemb, size_t size)
{
    //? check for overflow
    if (__builtin_umull_overflow(nmemb, size, &size))
        return NULL;

    //? Allocate a new block
    void *data = my_malloc(nmemb * size);
    if (data == NULL)
        return NULL;

    //? Set the allocated memory to 0
    memset(data, 0, nmemb * size);

    //? Return the pointer to the allocated memory
    return data;
}
