#include "asm_comp.h"

int main()
{
    Tree_t *tree = (Tree_t *) calloc(1, sizeof(Tree_t));
    if (tree == nullptr) return 101010101;

    int status = TreeCtor(tree);
    if (status) PRINT_X(status);

    status = FillTree("Code/ast", tree);
    if (status)
    {
        PRINT_D(status);
        PRINT_X(status);
    }

    status = GenerateASM("Code/asm", tree);
    if (status)
    {
        PRINT_D(status);
        PRINT_X(status);
    }

    TreeDump(tree);
    TreeDtor(tree);

    free(tree);

    return 0;
}
