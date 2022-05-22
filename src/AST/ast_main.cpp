#include "ast_comp.h"

int main()
{
    Tokens_t *tokens = (Tokens_t *) calloc(1, sizeof(Tokens_t));
    if (tokens == nullptr) return EXIT_FAILURE;

    TokensCtor(tokens);

    GetTokens("../code/code", tokens);

    // PrintTokens(tokens);

    Tree_t *tree = (Tree_t *)       calloc(1, sizeof(Tree_t));
    if (tree == nullptr)   return EXIT_FAILURE;
    TreeCtor(tree);

    FillTree(tree, tokens);

    // TreeDump(tree);

    PoopTree("../code/ast", tree);

    TokensDtor(tokens);
    TreeDtor(tree);

    free(tokens);
    free(tree);

    return 0;
}
