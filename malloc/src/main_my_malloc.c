#include "malloc.h"

#include <stdio.h>

int main(void)
{
    //? This is the main function of the program
    //? It is used to test the my_malloc function

    printf("struct Block size: %zu\n", sizeof(struct block));
    printf("struct Block size aligned: %zu\n", align(sizeof(struct block), MALLOC_ALIGN));

    //? Allocate 10 bytes
    char *ptr = my_malloc(13);
    if (ptr == NULL)
        return 1;

    char *ptr2 = my_malloc(13000);
    if (ptr2 == NULL)
        return 1;

    my_free(ptr);


    return 0;
}