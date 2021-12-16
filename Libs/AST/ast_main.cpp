#include "ast_comp.h"

int main()
{
    Tokens_t *tokens = (Tokens_t *) calloc(1, sizeof(Tokens_t));

    TokensCtor(tokens);

    GetTokens("Code/code", tokens);

    PRINT_UL(tokens->size);

    Token_t *token = nullptr;

for (size_t x = 0; x < tokens->size; x++)
{
    token = nullptr;
    int status = TokensElem(tokens, x, &token);
    if (status) return status;

    if (token == nullptr)
    {
        fprintf(stderr, "empty\n");
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
        #define DEF_KEYWORD(DEF, CODE, WORD)    \
            case KEYW_ ## DEF:                   \
                fprintf(stderr, "%s\n", #DEF);    \
                break;                             \

        #define DEF_OPER(DEF, CODE, SIGN)            \
            case OPER_ ## DEF:                        \
                fprintf(stderr, "%s\n", #SIGN);        \
                break;

        #define DEF_BRAC(DEF, CODE, BRAC)                 \
            case BRAC_ ## DEF:                             \
                fprintf(stderr, "%c\n", BRAC);              \
                break;
        
        switch (token->arg.key_w)
        {
            #include "../keywords"
            #include "../operators"

            default:
                break;
        }

        #undef DEF_KEYWORD
        #undef DEF_OPER
        #undef DEF_BRAC
    }
}

    TokensDtor(tokens);

    free(tokens);

    return 0;
}
