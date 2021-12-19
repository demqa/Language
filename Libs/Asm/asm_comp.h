#ifndef ASM_COMP_H
#define ASM_COMP_H

#include "../LexemAnalyzer/lex_anal.h"
#include <math.h>

namespace ASMcmp
{
    enum Errors
    {
        FUNC_IS_OK              = 0,
        BAD_ALLOC               = 0xBADA110C,

        PTR_IS_NULL             = 0x800,
        NODE_IS_NULL            = 0x801,
        STREAM_IS_NULL          = 0x802,
        FILENAME_IS_NULL        = 0x803,
        DEST_PTR_ISNT_NULL      = 0x804,
        TREE_ROOT_ISNT_NULL     = 0x805,
        NODE_HAS_CHILDREN       = 0x806,
        INVALID_BRACKETS        = 0x807,
        NO_CHILD                = 0x808,
        EMPTY_VALUE             = 0x809,
        NODE_VALUE_ISNT_NULL    = 0x80A,
        WRONG_VALUE_FORMAT      = 0x80B,
        TOKEN_IS_NULL           = 0x80C,
        UNDEFINED_KEYWORD       = 0x80D,
        WORD_MAX_LEN_REACHED    = 0x80E,
        WRONG_IDENTIFIER_FMT    = 0x80F,
        WRONG_NUM_FMT           = 0x810,
        TREE_FORMAT_ISNT_RIGHT  = 0x811,
        
    };
}

int InitEmptyNode(Node_t **node_);
int ReadBuffer(char **buffer, FILE *stream);
int FillNodes(char **ptr, Node_t *node);
int FillTree(const char *filename, Tree_t *tree);
int GenerateASM(const char *filename, Tree_t *tree);

#define CATCH_ERR    \
do                    \
{                      \
    if (status)         \
    {                    \
        PRINT_X(status);  \
        return status;     \
    }                       \
}                            \
while (0)
    

#endif // ASM_COMP_H
