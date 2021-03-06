#ifndef AST_COMP_H
#define AST_COMP_H

#include "../LexemAnalyzer/lex_anal.h"

extern Tree_t *ttree;

// int SyntaxError   (Tokens_t *tokens, size_t index);
int FillTree         (Tree_t *tree,     Tokens_t *tokens);
int ConnectNodes     (Node_t *parent,   Node_t *node, ChildNumeration child);
int InitEmptyNode    (Node_t  **node);
int PrintTokens      (Tokens_t *tokens);
int PrintToken       (Tokens_t *tokens, size_t  index);
int InitKeyword      (Node_t  **node,                  const int keyword);
int Require          (Tokens_t *tokens, size_t *index, const int  symbol);
Node_t *GetToken     (Tokens_t *tokens, size_t *index);
Node_t *GetVar       (Tokens_t *tokens, size_t *index);
Node_t *GetFuncDef   (Tokens_t *tokens, size_t *index);
Node_t *GetFuncParams(Tokens_t *tokens, size_t *index);
Node_t *GetCallParams(Tokens_t *tokens, size_t *index);
Node_t *GetIdentifier(Tokens_t *tokens, size_t *index);
Node_t *GetG         (Tokens_t *tokens, size_t *index);
Node_t *GetGS        (Tokens_t *tokens, size_t *index);
Node_t *GetE         (Tokens_t *tokens, size_t *index);
Node_t *GetT         (Tokens_t *tokens, size_t *index);
Node_t *GetP         (Tokens_t *tokens, size_t *index);
Node_t *GetW         (Tokens_t *tokens, size_t *index);
Node_t *GetF         (Tokens_t *tokens, size_t *index);
Node_t *GetV         (Tokens_t *tokens, size_t *index);
Node_t *GetN         (Tokens_t *tokens, size_t *index);
Node_t *GetStmt      (Tokens_t *tokens, size_t *index);
Node_t *GetStmts     (Tokens_t *tokens, size_t *index);
Node_t *GetFuncParams(Tokens_t *tokens, size_t *index);
Node_t *GetCallParams(Tokens_t *tokens, size_t *index);
int PoopTree(const char *filename, Tree_t *tree);

namespace ASTree
{
    enum Errors
    {
        FUNC_IS_OK              = 0,
        BAD_ALLOC               = 0xBADA110C,
         
        PTR_IS_NULL             = 0x700,
        INVALID_SYNTAX          = 0x701,
        TOKEN_IS_NULL           = 0x702,
        DESTPTR_ISNT_NULL       = 0x703,
        NODE_IS_NULL            = 0x704,
        CHILD_IS_INVALID        = 0x705,
        INVALID_CONNECTION_PAR  = 0x706,
        INVALID_CONNECTION_NODE = 0x707,
        STREAM_IS_NULL          = 0x708,
        DEAD_TOKEN_TYPE         = 0x709,
        WRONG_KEYWORD           = 0x70A,

    };

}

#define TOKEN_KEYW(__KEYW__)  (token->type == KEYW_TYPE && token->arg.key_w == (__KEYW__))

#define KEYW token->arg.key_w

#define TOKEN                 \
    PRINT_S("TOKEN");          \
    PrintToken(tokens, *index); \

#define CATCH_ERR                 \
do                                 \
{                                   \
    if (status)                      \
    {                                 \
        PRINT_X(status);               \
        return 0;                       \
    }                                    \
}                                         \
while (0)

#define CATCH_NULL(PTR)                      \
do                                            \
{                                              \
    if (PTR == nullptr)                         \
    {                                            \
        PRINT_PTR(PTR);                           \
        return nullptr;                            \
    }                                               \
} while (0)

#define GET_NEXT_TOKEN                                 \
do                                                      \
{                                                        \
    status = TokensElem(tokens, ++(*index), &token);      \
    CATCH_ERR;                                             \
}                                                           \
while (0)

#define TDUMP(NODE)                                            \
    ttree->root = NODE;                                         \
    TreeDump(ttree);

#endif // AST_COMP_H