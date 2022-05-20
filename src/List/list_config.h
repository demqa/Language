#ifndef LIST_CONFIG_H
#define LIST_CONFIG_H

#include "../Tokens/tokens.h"

struct Name_t
{
    int     type;   // Variable / Keyword / Function
    int64_t offset; // This cound be either offset in stack (that's why it is signed)
                    //               either offset in buffer
    // TODO:
    // Change char * with hash, because it will be
    // more effective. BUT. I cannot write this
    // for now, because I will be unable to debug
    // this version with hash normally.
    // So this is not so necessary right now.

    // to compare fast.
    int64_t hash;

    union
    {
        struct
        {
            char *variable;
        }; // Variable

        // In these two cases it proceeds equally, except of comparing...
        struct
        {
            int keyword_id; // I can have more than one
            int number;     // if/else/while etc.
                            // so I have to differ it.
        }; // Keyword

        struct
        {
            char *func_ptr;
        }; // Function
    } elem;

    char segfault_guard = 0;
};

struct Keyword
{
    int keyword_id; // I can have more than one
    int number;     // if/else/while etc.
                    // so I have to differ it.
}; // Keyword



enum NameID
{
    Variable_t = 0x7777,
     Keyword_t = 0x8888,
    Function_t = 0x9999,
};

enum KeywordID
{
    Keyword_if        = 1,
    Keyword_else      = 2,
    Keyword_if_end    = 3,

    Keyword_while     = 4,
    Keyword_while_end = 5,
};

typedef Name_t Val_t;

struct List_t;

/* size_t ListValueIndex(List_t *list, const char *str); */

const Val_t EMPTY_VALUE = {};
const Val_t DEAD_VALUE  = {987654321};
//    ^ MODIFIABLE TYPE  ^ MODIFIABLE VALUE

#endif
