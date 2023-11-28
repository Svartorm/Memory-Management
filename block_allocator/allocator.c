#include "allocator.h"

#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

struct blk_allocator *blka_new(void)
{
    struct blk_allocator *blka = malloc(sizeof(struct blk_allocator));
    blka->meta = NULL;
    return blka;
}

void blka_delete(struct blk_allocator *blka)
{
    while (blka->meta != NULL)
        blka_pop(blka);
    free(blka);
}

size_t align(size_t size, size_t align)
{
    size_t res;
    size = size == 0 ? 1 : size;

    //? Check for overflow
    if (__builtin_add_overflow(size, (align - 1), &res))
        return 0;

    return (size + (align - 1)) & ~(align - 1);
}

struct blk_meta *blka_alloc(struct blk_allocator *blka, size_t size)
{
    size_t block_size = align(size, sysconf(_SC_PAGESIZE));

    struct blk_meta *new = mmap(NULL, block_size, PROT_READ | PROT_WRITE,
                                MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (new == MAP_FAILED)
        return NULL;

    new->size = block_size - sizeof(struct blk_meta);
    new->next = blka->meta;
    blka->meta = new;

    return new;
}

void blka_free(struct blk_meta *blk)
{
    size_t block_size = blk->size + sizeof(struct blk_meta);
    munmap(blk, block_size);
}

void blka_pop(struct blk_allocator *blka)
{
    struct blk_meta *qwq = blka->meta;
    if (qwq == NULL)
        return;

    blka->meta = blka->meta->next;
    blka_free(qwq);
}
