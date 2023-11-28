#include "alignment.h"

size_t align(size_t size)
{
    size_t align = sizeof(long double);
    size_t res;

    //? Check for overflow
    if (__builtin_add_overflow(size, (align - 1), &res))
        return 0;

    return (size + (align - 1)) & ~(align - 1);
}
