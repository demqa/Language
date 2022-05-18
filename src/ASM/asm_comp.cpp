#include "asm_comp.h"

// rework all the backend I have there
// there is so much issues.
// I thought that these scopes that I did there with my NameTable is nice, but this is real BULLSHIT,
// because it does not work. So next thing is that I can use some struct Backend to deal with different
// functions, but I have not done it. So that why code in this file is shit, despite the beauty.

#define NODE_KEYW(NODE, KEYW) (NODE->value->type == KEYW_TYPE && NODE->value->arg.key_w == KEYW)
#define NODE_ID(NODE) (NODE->value->type == ID_TYPE)
#define KEYW(NODE) ((NODE->value->type == KEYW_TYPE) ? NODE->value->arg.key_w : 0)

#define CHECK_NODES do { int status = NodeVerify(node); CATCH_ERR; } while (0)

// DONE
int GetChild(char **ptr, Node_t **node)
{
    if ( node == nullptr)                     return ASMcmp::NODE_IS_NULL;
    if (*node != nullptr)                     return ASMcmp::DEST_PTR_ISNT_NULL;
    if ( ptr  == nullptr || *ptr  == nullptr) return ASMcmp::PTR_IS_NULL;

    if (**ptr == '{')
    {
        (*ptr)++;

        int status = InitEmptyNode(node);
        if (status) return status;

        status = FillNodes(ptr, *node);
        if (status) return status;

        if (**ptr != '}') return ASMcmp::INVALID_BRACKETS;

        (*ptr)++;

        return ASMcmp::FUNC_IS_OK;
    }

    return ASMcmp::NO_CHILD;
}

// DONE
// TODO
int CreateKeyword(Token_t **token, const int keyword)
{
    if (token == nullptr || *token == nullptr) return ASMcmp::TOKEN_IS_NULL;

    int status = ASMcmp::FUNC_IS_OK;

    // TODO: check variable keyword is KEYWORD REALLY
    (*token)->type      = KEYW_TYPE;
    (*token)->arg.key_w = keyword;

    return status;
}

// DONE
int GetKeyword(char **ptr, Node_t *node)
{
    if (ptr  == nullptr || *ptr == nullptr)   return ASMcmp::PTR_IS_NULL;
    int status = NodeVerify(node);
    if (status) return status;

    #define DEF_KEYW(DEF, CODE, KEYW, FMT)                   \
        if (StrEqual(*ptr, #FMT))                             \
        {                                                      \
            status = CreateKeyword(&node->value, KEYW_ ## DEF); \
            if (status) return status;                           \
            (*ptr) += strlen(#FMT);                               \
        }                                                          \
        else

    #define DEF_OPER(DEF, CODE, SIGN)                                 \
        if (StrEqual(*ptr, #SIGN))                                     \
        {                                                               \
            status = CreateKeyword(&node->value, KEYW_ ## DEF);          \
            if (status) return status;                                    \
            (*ptr) += strlen(#SIGN);                                       \
        }                                                                   \
        else

    #define DEF_HELP(DEF, CODE, HELP)

    #include "../keywords"
    #include "../operators"
 /* else */
    {
        PRINT(UNDEFINED_KEYWORD);
        PRINT_SM(*ptr, 10);
        return ASMcmp::UNDEFINED_KEYWORD;
    }

    #undef DEF_KEYW
    #undef DEF_OPER
    #undef DEF_HELP

    return status;
}

// DONE
int GetIdentifier(char **ptr, Node_t *node)
{
    if (ptr  == nullptr || *ptr == nullptr)   return ASMcmp::PTR_IS_NULL;
    int status = NodeVerify(node);
    if (status) return status;

    if (**ptr != '\'')                        return ASMcmp::WRONG_IDENTIFIER_FMT;

    (*ptr)++;

    Token_t *token = node->value;
    if (token == nullptr) return ASMcmp::TOKEN_IS_NULL;

    token->type = ID_TYPE;

    size_t index = 0;

    while (**ptr != '\'')
    {
        if (index == WORD_MAX_LEN) return ASMcmp::WORD_MAX_LEN_REACHED;
        token->arg.id[index++] = **ptr;
        (*ptr)++;
    }

    if (**ptr != '\'') return ASMcmp::WRONG_IDENTIFIER_FMT;

    (*ptr)++;

    return status;
}

// DONE
int GetNumber(char **ptr, Node_t *node)
{
    if (ptr  == nullptr || *ptr == nullptr)   return ASMcmp::PTR_IS_NULL;
    int status = NodeVerify(node);
    if (status) return status;

    if (!(**ptr >= '0' && **ptr <= '9'))      return ASMcmp::WRONG_IDENTIFIER_FMT;

    Token_t *token = node->value;
    if (token == nullptr) return ASMcmp::TOKEN_IS_NULL;

    token->type = NUM_TYPE;

    int length = 0;
    double num = NAN;
    sscanf(*ptr, "%lf%n", &num, &length);
    if (length == 0 || isnan(num))            return ASMcmp::WRONG_NUM_FMT;

    token->arg.num = num;

    (*ptr) += length;

    return status;
}

// DONE
int GetValue(char **ptr, Node_t *node)
{
    if (ptr  == nullptr || *ptr == nullptr)   return ASMcmp::PTR_IS_NULL;
    int status = NodeVerify(node);
    if (status) return status;

    char *start = *ptr;
    char *close_fig_bracket = strchr(*ptr, '}');

    ptrdiff_t length = close_fig_bracket - start;
    if (length == 0)                          return ASMcmp::EMPTY_VALUE;

    if (**ptr == '\'')
    {
        status = GetIdentifier(ptr, node);
        if (status) return status;
    }
    else
    if (**ptr >= '0' && **ptr <= '9')
    {
        status = GetNumber(ptr, node);
        if (status) return status;
    }
    else
    {
        status = GetKeyword(ptr, node);
        if (status) return status;
    }

    return status;
}

// DONE
int FillNodes(char **ptr, Node_t *node)
{
    if (ptr  == nullptr || *ptr == nullptr)   return ASMcmp::PTR_IS_NULL;
    int status = NodeVerify(node);
    if (status) return status;
    if (node->left  != nullptr ||
        node->right != nullptr)               return ASMcmp::NODE_HAS_CHILDREN;

    status = GetChild(ptr, &node->left);
    if (status && status != ASMcmp::NO_CHILD) return status;
    if (status == ASMcmp::NO_CHILD) status = ASMcmp::FUNC_IS_OK;
    else
        node->left->parent = node;

    assert(**ptr != '{' && **ptr != '}');

    status = GetValue(ptr, node);
    if (status) return status;

    status = GetChild(ptr, &node->right);
    if (status && status != ASMcmp::NO_CHILD) return status;
    if (status == ASMcmp::NO_CHILD) status = ASMcmp::FUNC_IS_OK;
    else
        node->right->parent = node;

    return status;
}

// DONE
int FillTree(const char *filename, Tree_t *tree)
{
    int status = TreeVerify(tree);
    if (status) return status;
    if (filename   == nullptr) return ASMcmp::FILENAME_IS_NULL;

    FILE *stream = nullptr;
    status = OpenFile(filename, &stream, "r");
    if (status) return status;

    char *buffer = nullptr;
    status = ReadBuffer(&buffer, stream);
    if (status) return status;

    if (tree->root != nullptr) return ASMcmp::TREE_ROOT_ISNT_NULL;

    status = InitEmptyNode(&tree->root);
    if (status) return status;

    char *ptr = buffer;

    if (*ptr != '{')           return ASMcmp::TREE_FORMAT_ISNT_RIGHT;
    ptr++;

    status = FillNodes(&ptr, tree->root);
    if (status) return status;

    if (*ptr != '}')           return ASMcmp::TREE_FORMAT_ISNT_RIGHT;

    free(buffer);

    return status;
}

uint32_t HashCRC32(void *memory, size_t length)
{
    if (memory == nullptr) return 0;

    uint32_t hash = 0xFFFFFFFF;

    size_t index = 0;

    for ( ; index + 7 < length; index += 8)
        hash = _mm_crc32_u64(hash, *(uint64_t *)(((uint8_t *)memory) + index));

    for ( ; index + 3 < length; index += 4)
        hash = _mm_crc32_u32(hash, *(uint32_t *)(((uint8_t *)memory) + index));

    for ( ; index + 1 < length; index += 2)
        hash = _mm_crc32_u16(hash, *(uint16_t *)(((uint8_t *)memory) + index));

    for ( ; index + 0 < length; index += 1)
        hash = _mm_crc32_u8(hash,  *(uint8_t  *)(((uint8_t *)memory) + index));

    return hash ^ 0xFFFFFFFF;
}




int BackendVerify(Backend *back)
{
    if (back == nullptr) return ASMcmp::BACK_NULL;

    return 0;
}

int EmitByte(Backend *back, uint8_t byte)
{
    int status = BackendVerify(back);
    CATCH_ERR;

    if (back->buff_ptr >= back->buff_size)
    {
        PRINT(END_OF_BUFFER_CANT_EMIT);
        return ASMcmp::BUFF_OVERFLOW;
    }

    back->code_buff[back->buff_ptr] = byte;
    back->buff_ptr += 1;

    return 0;
}

int EmitDword(Backend *back, uint32_t dword) // this function is usually used to store dw address
{
    int status = BackendVerify(back);
    CATCH_ERR;

    if (back->buff_ptr >= back->buff_size)
    {
        PRINT(END_OF_BUFFER_CANT_EMIT);
        return ASMcmp::BUFF_OVERFLOW;
    }

    *((uint32_t *)(back->code_buff + back->buff_ptr)) = dword;
    back->buff_ptr += 4;

    return 0;
}

int __EmitBytes(Backend *back, size_t number_of_items, ...)
{
    va_list arguments = {};

    va_start(arguments, number_of_items);

    for (size_t item = 0; item < number_of_items; item++)
        EmitByte(back, (uint8_t) va_arg(arguments, int));

    va_end(arguments);

    return 0;
}

#define __VA_COUNT__(...) (sizeof ((int[]) {0, ##__VA_ARGS__}) / sizeof (int) - 1)
#define EmitBytes(__BACK__, ...) do { __EmitBytes(__BACK__, __VA_COUNT__(__VA_ARGS__), __VA_ARGS__); } while(0)

#define NODE_KEYW(NODE, KEYW) (NODE->value->type == KEYW_TYPE && NODE->value->arg.key_w == KEYW)
#define   NODE_ID(NODE)       (NODE->value->type == ID_TYPE)
#define      KEYW(NODE)      ((NODE->value->type == KEYW_TYPE) ? NODE->value->arg.key_w : 0)

int EmitScan      (Node_t *node, Backend *back);
int EmitPrint     (Node_t *node, Backend *back);

int EmitMathOper  (Node_t *node, Backend *back);
int EmitNum       (Node_t *node, Backend *back);
int EmitVarInit   (Node_t *node, Backend *back);
int EmitVar       (Node_t *node, Backend *back);
int EmitExpr      (Node_t *node, Backend *back);
int EmitCallParams(Node_t *node, Backend *back);
int EmitCall      (Node_t *node, Backend *back);
int EmitJump      (Node_t *node, Backend *back);
int EmitCond      (Node_t *node, Backend *back);
int EmitIf        (Node_t *node, Backend *back);
int EmitReturn    (Node_t *node, Backend *back);
int EmitAssign    (Node_t *node, Backend *back);
int EmitWhile     (Node_t *node, Backend *back);
int EmitDefParams (Node_t *node, Backend *back);
int EmitFuncDef   (Node_t *node, Backend *back);
int EmitMain      (Node_t *node, Backend *back);
int EmitGlobExpr  (Node_t *node, Backend *back);

int EmitMark      (Node_t *node);

// int EmitGlobalVar (Node_t *node, Backend *back);
// int IncreaseRSP   (Node_t *node, Backend *back);
// int DecreaseRSP   (Node_t *node, Backend *back);

int EmitSysHeader (Backend *back);
int EmitGS        (Node_t *node, Backend *back);
int EmitStdLib    (Node_t *node, Backend *back);

int EmitASM       (const char *filename, Tree_t *tree);

// DONE
int IsNum(Node_t *node)
{
    return node != nullptr && (node->value->type == NUM_TYPE);
}

// DONE
int IsVar(Node_t *node)
{
    return node != nullptr && (node->value->type == ID_TYPE && node->parent && KEYW(node->parent) != KEYW_DEFINE);
}

// DONE
int IsMathOper(Node_t *node)
{
    return node != nullptr && ((KEYW(node) != 0) && (KEYW(node) >= KEYW_ADD) && (KEYW(node) <= KEYW_POW));
}

// DONE
int IsLogOper(Node_t *node)
{
    return node != nullptr && ((KEYW(node) != 0) && (KEYW(node) >= KEYW_EQUAL) && (KEYW(node) <= KEYW_OR));
}

// DONE
int EmitLog(Backend *back, const char *msg)
{
    int status = BackendVerify(back);
    CATCH_ERR;

    if (msg == nullptr)
    {
        PRINT(MSG_IS_NULLPTR);
        return ASMcmp::MSG_IS_NULLPTR;
    }

    fprintf(back->asm_log, "\t%s\n", msg);

    return status;
}



// returns ASMcmp::VARIABLE_FOUND if found
// returns 0 if not found
size_t SearchVariable(Node_t *node, Backend *back)
{
    int status = BackendVerify(back);
    CATCH_ERR;

    int64_t hash = HashCRC32(node->value->arg.id, WORD_MAX_LEN);

    // Now I think that collision chance is equal to zero, so
    // I will compare only hashes.
    size_t index = ListValueIndex(back->NT, hash);

    return index;
}

// returns ASMcmp::VARIABLE_FOUND if found
// returns 0 if not found
size_t PushVariable(Node_t *node, Backend *back)
{
    int status = BackendVerify(back);
    CATCH_ERR;

    int64_t hash = HashCRC32(node->value->arg.id, WORD_MAX_LEN);

    List_t *NT = back->NT;

    Val_t value = {};
    value.hash          = hash;
    value.offset        = -16 + (-8) * NT->size;
    value.elem.variable = node->value->arg.id;

    // Now I think that collision chance is equal to zero, so
    // I will compare only hashes.
    size_t index = ListValueIndex(back->NT, hash);

    return index;
}


// DONE
int EmitStmt(Node_t *node, Backend *back)
{
    int status = NodeVerify(node);
    CATCH_ERR;

    switch (KEYW(node))
    {
        case KEYW_ASSIGN: status = EmitAssign(node, back); break;
        case KEYW_IF:     status = EmitIf    (node, back); break;
        case KEYW_WHILE:  status = EmitWhile (node, back); break;
        case KEYW_CALL:   status = EmitCall  (node, back); break;
        case KEYW_RETURN: status = EmitReturn(node, back); break;
        case KEYW_SCAN:   status = EmitScan  (node, back); break;
        case KEYW_PRINT:  status = EmitPrint (node, back); break;
        case KEYW_ADD:
        case KEYW_SUB:
        case KEYW_MUL:
        case KEYW_DIV:    status = EmitExpr  (node, back); break;

        default:
            status = ASMcmp::INVALID_STMT;
            CATCH_ERR;
            break;
    }

    CATCH_ERR;
    return status;
}

// DONE
int EmitStmts(Node_t *node, Backend *back)
{
    int status = NodeVerify(node);
    CATCH_ERR;

    if (!NODE_KEYW(node, KEYW_STMT)) return ASMcmp::INVALID_STMT;

    while (node->left) node = node->left;

    while (NODE_KEYW(node, KEYW_STMT))
    {
        int status = EmitStmt(node->right, back);
        CATCH_ERR;

        node = node->parent;
    }

    return status;
}

int EmitScan      (Node_t *node, Backend *back);
int EmitPrint     (Node_t *node, Backend *back);

int EmitMathOper  (Node_t *node, Backend *back);
int EmitNum       (Node_t *node, Backend *back);
int EmitVarInit   (Node_t *node, Backend *back);
int EmitVar       (Node_t *node, Backend *back);
int EmitExpr      (Node_t *node, Backend *back);
int EmitCallParams(Node_t *node, Backend *back);
int EmitCall      (Node_t *node, Backend *back);
int EmitJump      (Node_t *node, Backend *back);
int EmitCond      (Node_t *node, Backend *back);
int EmitIf        (Node_t *node, Backend *back);
int EmitReturn    (Node_t *node, Backend *back);
int EmitWhile     (Node_t *node, Backend *back);
int EmitDefParams (Node_t *node, Backend *back);
int EmitFuncDef   (Node_t *node, Backend *back);
int EmitMain      (Node_t *node, Backend *back);
int EmitGlobExpr  (Node_t *node, Backend *back);

int EmitMark      (Node_t *node);

// int EmitGlobalVar (Node_t *node, Backend *back);
// int IncreaseRSP   (Node_t *node, Backend *back);
// int DecreaseRSP   (Node_t *node, Backend *back);

int EmitSysHeader(Backend *back);
int EmitGS       (Node_t *node, Backend *back);
int EmitStdLib   (Node_t *node, Backend *back);

int EmitASM      (const char *filename, Tree_t *tree);




int EmitDefParams(Node_t *node, Backend *back)
{
    int status = BackendVerify(back);
    CATCH_ERR;

    if (back->NT->size != 0)
    {
        status = ASMcmp::NAMETABLE_ISNT_CLEAR;
        CATCH_ERR;
    }

    // pushing parameters in reverse order, like System V
    while (node != nullptr)
    {
        status = SearchInNametable(node->right, back);
        if (status == ASMcmp::VARIABLE_FOUND) status = ASMcmp::VARIABLE_IS_ENGAGED;
        CATCH_ERR;

        status = PushInNametable(node->right, back);
        CATCH_ERR;

        node = node->left;
    }

    return status;
}

int EmitFuncDef(Node_t *node, Backend *back)
{
    int status = BackendVerify(back);
    CATCH_ERR;

    Node_t *func = node->left;
    CATCH_NULL(func);

    Node_t *mark = func->left;
    CATCH_NULL(mark);

    if (KEYW(node) == KEYW_MAIN1)
    {
        if (back->main == nullptr)
        {
            back->main = node;
            return ASMcmp::FUNC_IS_OK;
        }
        else
        {
            status = ASMcmp::REPEATING_MAIN;
            CATCH_ERR;
        }
    }

    status = EmitMark(mark);
    CATCH_ERR;

    Node_t *params = func->right;

    if (params != nullptr)
    {
        status = EmitDefParams(params, back);
        CATCH_ERR;
    }

    Node_t *stmts  = node->right;
    CATCH_NULL(stmts);

    status = EmitStmts(stmts, back);
    CATCH_ERR;

    return status;
}

int EmitGS(Node_t *node, Backend *back)
{
    int status = BackendVerify(back);
    return status;
    status = NodeVerify(node);
    CATCH_ERR;

    if (!NODE_KEYW(node, KEYW_STMT)) return ASMcmp::GLOBAL_STMTS_ARE_NOT_STMTS;

    while (node->left) node = node->left;
    // IM AT BOTTOM STMT

    // Node_t *bottom = node;
    //
    // Processing global variables
    // while (node)
    // {
    //     if (KEYW(node->right) == KEYW_ASSIGN)
    //     {
    //         status = EmitAssign(node, back);
    //         CATCH_ERR;
    //     }
    //     node = node->parent;
    // }
    //
    // node = bottom;

    // IM AT BOTTOM STMT
    while (node != nullptr)
    {
        //-----------------------------------------
        //   It is proceeding global variables   //
        // ----------------------------------------

        // if (NODE_KEYW(node->right, KEYW_ASSIGN))
        // {
        //     node = node->parent;
        //     continue;
        // }
        // else
        if (NODE_KEYW(node->right, KEYW_DEFINE))
        {
            status = EmitFuncDef(node->right, back);
            CATCH_ERR;
        }
        else
        {
            PRINT_(INVALID_GLOBAL_STMT);
            status = ASMcmp::INVALID_GLOBAL_STMT;
            CATCH_ERR;
        }

        node = node->parent;
    }

    status = EmitMain(back->main, back);
    CATCH_ERR;

    return status;
}

int EmitElf(Backend *back)
{
    int status = BackendVerify(back);
    CATCH_ERR;

    //  Now there is no translation into elf. So.

    // mov rax, 0xFFFFFF
    EmitBytes(back, 0xB8);
    EmitDword(back, 0xFFFFFF);
    // saving RET
    EmitBytes(back, RET);

    int pagesize = getpagesize();

    char *code = nullptr;
    status = posix_memalign((void **)&code, pagesize, back->buff_ptr);
    if (status)
    {
        PRINT(POSIX_MEMALINGN_ERR);
        return ASMcmp::POSIX_MEMALIGN_FAILED;
    }

    for (size_t index = 0; index < back->buff_ptr; ++index)
        code[index] = back->code_buff[index];

    status = mprotect(code, pagesize, PROT_EXEC); /* | PROT_WRITE | PROT_READ */
    if (status)
    {
        PRINT(MPROTECT_RETURNED_SHIT);
        return ASMcmp::MPROTECT_FAILED;
    }

    int x = ((int(*)())code)();
    PRINT_X(x);

    return status;
}

int EmitASM(const char *filename, Tree_t *tree)
{
    int status = TreeVerify(tree);
    CATCH_ERR;
    if (filename == nullptr) return ASMcmp::FILENAME_IS_NULL;

    Backend *back = (Backend *) calloc(1, sizeof(Backend));
    if (back == nullptr) return ASMcmp::BAD_ALLOC;

    back->buff_ptr  = 0;
    back->buff_size = TreeSize(tree->root) * sizeof(uint64_t) * 2;

    back->code_buff = (char *) calloc(back->buff_size, sizeof(char));
    if (back->code_buff == nullptr) return ASMcmp::BAD_ALLOC;

    back->asm_log = fopen(filename, "w");
    if (back->asm_log == nullptr)
    {
        PRINT(FILE_CANT_BE_OPENED);
        return ASMcmp::FILE_CANT_BE_OPENED;
    }

    back->NT = (List_t *) calloc(1, sizeof(List_t));
    if (back->NT == nullptr) return ASMcmp::BAD_ALLOC;

    status = ListCtor(back->NT, ASMcmp::INITIAL_CAPACITY);
    CATCH_ERR;


    status = EmitSysHeader(back);
    CATCH_ERR;

    status = EmitGS(tree->root, back);
    CATCH_ERR;

    status = EmitElf(back);
    CATCH_ERR;


    status = ListDtor(back->NT);
    if (status == LIST_IS_DESTRUCTED) status = ASMcmp::FUNC_IS_OK;
    CATCH_ERR;

    fclose(back->asm_log);

    free(back->code_buff);
    free(back->NT);
    free(back);

    return status;
}

#undef __VA_COUNT__
#undef EmitBytes

#undef NODE_KEYW
#undef NODE_ID
#undef KEYW
