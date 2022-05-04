#ifndef LIST_CONFIG_H
#define LIST_CONFIG_H

#include "../Tokens/tokens.h"

struct Name_t
{
    size_t index;
    char    name[WORD_MAX_LEN + 1];
};

typedef Name_t Val_t;

struct List_t;

size_t ListValueIndex(List_t *list, const char *str);

const Val_t EMPTY_VALUE = {};
const Val_t DEAD_VALUE  = {987654321, 123};
//    ^ MODIFIABLE TYPE  ^ MODIFIABLE VALUE

#endif