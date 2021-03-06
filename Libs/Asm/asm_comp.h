#ifndef ASM_COMP_H
#define ASM_COMP_H

#include "../LexemAnalyzer/lex_anal.h"
#include "../List/list.h"
#include <math.h>

namespace ASMcmp
{
    enum Errors
    {   
        FUNC_IS_OK                 = 0,
        BAD_ALLOC                  = 0xBADA110C,

        PTR_IS_NULL                = 0x800,
        NODE_IS_NULL               = 0x801,
        STREAM_IS_NULL             = 0x802,
        FILENAME_IS_NULL           = 0x803,
        DEST_PTR_ISNT_NULL         = 0x804,
        TREE_ROOT_ISNT_NULL        = 0x805,
        NODE_HAS_CHILDREN          = 0x806,
        INVALID_BRACKETS           = 0x807,
        NO_CHILD                   = 0x808,
        EMPTY_VALUE                = 0x809,
        NODE_VALUE_ISNT_NULL       = 0x80A,
        WRONG_VALUE_FORMAT         = 0x80B,
        TOKEN_IS_NULL              = 0x80C,
        UNDEFINED_KEYWORD          = 0x80D,
        WORD_MAX_LEN_REACHED       = 0x80E,
        WRONG_IDENTIFIER_FMT       = 0x80F,
        WRONG_NUM_FMT              = 0x810,
        TREE_FORMAT_ISNT_RIGHT     = 0x811,
        GLOBAL_STMTS_ARE_NOT_STMTS = 0x812,
        STMT_HASNT_GOT_STMT        = 0x813,
        REPEATING_VARIABLE         = 0x814,
        VARIABLE_FOUND             = 0x815,
        INVALID_GLOBAL_STMT        = 0x816,
        OUT_STREAM_IS_NULL         = 0x817,
        ANS_IS_NULL                = 0x818,
        INVALID_STMT               = 0x819,
        CANT_FIND_NON_ID_IN_NT     = 0x81A,
        VARIABLE_NOT_FOUND         = 0x81B,
        CANT_USE_FUNC_IN_GLOBAL    = 0x81C,
        UNDEFINED_OPERATOR         = 0x81D,
        REPEATING_MAIN             = 0x81E,
        CANT_USE_NON_ID_LIKE_MARK  = 0x81F,
        NAMETABLE_ISNT_CLEAR       = 0x820,
        VARIABLE_IS_ENGAGED        = 0x821,
        NO_MAIN_IN_PROGRAM         = 0x822,
        UNEXPECTED_MATH_OPER       = 0x823,
        PRINT_DOESNT_HAVE_ARG      = 0x824,
        NOT_LOG_OPER               = 0x825,
        PARAM_ISNT_PARAM           = 0x826,
        PARAM_ISNT_THE_ONLY        = 0x827,
        NO_PARAMS                  = 0x828,
        RETURN_ISNT_RETURN         = 0x829,


    };

    const size_t INITIAL_CAPACITY = 16;
}

int InitEmptyNode(Node_t **node_);
int ReadBuffer   (char **buffer, FILE *stream);
int FillNodes    (char **ptr, Node_t *node);
int FillTree     (const char *filename, Tree_t *tree);
int GenerateASM  (const char *filename, Tree_t *tree);



int IsNum             (Node_t *node, int *ans);
int IsVar             (Node_t *node, int *ans);
int IsMathOper        (Node_t *node, int *ans);
int IsLogOper         (Node_t *node, int *ans);

int IncreaseBX        (const size_t number);
int DecreaseBX        (const size_t number);

int GenerateMark      (Node_t *mark);
int GenerateMain      (Node_t *node, List_t *NT, List_t *GlobalNT);
int GenerateFuncDef   (Node_t *node, List_t *NT, List_t *GlobalNT);
int GenerateDefParams (Node_t *node, List_t *NT, List_t *GlobalNT, size_t *free_memory_index);

int EvalVar           (Node_t *node, List_t *NT, List_t *GlobalNT);
int InitVar           (Node_t *node, List_t *NT, List_t *GlobalNT);
int GenerateVar       (Node_t *node, List_t *NT, List_t *GlobalNT);
int GenerateExpr      (Node_t *node, List_t *NT, List_t *GlobalNT);
int InitGlobVar       (Node_t *node, List_t *GlobalNT);
int GenerateGlobExpr  (Node_t *node, List_t *GlobalNT);
int GenerateGlobVar   (Node_t *node, List_t *GlobalNT);

int GenerateMathOper  (Node_t *node);
int GenerateNum       (Node_t *node);
int InitCallParams(Node_t *node, List_t *NT, List_t *GlobalNT, size_t *num_of_params);
int GenerateCall      (Node_t *node, List_t *NT, List_t *GlobalNT);
int GenerateJump      (Node_t *node, List_t *NT, List_t *GlobalNT, const char *mark, const int num);
int GenerateCond      (Node_t *node, List_t *NT, List_t *GlobalNT, const char *mark, const int num);
int GenerateIf        (Node_t *node, List_t *NT, List_t *GlobalNT);
int GenerateReturn    (Node_t *node, List_t *NT, List_t *GlobalNT);
int GenerateWhile     (Node_t *node, List_t *NT, List_t *GlobalNT);
int GenerateScan      (Node_t *node, List_t *NT, List_t *GlobalNT);
int GeneratePrint     (Node_t *node, List_t *NT, List_t *GlobalNT);
int GenerateAssign    (Node_t *node, List_t *NT, List_t *GlobalNT);

int GenerateStmt      (Node_t *node, List_t *NT, List_t *GlobalNT);
int GenerateStmts     (Node_t *node, List_t *NT, List_t *GlobalNT);
int GenerateGS        (Node_t *node, List_t *GlobalNT);
int GenerateASM       (const char *filename, Tree_t *tree);


#ifdef DEBUG_LIB_H
#define CATCH_ERR   \
do                   \
{                     \
    PRINT_(1234_);       \
    if (status)        \
    {                   \
        PRINT(ERR);      \
        PRINT_X(status);  \
        return status;     \
    }                       \
}                            \
while (0)
#else
#define CATCH_ERR               \
do                               \
{                                 \
    if (status) return status;     \
}                                   \
while (0)
#endif

#ifdef DEBUG_LIB_H
#define CATCH_NULL(PTR)                  \
do                                        \
{                                          \
    if (PTR == nullptr)                     \
    {                                        \
        PRINT_PTR(PTR);                       \
        return ASMcmp::PTR_IS_NULL;            \
    }                                           \
}                                                \
while (0)
#else
#define CATCH_NULL(PTR)                             \
do                                                   \
{                                                     \
    if (PTR == nullptr) return ASMcmp::PTR_IS_NULL;    \
}                                                       \
while (0)
#endif

#endif // ASM_COMP_H
