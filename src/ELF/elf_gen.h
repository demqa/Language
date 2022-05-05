#ifndef ELF_GEN_H_
#define ELF_GEN_H_

#include <stddef.h>

struct Buff
{
    char *buffer;
    size_t size;
};

int GenerateElf(const char *filename, Buff *buff);

#endif // ELF_GEN_H_
