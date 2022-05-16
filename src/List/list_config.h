#ifndef LIST_CONFIG_H
#define LIST_CONFIG_H

#include "../Tokens/tokens.h"

struct Name_t
{
    int type; // (Variable) / (Dest or Source)

    union
    {
        struct
        {
            int    offset;
            char   name[WORD_MAX_LEN + 1];
        }; // Variable

        struct
        {
            size_t code_ptr;
            char  *mark;
        }; // Label
    } elem;
};

enum NameID
{
    VariableType = 0x777,

    DestinLabel  = 0x999,
    SourceLabel  = 0x333,
};

typedef Name_t Val_t;

struct List_t;

/* size_t ListValueIndex(List_t *list, const char *str); */

const Val_t EMPTY_VALUE = {};
const Val_t DEAD_VALUE  = {987654321};
//    ^ MODIFIABLE TYPE  ^ MODIFIABLE VALUE

#endif
