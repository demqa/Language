#ifndef LEX_ANAL_H
#define LEX_ANAL_H

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>

#include "../Tokens/tokens.h"
#include "../BinaryTree/bin_tree.h"

namespace Text
{
    enum Errors
    {
        FUNC_IS_OK            = 0,
        BAD_ALLOC             = 0xBADA110C,

        STREAM_IS_NULL        = 0x300,
        BUFFER_IS_NULL        = 0x301,
        PTR_IS_NOT_NULL       = 0x302,
        READ_WAS_UNSUCCESSFUL = 0x303,
        FILENAME_IS_NULL      = 0x304,
        CANT_OPEN_FILE        = 0x305,
        DEST_PTR_IS_NULL      = 0x306,

    };

    enum StringEquality
    {
        EQUAL     = 1,
        NOT_EQUAL = 0,
    };
}


int GetTokens(const char *filename, Tokens_t *tokens);
int StrEqual (const char *l, const char *r);

#endif // LEX_ANAL_H
