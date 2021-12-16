#ifndef TOKENS_H
#define TOKENS_H

#include <assert.h>
#include <string.h>
#include <cmath>
#include <ctype.h>
#include "tokens_config.h"

namespace Tokens
{
    enum Errors
    {   
        FUNC_IS_OK                 = 0,
        BAD_ALLOC                  = 0xBADA110C,
   
        FILENAME_IS_NULL           = 0x400,
        TOKENS_ARE_NULL            = 0x401,
        TOKENS_ARE_CLEAR           = 0x402,
        TOKENS_CANT_CONSTRUCT      = 0x403,
        SIZE_GREATER_CTY           = 0x404,
        DATA_IS_NULL               = 0x405,
        DONT_NEED_RESIZE           = 0x406,
        MIN_CTY_REACHED            = 0x407,
        REALLOC_ERROR              = 0x408,
        PUSHABLE_TOKEN_NULL        = 0x409,
        TOKEN_IS_NULL              = 0x40A,
        TOKEN_DEST_ISNT_NULL       = 0x40B,
        UNAVAILABLE_ELEM           = 0x40C,
        PTR_IS_NULL                = 0x40D,
        END_OF_FILE                = 0x40E,
        SSCANF_CANT_SCAN           = 0x40F,
        SSCANF_BAD_ALLOC           = 0x410,
        UNKNOWN_KEYWORD            = 0x411,
        UNEXPECTED_LEXEM           = 0x412,
        IDENTIFIER_MAX_LEN_REACHED = 0x413,

    };

}

const size_t WORD_MAX_LEN = 20;

struct Token_t
{
    int    type;

    union NodeType
    {
        int   key_w;
        double  num;
        char     id[WORD_MAX_LEN];
    } arg;
};

struct Tokens_t
{
    Token_t *token;

    size_t   size;
    size_t   capacity;
};

int TokensCtor    (Tokens_t *tokens);
int TokensVerify  (Tokens_t *tokens);
int TokensAreClear(Tokens_t *tokens);
int TokensDtor    (Tokens_t *tokens);
int TokensResize  (Tokens_t *tokens);
int TokensPush    (Tokens_t *tokens, const Token_t *token);
int TokensElem    (Tokens_t *tokens, size_t index, Token_t **token);

enum KeywordCode
{

#define DEF_KEYWORD(DEF, CODE, WORD) \
    KEYW_ ## DEF = CODE,
    
    #include "../keywords"

#undef DEF_KEYWORD

};

enum OperatorCode
{

#define DEF_OPER(DEF, CODE, SIGN) \
    OPER_ ## DEF = CODE,

#define DEF_BRAC(DEF, CODE, BRAC)    \
    BRAC_ ## DEF = CODE,

    #include "../operators"

#undef DEF_OPER
#undef DEF_BRAC

};

#endif // TOKENS_H
