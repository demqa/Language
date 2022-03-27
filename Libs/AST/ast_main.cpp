#include "ast_comp.h"

int main()
{
    ttree = (Tree_t *) calloc(1, sizeof(Tree_t));
    TreeCtor(ttree);

    Tokens_t *tokens = (Tokens_t *) calloc(1, sizeof(Tokens_t));
    if (tokens == nullptr) return 101010101;

    TokensCtor(tokens);

    GetTokens("Code/code", tokens);

    PrintTokens(tokens);

    Tree_t *tree = (Tree_t *)       calloc(1, sizeof(Tree_t));
    if (tree == nullptr)   return 101010101;
    TreeCtor(tree);

    PRINT_LINE;

    FillTree(tree, tokens);

    PRINT_LINE;

    PoopTree("Code/ast", tree);

    PRINT_LINE;

    TreeDump(tree);

    TokensDtor(tokens);
    TreeDtor(tree);

    free(tokens);
    free(tree);

    free(ttree);

    return 0;
}
