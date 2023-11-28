#include "recycler.h"

#include <stdio.h>
#include <stdlib.h>
struct recycler *recycler_create(size_t block_size, size_t total_size)
{
    if (block_size == 0 || total_size == 0 || total_size % block_size != 0
        || block_size % sizeof(size_t) != 0)
        return NULL;

    struct recycler *recycler = malloc(sizeof(struct recycler));
    if (recycler == NULL)
        return NULL;

    recycler->block_size = block_size;
    recycler->capacity = total_size / block_size;
    recycler->chunk = malloc(total_size);
    if (recycler->chunk == NULL)
    {
        free(recycler);
        return NULL;
    }

    recycler->free = recycler->chunk;
    struct free_list *head = recycler->free;
    for (size_t i = 0; i < recycler->capacity - 1; i++)
    {
        struct free_list *n = head + block_size / sizeof(struct free_list);
        head->next = n;
        head = head->next;
    }
    head->next = NULL;

    return recycler;
}

void recycler_destroy(struct recycler *r)
{
    if (r == NULL)
        return;

    free(r->chunk);
    free(r);
}

void *recycler_allocate(struct recycler *r)
{
    if (r == NULL)
        return NULL;

    if (r->free == NULL)
        return NULL;

    struct free_list *head = r->free;
    r->free = head->next;
    return head;
}

void recycler_free(struct recycler *r, void *block)
{
    if (r == NULL || block == NULL)
        return;

    struct free_list *head = block;
    head->next = r->free;
    r->free = head;
}
