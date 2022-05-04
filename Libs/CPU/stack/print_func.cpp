#include <stdio.h>

void PrintDouble(void *memory, size_t size, FILE *stream){
    fprintf(stream, "{%f}\n", *((double *)memory));
}