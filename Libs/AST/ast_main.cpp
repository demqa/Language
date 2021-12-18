#include "ast_comp.h"

int main()
{
    Tokens_t *tokens = (Tokens_t *) calloc(1, sizeof(Tokens_t));
    if (tokens == nullptr) return 101010101;

    TokensCtor(tokens);

    GetTokens("Code/code", tokens);
    
    PrintTokens(tokens);

    Tree_t *tree = (Tree_t *)       calloc(1, sizeof(Tree_t));
    if (tree == nullptr)   return 101010101;
    TreeCtor(tree);

    FillTree(tree, tokens);

    TreeDump(tree);

    TokensDtor(tokens);
    TreeDtor(tree);

    free(tokens);
    free(tree);

    return 0;
}

int PrintTokens(Tokens_t *tokens)
{
    int status = TokensVerify(tokens);
    if (status) return status;

    for (size_t index = 0; index < tokens->size; index++)
        PrintToken(tokens, index);
    
    return status;
}

int PrintToken(Tokens_t *tokens, size_t index)
{
    int status = TokensVerify(tokens);
    if (status) return status;

    Token_t *token = nullptr;

    status = TokensElem(tokens, index, &token);
    if (status) return status;

    if (token == nullptr)
    {
        fprintf(stderr, "token is empty(nullptr)\n");
    }
    else
    if (token->type == ID_TYPE)
    {
        if (token->arg.id == nullptr)
            fprintf(stderr, "dead\n");
        else
            fprintf(stderr, "\'%s\'\n", token->arg.id);
    }
    else
    if (token->type == NUM_TYPE)
    {
        fprintf(stderr, "%lg\n", token->arg.num);
    }
    else
    if (token->type == KEYW_TYPE)
    {
        #define DEF_KEYW(DEF, CODE, WORD)       \
            case KEYW_ ## DEF:                   \
                fprintf(stderr, "%s\n", #DEF);    \
                break;                             \

        #define DEF_OPER(DEF, CODE, SIGN)            \
            case KEYW_ ## DEF:                        \
                fprintf(stderr, "%s\n", #SIGN);        \
                break;

        #define DEF_HELP(DEF, CODE, HELP)                 \
            case KEYW_ ## DEF:                             \
                fprintf(stderr, "%c\n", HELP);              \
                break;
        
        switch (token->arg.key_w)
        {
            #include "../keywords"
            #include "../operators"

            default:
                fprintf(stderr, "dead\n");
                break;
        }

        #undef DEF_KEYW
        #undef DEF_OPER
        #undef DEF_HELP
    }
}
