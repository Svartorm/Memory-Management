#include <stddef.h>

void *page_begin(void *ptr, size_t page_size)
{
    size_t begin = (size_t)ptr;
    size_t mask = ~(page_size - 1);
    size_t end = begin - (begin & mask);
    char *begin_ptr = ptr;
    return begin_ptr - end;
}
