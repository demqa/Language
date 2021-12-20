#include "asm_comp.h"

#define NODE_KEYW(NODE, KEYW) (NODE->value->type == KEYW_TYPE && NODE->value->arg.key_w == KEYW)
#define NODE_ID(NODE) (NODE->value->type == ID_TYPE)
#define KEYW(NODE) ((node->value->type == KEYW_TYPE) ? node->value->arg.key_w : 0)

#define CHECK_NODES_N_LISTS       \
    int status = NodeVerify(node); \
    CATCH_ERR;                      \
    status = ListVerify(GlobalNT);   \
    CATCH_ERR;                        \
    status = ListVerify(NT);           \
    CATCH_ERR;                          \
    if (out == nullptr) return ASMcmp::OUT_STREAM_IS_NULL;

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

int CreateKeyword(Token_t **token, const int keyword)
{
    if (token == nullptr || *token == nullptr) return ASMcmp::TOKEN_IS_NULL;

    int status = ASMcmp::FUNC_IS_OK;

    // TODO: check variable keyword is KEYWORD REALLY
    (*token)->type      = KEYW_TYPE;
    (*token)->arg.key_w = keyword;

    return status;
}

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
        return ASMcmp::UNDEFINED_KEYWORD;
    }

    #undef DEF_KEYW
    #undef DEF_OPER
    #undef DEF_HELP

    return status;
}

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


int PushInNametable(Node_t *node, List_t *NT)
{
    int status = NodeVerify(node);
    CATCH_ERR;
    status = ListVerify(NT);
    CATCH_ERR;

    Name_t name  = {};
    
    // TODO:
    // use index to save normal index in RAM

    memcpy(name.name, node->value->arg.id, sizeof(node->value->arg.id));

    status = ListPushBack(NT, name);
    CATCH_ERR;

    return status;
}

int IndexNametable(Node_t *node, List_t *NT, size_t *index)
{
    int status = NodeVerify(node);
    CATCH_ERR;
    status = ListVerify(NT);
    CATCH_ERR;
    if (index == nullptr) return ASMcmp::PTR_IS_NULL;

    if (node->value->type != ID_TYPE)
    {
        status = ASMcmp::CANT_FIND_NON_ID_IN_NT;
        CATCH_ERR;
    }

    size_t number = ListValueIndex(NT, node->value->arg.id);
    if (number == 0)
    {
        status = ASMcmp::VARIABLE_NOT_FOUND;
        CATCH_ERR;
    }

    *index = number;

    return status;
}

// \brief Returns ASMcmp::VARIABLE_FOUND
//        if found, 0 if ok, error code if error
int SearchInNametable(Node_t *node, List_t *NT)
{
    int status = NodeVerify(node);
    CATCH_ERR;
    status = ListVerify(NT);
    CATCH_ERR;

    size_t index = ListValueIndex(NT, node->value->arg.id);
    if (index != 0)
    {
        return ASMcmp::VARIABLE_FOUND;
    }

    return ASMcmp::FUNC_IS_OK;
}

static FILE *out         = nullptr;
static Node_t *node_main = nullptr;

int GenerateScan(Node_t *node, List_t *NT, List_t *GlobalNT)
{
    CHECK_NODES_N_LISTS;

    PRINT_(NOT_IMPLEMENTED);

    return status;
}

int GeneratePrint(Node_t *node, List_t *NT, List_t *GlobalNT)
{
    CHECK_NODES_N_LISTS;

    PRINT_(NOT_IMPLEMENTED);

    return status;
}

// DONE
int GenerateAssign(Node_t *node, List_t *NT, List_t *GlobalNT)
{
    int status = NodeVerify(node);
    CATCH_ERR;
    status = ListVerify(GlobalNT);
    CATCH_ERR;

    if (NT == nullptr)
    {
        status = InitGlobVar(node, GlobalNT);
        CATCH_ERR;
    }
    else
    {
        status = InitVar(node, NT, GlobalNT);
        CATCH_ERR;
    }

    return status;
}

int GenerateStmt  (Node_t *node, List_t *NT, List_t *GlobalNT)
{
    CHECK_NODES_N_LISTS;

    switch (KEYW(node))
    {
        case KEYW_ASSIGN: status = GenerateAssign(node, NT, GlobalNT); break;
        case KEYW_IF:     status = GenerateIf    (node, NT, GlobalNT); break;
        case KEYW_WHILE:  status = GenerateWhile (node, NT, GlobalNT); break;
        case KEYW_CALL:   status = GenerateCall  (node, NT, GlobalNT); break;
        case KEYW_RETURN: status = GenerateReturn(node, NT, GlobalNT); break;
        case KEYW_SCAN:   status = GenerateScan  (node, NT, GlobalNT); break;
        case KEYW_PRINT:  status = GeneratePrint (node, NT, GlobalNT); break;

        default:
            status = ASMcmp::INVALID_STMT;
            CATCH_ERR;
            break;
    }

    return status;
}

int GenerateStmts (Node_t *node, List_t *NT, List_t *GlobalNT)
{
    CHECK_NODES_N_LISTS;

    if (KEYW(node) != KEYW_STMT) return ASMcmp::INVALID_STMT;

    while (node->left) node = node->left;

    while (node != nullptr)
    {
        int status = GenerateStmt(node->right, NT, GlobalNT);
        CATCH_ERR;

        node = node->parent;
    }

    PRINT_(NOT_IMPLEMENTED);

    return status;
}

int IsNum(Node_t *node, int *ans)
{
    int status = NodeVerify(node);
    CATCH_ERR;
    if (ans == nullptr) return ASMcmp::ANS_IS_NULL;

    *ans = (node->value->type == NUM_TYPE);

    return status;
}

int IsVar(Node_t *node, int *ans)
{
    int status = NodeVerify(node);
    CATCH_ERR;
    if (ans == nullptr) return ASMcmp::ANS_IS_NULL;

    *ans = (node->value->type == ID_TYPE && node->parent && KEYW(node->parent) != KEYW_DEFINE);

    return status;
}

int IsMathOper(Node_t *node, int *ans)
{
    int status = NodeVerify(node);
    CATCH_ERR;
    if (ans == nullptr) return ASMcmp::ANS_IS_NULL;

    *ans = ((KEYW(node) != 0) && (KEYW(node) >= KEYW_ADD) && (KEYW(node) <= KEYW_POW));

    return status;
}

int IsLogOper(Node_t *node, int *ans)
{
    int status = NodeVerify(node);
    CATCH_ERR;
    if (ans == nullptr) return ASMcmp::ANS_IS_NULL;

    *ans = ((KEYW(node) != 0) && (KEYW(node) >= KEYW_EQUAL) && (KEYW(node) <= KEYW_OR));

    return status;
}

int GenerateMathOper(Node_t *node)
{
    int status = NodeVerify(node);
    CATCH_ERR;
    if (out = nullptr) return ASMcmp::OUT_STREAM_IS_NULL;

    int is_math_oper = 0;
    status = IsMathOper(node, &is_math_oper);
    CATCH_ERR;

    PRINT_(NOT_IMPLEMENTED);

    return status;
}

// DONE
int GenerateNum(Node_t *node)
{
    int status = NodeVerify(node);
    CATCH_ERR;

    fprintf(out, "PUSH %lg\n", node->value->arg.num);

    return status;
}

// DONE
int InitVar(Node_t *node, List_t *NT, List_t *GlobalNT)
{
    CHECK_NODES_N_LISTS;

    status = SearchInNametable(node->left, GlobalNT);
    if (status == ASMcmp::VARIABLE_FOUND) status = ASMcmp::REPEATING_VARIABLE;
    CATCH_ERR;

    status = SearchInNametable(node->left, NT);
    if (status == ASMcmp::VARIABLE_FOUND) status = ASMcmp::REPEATING_VARIABLE;
    CATCH_ERR;

    status = PushInNametable(node->left, NT);
    CATCH_ERR;

    size_t index = 0;
    status = IndexNametable(node->left, NT, &index);
    CATCH_ERR;

    assert(index >= 1);

    status = GenerateExpr(node->right, NT, GlobalNT);
    CATCH_ERR;

    fprintf(out, "POP [bx+%lu]\n", index - 1);

    return status;
}

// DONE
int GenerateVar(Node_t *node, List_t *NT, List_t *GlobalNT)
{
    CHECK_NODES_N_LISTS;

    status = SearchInNametable(node, GlobalNT);
    if (status == ASMcmp::VARIABLE_FOUND)
    {
        status = GenerateGlobVar(node, GlobalNT);
        return status;
    }
    CATCH_ERR;

    status = SearchInNametable(node, NT);
    if (status == ASMcmp::VARIABLE_FOUND)
    {
        size_t index = 0;
        status = IndexNametable(node, NT, &index);
        CATCH_ERR;

        fprintf(out, "PUSH [bx+%lu]\n", index);

        return status;
    }

    CATCH_ERR;

    status = ASMcmp::VARIABLE_NOT_FOUND;
    CATCH_ERR;
}

// DONE
int GenerateExpr  (Node_t *node, List_t *NT, List_t *GlobalNT)
{
    CHECK_NODES_N_LISTS;

    if (KEYW(node) == KEYW_CALL)
    {
        status = GenerateCall(node, NT, GlobalNT);
        CATCH_ERR;
        return status;
    }

    if (node->left)
    {
        status = GenerateExpr(node->left, NT, GlobalNT);
        CATCH_ERR;
    }

    if (node->right)
    {
        status = GenerateExpr(node->left, NT, GlobalNT);
        CATCH_ERR;
    }

    int ans = 0;

    status = IsMathOper(node, &ans);
    CATCH_ERR;
    if (ans)
    {
        status = GenerateMathOper(node);
        CATCH_ERR;

        return status;
    }

    status = IsNum(node, &ans);
    CATCH_ERR;
    if (ans)
    {
        status = GenerateNum(node);
        CATCH_ERR;

        return status;
    }

    status = IsVar(node, &ans);
    CATCH_ERR;
    if (ans)
    {
        status = GenerateVar(node, NT, GlobalNT);
        CATCH_ERR;

        return status;
    }

    status = ASMcmp::UNDEFINED_OPERATOR;
    CATCH_ERR;
}

int GenerateCall(Node_t *node, List_t *NT, List_t *GlobalNT)
{
    CHECK_NODES_N_LISTS;

    PRINT_(NOT_IMPLEMENTED);

    return status;
}

int GenerateJump(Node_t *node, List_t *NT, List_t *GlobalNT, const char *mark, const int num)
{
    CHECK_NODES_N_LISTS;
    if (mark == nullptr) return ASMcmp::PTR_IS_NULL;

    PRINT_(NOT_IMPLEMENTED);

    return status;
}

int GenerateCond(Node_t *node, List_t *NT, List_t *GlobalNT)
{
    CHECK_NODES_N_LISTS;

    PRINT_(NOT_IMPLEMENTED);

    return status;
}

static size_t __IF_COUNTER__    = 0;
static size_t __WHILE_COUNTER__ = 0;

int GenerateIf(Node_t *node, List_t *NT, List_t *GlobalNT)
{
    CHECK_NODES_N_LISTS;

    PRINT_(NOT_IMPLEMENTED);

    return status;
}

int GenerateReturn(Node_t *node, List_t *NT, List_t *GlobalNT)
{
    CHECK_NODES_N_LISTS;

    PRINT_(NOT_IMPLEMENTED);

    return status;
}

int GenerateWhile(Node_t *node, List_t *NT, List_t *GlobalNT)
{
    CHECK_NODES_N_LISTS;

    PRINT_(NOT_IMPLEMENTED);

    return status;
}

int GenerateFuncDef(Node_t *node, List_t *NT, List_t *GlobalNT)
{
    CHECK_NODES_N_LISTS;

    size_t free_memory = 0;

    Node_t *func   = node->left;
    CATCH_NULL(func);

    Node_t *mark   = func->left;
    CATCH_NULL(mark);

    if (KEYW(mark) == KEYW_MAIN1)
    {
        if (node_main == nullptr)
        {
            node_main = node;
            return ASMcmp::FUNC_IS_OK;
        }
        else
        {
            status = ASMcmp::REPEATING_MAIN;
            CATCH_ERR;
        }
    }

    Node_t *params = func->right;

    if (params)
    {
        status = GenerateParams(params, NT, GlobalNT);
        CATCH_ERR;
    }


    status = GenerateMark(mark);
    CATCH_ERR;

    PRINT_(NOT_IMPLEMENTED);

    // fprintf(stream, "%s:\n", mark->value->arg.id);

    // while params: POP

    return status;
}

// DONE
int GenerateGlobVar(Node_t *node, List_t *GlobalNT)
{
    if (out == nullptr) return ASMcmp::OUT_STREAM_IS_NULL;
    int status = NodeVerify(node);
    CATCH_ERR;
    status = ListVerify(GlobalNT);
    CATCH_ERR;

    status = SearchInNametable(node, GlobalNT);
    if (status == ASMcmp::VARIABLE_FOUND)
    {
        size_t index = 0;
        status = IndexNametable(node, GlobalNT, &index);
        CATCH_ERR;

        // TODO: take beginning of RAM somewhere
        fprintf(out, "PUSH [%lu]\n", index);

        return status;
    }

    CATCH_ERR;

    status = ASMcmp::VARIABLE_NOT_FOUND;
    CATCH_ERR;
}

// DONE
int GenerateGlobExpr(Node_t *node, List_t *GlobalNT)
{
    if (out == nullptr) return ASMcmp::OUT_STREAM_IS_NULL;
    int status = NodeVerify(node);
    CATCH_ERR;
    status = ListVerify(GlobalNT);
    CATCH_ERR;

    if (KEYW(node) == KEYW_CALL)
    {
        // NOT IMPLEMENTED
        status = ASMcmp::CANT_USE_FUNC_IN_GLOBAL;
        CATCH_ERR;
    }
    
    if (node->left)
    {
        status = GenerateGlobExpr(node->left, GlobalNT);
        CATCH_ERR;
    }

    if (node->right)
    {
        status = GenerateGlobExpr(node->right, GlobalNT);
        CATCH_ERR;
    }

    int ans = 0;

    status = IsMathOper(node, &ans);
    CATCH_ERR;
    if (ans)
    {
        status = GenerateMathOper(node);
        CATCH_ERR;

        return status;
    }

    status = IsNum(node, &ans);
    CATCH_ERR;
    if (ans)
    {
        status = GenerateNum(node);
        CATCH_ERR;

        return status;
    }

    status = IsVar(node, &ans);
    CATCH_ERR;
    if (ans)
    {
        status = GenerateGlobVar(node, GlobalNT);
        CATCH_ERR;

        return status;
    }

    status = ASMcmp::UNDEFINED_OPERATOR;
    CATCH_ERR;
}

// DONE
int InitGlobVar(Node_t *node, List_t *GlobalNT)
{
    if (out == nullptr) return ASMcmp::OUT_STREAM_IS_NULL;
    int status = NodeVerify(node);
    CATCH_ERR;
    status = ListVerify(GlobalNT);
    CATCH_ERR;

    status = SearchInNametable(node->left, GlobalNT);
    if (status == ASMcmp::VARIABLE_FOUND) status = ASMcmp::REPEATING_VARIABLE;
    CATCH_ERR;

    status = PushInNametable(node->left, GlobalNT);
    CATCH_ERR;

    size_t index = 0;
    status = IndexNametable(node->left, GlobalNT, &index);
    CATCH_ERR;
    
    assert(index >= 1);

    status = GenerateGlobExpr(node->right, GlobalNT);
    CATCH_ERR;

    fprintf(out, "POP [bx+%lu]\n", index - 1);

    return status;
}

int GenerateGS(Node_t *node, List_t *GlobalNT)
{
    if (out == nullptr) return ASMcmp::OUT_STREAM_IS_NULL;
    int status = NodeVerify(node);
    CATCH_ERR;
    status = ListVerify(GlobalNT);
    CATCH_ERR;

    if (!NODE_KEYW(node, KEYW_STMT)) return ASMcmp::GLOBAL_STMTS_ARE_NOT_STMTS;

    // IM AT BOTTOM STMT
    while (node->left) node = node->left;
    
    Node_t *bottom = node;

    while (node)
    {
        if (KEYW(node->right) == KEYW_ASSIGN)
        {
            status = GenerateAssign(node, nullptr, GlobalNT);
            CATCH_ERR;
        }
        node = node->parent;
    }

    // bx += (GlobalNT)->size;
    #ifdef DEBUG_LIB_H
        fprintf(out, "\n");
    #endif
    fprintf(out, "PUSH bx \n");
    fprintf(out, "PUSH %lu\n", GlobalNT->size);
    fprintf(out, "ADD     \n");
    fprintf(out, "POP bx  \n");
    #ifdef DEBUG_LIB_H
        fprintf(out, "\n");
    #endif

    List_t *NT = (List_t *) calloc(1, sizeof(List_t));
    if (NT == nullptr) return ASMcmp::BAD_ALLOC;

    status = ListCtor(NT, ASMcmp::INITIAL_CAPACITY);
    CATCH_ERR;

    node = bottom;
    // IM AT BOTTOM STMT
    while (node != nullptr)
    {
        if (NODE_KEYW(node->right, KEYW_DEFINE))
        {
            status = GenerateFuncDef(node->right, NT, GlobalNT);
            CATCH_ERR;
            status = ListClear(NT);
            CATCH_ERR;
        }
        else
        if (NODE_KEYW(node->right, KEYW_ASSIGN))
        {
            node = node->parent;
            continue;
        }
        else
        {
            PRINT_(INVALID_GLOBAL_STMT);
            status = ASMcmp::INVALID_GLOBAL_STMT;
            CATCH_ERR;
        }
        node = node->parent;
    }

    status = GenerateMain(node_main, NT, GlobalNT);
    CATCH_ERR;

    status = ListDtor(NT);
    CATCH_ERR;

    free(NT);
    
    return status;
}

int GenerateASM(const char *filename, Tree_t *tree)
{
    int status = TreeVerify(tree);
    if (status) return status;
    if (filename   == nullptr) return ASMcmp::FILENAME_IS_NULL;

    FILE *stream = nullptr;
    status = OpenFile(filename, &stream, "w");
    if (status) return status;

    out = stream;

    List_t *GlobalNT = (List_t *) calloc(1, sizeof(List_t));
    if (GlobalNT == nullptr) return ASMcmp::BAD_ALLOC;

    status = ListCtor(GlobalNT, ASMcmp::INITIAL_CAPACITY);
    CATCH_ERR;

    status = GenerateGS(tree->root, GlobalNT);
    CATCH_ERR;

    status = ListDtor(GlobalNT);
    CATCH_ERR;

    fclose(out);
    out = nullptr;

    free(GlobalNT);

    return status;
}
