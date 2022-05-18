#include "asm_comp.h"
#include "../ELF/elf_gen.h"

int main()
{
    Tree_t *tree = (Tree_t *) calloc(1, sizeof(Tree_t));
    if (tree == nullptr) return 101010101;

    int status = TreeCtor(tree);
    if (status) PRINT_X(status);

    status = FillTree("../code/ast", tree);
    if (status)
    {
        PRINT_D(status);
        PRINT_X(status);
    }

    // TreeDump(tree);

    status = EmitASM("../code/asm_log", tree);
    if (status)
    {
        PRINT_D(status);
        PRINT_X(status);
    }

    TreeDtor(tree);

    free(tree);

    return 0;
}
