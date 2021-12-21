#include "ast_to_lang.h"

int FillTree(const char *filename, Tree_t *tree);
int PoopLang(const char *filename, Tree_t *tree);

int main()
{
    Tree_t *tree = (Tree_t *) calloc(1, sizeof(Tree_t));

    TreeCtor(tree);

    PRINT_LINE;

    FillTree("Code/asm", tree);

    PRINT_LINE;

    PoopLang("Code/lang", tree);

    PRINT_LINE;
    
    TreeDtor(tree);

    PRINT_LINE;

    free(tree);

    return 0;
}