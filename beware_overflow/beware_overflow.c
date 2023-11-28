#include "beware_overflow.h"

void *beware_overflow(void *ptr, size_t nmemb, size_t size)
{
    char *p = ptr;
    size_t res;
    if (__builtin_umull_overflow(nmemb, size, &res))
        return NULL;

    return p + res;
}
