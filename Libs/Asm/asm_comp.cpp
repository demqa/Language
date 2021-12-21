#include "asm_comp.h"

#define NODE_KEYW(NODE, KEYW) (NODE->value->type == KEYW_TYPE && NODE->value->arg.key_w == KEYW)
#define NODE_ID(NODE) (NODE->value->type == ID_TYPE)
#define KEYW(NODE) ((NODE->value->type == KEYW_TYPE) ? NODE->value->arg.key_w : 0)

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
        PRINT_SM(*ptr, 10);
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

// DONE
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

// DONE
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

static FILE   *out       = nullptr;
static Node_t *node_main = nullptr;

int GenerateScan(Node_t *node, List_t *NT, List_t *GlobalNT)
{
    CHECK_NODES_N_LISTS;

    PRINT_(NOT_IMPLEMENTED);

    return status;
}

// DONE
int GeneratePrint(Node_t *node, List_t *NT, List_t *GlobalNT)
{
    CHECK_NODES_N_LISTS;

    if (node->right == nullptr) return ASMcmp::PRINT_DOESNT_HAVE_ARG;

    status = GenerateExpr(node->right, NT, GlobalNT);
    CATCH_ERR;

    fprintf(out, "OUT\n");
    fprintf(out, "POP dx\n");

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

// DONE
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
        case KEYW_ADD:
        case KEYW_SUB:
        case KEYW_MUL:
        case KEYW_DIV:
        case KEYW_POW:    status = GenerateExpr  (node, NT, GlobalNT); break;


        default:
            status = ASMcmp::INVALID_STMT;
            CATCH_ERR;
            break;
    }

    CATCH_ERR;
    return status;
}

// DONE
int GenerateStmts (Node_t *node, List_t *NT, List_t *GlobalNT)
{
    CHECK_NODES_N_LISTS;

    if (!NODE_KEYW(node, KEYW_STMT)) return ASMcmp::INVALID_STMT;

    while (node->left) node = node->left;

    while (NODE_KEYW(node, KEYW_STMT))
    {
        int status = GenerateStmt(node->right, NT, GlobalNT);
        CATCH_ERR;

        node = node->parent;
    }

    return status;
}

// DONE
int IsNum(Node_t *node, int *ans)
{
    int status = NodeVerify(node);
    CATCH_ERR;
    if (ans == nullptr) return ASMcmp::ANS_IS_NULL;

    *ans = (node->value->type == NUM_TYPE);

    return status;
}

// DONE
int IsVar(Node_t *node, int *ans)
{
    int status = NodeVerify(node);
    CATCH_ERR;
    if (ans == nullptr) return ASMcmp::ANS_IS_NULL;

    *ans = (node->value->type == ID_TYPE && node->parent && KEYW(node->parent) != KEYW_DEFINE);

    return status;
}

// DONE
int IsMathOper(Node_t *node, int *ans)
{
    int status = NodeVerify(node);
    CATCH_ERR;
    if (ans == nullptr) return ASMcmp::ANS_IS_NULL;

    *ans = ((KEYW(node) != 0) && (KEYW(node) >= KEYW_ADD) && (KEYW(node) <= KEYW_POW));

    return status;
}

// DONE
int IsLogOper(Node_t *node, int *ans)
{
    int status = NodeVerify(node);
    CATCH_ERR;
    if (ans == nullptr) return ASMcmp::ANS_IS_NULL;

    *ans = ((KEYW(node) != 0) && (KEYW(node) >= KEYW_EQUAL) && (KEYW(node) <= KEYW_OR));

    return status;
}

// DONE
int GenerateMathOper(Node_t *node)
{
    if (out == nullptr) return ASMcmp::OUT_STREAM_IS_NULL;
    int status = NodeVerify(node);
    CATCH_ERR;

    int is_math_oper = 0;
    status = IsMathOper(node, &is_math_oper);
    CATCH_ERR;

    switch (KEYW(node))
    {
        case KEYW_ADD: fprintf(out, "ADD\n"); break;
        case KEYW_SUB: fprintf(out, "SUB\n"); break;
        case KEYW_MUL: fprintf(out, "MUL\n"); break;
        case KEYW_DIV: fprintf(out, "DIV\n"); break;

        // TODO: fix. cpu doesnt allow this now.
        case KEYW_POW: fprintf(out, "POW\n"); break;

        default:
            status = ASMcmp::UNEXPECTED_MATH_OPER;
            CATCH_ERR;
            break;
    }

    return status;
}

// DONE
int GenerateNum(Node_t *node)
{
    if (out == nullptr) return ASMcmp::OUT_STREAM_IS_NULL;
    int status = NodeVerify(node);
    CATCH_ERR;

    fprintf(out, "PUSH %lg\n", node->value->arg.num);
    // fprintf(stderr, "PUSH %lg\n", node->value->arg.num);

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
    if (status == ASMcmp::VARIABLE_FOUND) 
    {
        status = GenerateExpr(node->right, NT, GlobalNT);
        CATCH_ERR;

        size_t index = 0;
        status = IndexNametable(node->left, NT, &index);
        CATCH_ERR;

        assert(index >= 1);

        fprintf(out, "POP [bx+%lu]\n", index - 1);

        status = ASMcmp::FUNC_IS_OK;

        CATCH_ERR;

        // return status;
    }
    
    CATCH_ERR;

    status = PushInNametable(node->left, NT);
    CATCH_ERR;

    size_t index = 0;
    status = IndexNametable(node->left, NT, &index);
    CATCH_ERR;

    assert(index >= 1);

    status = GenerateExpr(node->right, NT, GlobalNT);
    CATCH_ERR;

    // fprintf(out, "PUSH 0\n");

    fprintf(out, "POP [bx+%lu]\n", index - 1);
    // fprintf(stderr, "POP [bx+%lu]\n", index - 1);

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

        fprintf(out, "PUSH [bx+%lu]\n", index - 1);
        // fprintf(stderr, "PUSH [bx+%lu]\n", index - 1);

        return status;
    }

    CATCH_ERR;

    status = ASMcmp::VARIABLE_NOT_FOUND;
    CATCH_ERR;

    return status;
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
        status = GenerateExpr(node->left,  NT, GlobalNT);
        CATCH_ERR;
    }

    if (node->right)
    {
        status = GenerateExpr(node->right, NT, GlobalNT);
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

    return status;
}

// DONE
int InitCallParams(Node_t *node, List_t *NT, List_t *GlobalNT, size_t *num_of_params)
{
    CHECK_NODES_N_LISTS;
    if (num_of_params == nullptr) return ASMcmp::PTR_IS_NULL;

    if (!NODE_KEYW(node, KEYW_PARAM))
    {
        status = ASMcmp::PARAM_ISNT_PARAM;
        CATCH_ERR;
    }

    if (node->left != nullptr)
    {
        PRINT_(NOT_IMPLEMENTED);
        status = ASMcmp::PARAM_ISNT_THE_ONLY;
        CATCH_ERR;
    }

    // TODO:
    // rewrite if I want more than one arguement
    assert(*num_of_params == 0);

    (*num_of_params)++;

    status = GenerateExpr(node->right, NT, GlobalNT);
    CATCH_ERR;

    fprintf(out, "POP [bx+%d]\n", *num_of_params);
    // fprintf(stderr, "POP [bx+1]\n");

    return status;
}

// DONE
int GenerateCall(Node_t *node, List_t *NT, List_t *GlobalNT)
{
    CHECK_NODES_N_LISTS;

    Node_t *params = node->right;
    if (params == nullptr)
    {
        PRINT_(NOT_IMPLEMENTED);
        status = ASMcmp::NO_PARAMS;
        CATCH_ERR;
    }

    size_t num_of_params = 0;

    status = InitCallParams(params, NT, GlobalNT, &num_of_params);
    CATCH_ERR;
    // TODO:
    // increase number of params to inf
    assert(num_of_params == 1);

    Node_t *name = node->left;
    CATCH_NULL(name);

    fprintf(out, "\nCALL %s:\n", name->value->arg.id);

    fprintf(out, "PUSH cx\n");

    return status;
}

// DONE
int GenerateJump(Node_t *node, List_t *NT, List_t *GlobalNT, const char *mark, const int num)
{
    CHECK_NODES_N_LISTS;
    if (mark == nullptr) return ASMcmp::PTR_IS_NULL;

    int is_log_op = 0;
    status = IsLogOper(node, &is_log_op);
    CATCH_ERR;

    if (!is_log_op)
    {
        status = ASMcmp::NOT_LOG_OPER;
        CATCH_ERR;
    }

    switch (KEYW(node))
    {
        case KEYW_LESS:      fprintf(out, "JAE"); break;
        case KEYW_LESSOREQ:  fprintf(out, "JA" ); break;
        case KEYW_NOTEQUAL:  fprintf(out, "JE" ); break;
        case KEYW_EQUAL:     fprintf(out, "JNE"); break;
        case KEYW_GREATOREQ: fprintf(out, "JB" ); break;
        case KEYW_GREAT:     fprintf(out, "JBE"); break;

        default:
            status = ASMcmp::UNDEFINED_OPERATOR;
            CATCH_ERR;
            break;
    }

    fprintf(out, " %s_%d:\n", mark, num);

    return status;
}

// DONE
int GenerateCond(Node_t *node, List_t *NT, List_t *GlobalNT, const char *mark, const int num)
{
    CHECK_NODES_N_LISTS;

    status = GenerateExpr(node->left,  NT, GlobalNT);
    CATCH_ERR;

    status = GenerateExpr(node->right, NT, GlobalNT);
    CATCH_ERR;

    status = GenerateJump(node, NT, GlobalNT, mark, num);
    CATCH_ERR;

    return status;
}

static size_t __IF_COUNTER__    = 0;
static size_t __WHILE_COUNTER__ = 0;

// DONE
int GenerateIf(Node_t *node, List_t *NT, List_t *GlobalNT)
{
    CHECK_NODES_N_LISTS;

    Node_t *decision   = node->right;
    CATCH_NULL(decision);

    Node_t *condition  = node->left;
    CATCH_NULL(condition);

    Node_t *if_stmts   = decision->left;
    CATCH_NULL(if_stmts);

    Node_t *else_stmts = decision->right;

    int counter = __IF_COUNTER__++;

    if (else_stmts)
    {
        status = GenerateCond(condition, NT, GlobalNT, "ELSE", counter);
        CATCH_ERR;

        status = GenerateStmts(if_stmts, NT, GlobalNT);
        CATCH_ERR;

        fprintf(out, "JMP END_IF_%d:\n", counter);

        fprintf(out, "ELSE_%d:\n", counter);

        status = GenerateStmts(else_stmts, NT, GlobalNT);
        CATCH_ERR;

        // fprintf(out, "JMP END_IF_%d:\n", counter);
    }
    else
    {
        status = GenerateCond(condition, NT, GlobalNT, "END_IF", counter);
        CATCH_ERR;

        status = GenerateStmts(if_stmts, NT, GlobalNT);
        CATCH_ERR;

        // fprintf(out, "JMP END_IF_%d:\n", counter);
    }

    fprintf(out, "END_IF_%d:\n", counter);

    return status;
}

// DONE
int GenerateReturn(Node_t *node, List_t *NT, List_t *GlobalNT)
{
    CHECK_NODES_N_LISTS;

    if (!NODE_KEYW(node, KEYW_RETURN))
    {
        status = ASMcmp::RETURN_ISNT_RETURN;
        CATCH_ERR;
    }

    status = GenerateExpr(node->right, NT, GlobalNT);
    CATCH_ERR;

    fprintf(out, "POP cx\n");

    fprintf(out, "RET\n");

    return status;
}

// DONE
int GenerateWhile(Node_t *node, List_t *NT, List_t *GlobalNT)
{
    CHECK_NODES_N_LISTS;

    Node_t *condition = node->left;
    CATCH_NULL(condition);

    Node_t *while_stmts = node->right;
    CATCH_NULL(while_stmts);

    int counter = __WHILE_COUNTER__++;

    fprintf(out, "WHILE_%d:\n", counter);

    status = GenerateCond(condition, NT, GlobalNT, "END_WHILE", counter);
    CATCH_ERR;

    fprintf(out, "DUMP\n");

    Tree_t tree = {};

    tree.root = while_stmts;

    TreeDump(&tree);

    status = GenerateStmts(while_stmts, NT, GlobalNT);
    CATCH_ERR;

    fprintf(out, "DUMP\n");

    fprintf(out, "JMP WHILE_%d:\n", counter);

    fprintf(out, "END_WHILE_%d:\n", counter);

    return status;
}

// DONE
int GenerateDefParams(Node_t *node, List_t *NT, List_t *GlobalNT, size_t *free_memory_index)
{
    CHECK_NODES_N_LISTS;
    if (free_memory_index == nullptr) return ASMcmp::PTR_IS_NULL;

    if (NT->size != 0)                return ASMcmp::NAMETABLE_ISNT_CLEAR;

    if (node->left)
    {
        PRINT_(NOT_IMPLEMENTED);

        return status;
    }
    else
    {
        *free_memory_index = 1;

        status = SearchInNametable(node->right, GlobalNT);
        if (status == ASMcmp::VARIABLE_FOUND) status = ASMcmp::VARIABLE_IS_ENGAGED;
        CATCH_ERR;

        status = PushInNametable(node->right, NT);
        CATCH_ERR;
    }

    return status;
}

// DONE
int IncreaseBX(const size_t number)
{
    if (out == nullptr) return ASMcmp::OUT_STREAM_IS_NULL;

    // fprintf(out, "        \n");
    fprintf(out, "PUSH bx\n");
    fprintf(out, "PUSH %lu\n", number);
    fprintf(out, "ADD\n");
    fprintf(out, "POP bx\n");
    // fprintf(out, "        \n");

    return ASMcmp::FUNC_IS_OK;
}

// DONE
int DecreaseBX(const size_t number)
{
    if (out == nullptr) return ASMcmp::OUT_STREAM_IS_NULL;

    // fprintf(out, "        \n");
    fprintf(out, "PUSH bx\n");
    fprintf(out, "PUSH %lu\n", number);
    fprintf(out, "SUB\n");
    fprintf(out, "POP bx\n");
    // fprintf(out, "        \n");

    return ASMcmp::FUNC_IS_OK;
}

// DONE
int GenerateMark(Node_t *mark)
{
    if (out == nullptr) return ASMcmp::OUT_STREAM_IS_NULL;
    int status = NodeVerify(mark);
    CATCH_ERR;

    if (mark->value->type == ID_TYPE || NODE_KEYW(mark, KEYW_MAIN1))
    {
        if (mark->value->type == ID_TYPE)
        {
            fprintf(out, "\n%s:\n", mark->value->arg.id);
            return status;
        }
        else
        if (KEYW(mark) == KEYW_MAIN1)
        {
            fprintf(out, "\nmain:\n");
            return status;
        }
    }

    return ASMcmp::CANT_USE_NON_ID_LIKE_MARK;
}

// DONE
int GenerateFuncDef(Node_t *node, List_t *NT, List_t *GlobalNT)
{
    CHECK_NODES_N_LISTS;

    if (NT->size != 0)
    {
        status = ASMcmp::NAMETABLE_ISNT_CLEAR;
        CATCH_ERR;
    }

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

    size_t free_memory_index = 0;

    status = GenerateMark(mark);
    CATCH_ERR;

    if (params)
    {
        status = GenerateDefParams(params, NT, GlobalNT, &free_memory_index);
        CATCH_ERR;
    }

    // bx += free_memory_index
    status = IncreaseBX(free_memory_index);
    CATCH_ERR;

    Node_t *stmts  = node->right;
    CATCH_NULL(stmts);


    status = GenerateStmts(stmts, NT, GlobalNT);
    CATCH_ERR;

    // bx -= free_memory_index
    status = DecreaseBX(free_memory_index);
    CATCH_ERR;

    // Node_t *ret = stmts->right;
    // if (ret && NODE_KEYW(ret, KEYW_RETURN))
    // {
    //     status = GenerateStmt(ret, NT, GlobalNT);
    //     CATCH_ERR;
    // }

    status = ListClear(NT);
    CATCH_ERR;

    return status;
}

// DONE
int GenerateMain(Node_t *node, List_t *NT, List_t *GlobalNT)
{
    CHECK_NODES_N_LISTS;

    if (NT->size != 0)
    {
        status = ASMcmp::NAMETABLE_ISNT_CLEAR;
        CATCH_ERR;
    }

    if (node_main == nullptr) return ASMcmp::NO_MAIN_IN_PROGRAM;

    Node_t *func = node->left;
    CATCH_NULL(func);

    Node_t *main = func->left;
    CATCH_NULL(main);

    status = IncreaseBX(GlobalNT->size);
    CATCH_ERR;

    status = GenerateMark(main);
    CATCH_ERR;

    assert(node);

    Node_t *stmts  = node->right;
    CATCH_NULL(stmts);

    status = GenerateStmts(stmts, NT, GlobalNT);
    CATCH_ERR;

    status = ListClear(NT);
    CATCH_ERR;

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
        fprintf(out, "PUSH [%lu]\n", index - 1);

        return status;
    }

    CATCH_ERR;

    status = ASMcmp::VARIABLE_NOT_FOUND;
    CATCH_ERR;

    return status;
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

    return status;
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

// DONE
int GenerateGS(Node_t *node, List_t *GlobalNT)
{
    if (out == nullptr) return ASMcmp::OUT_STREAM_IS_NULL;
    int status = NodeVerify(node);
    CATCH_ERR;
    status = ListVerify(GlobalNT);
    CATCH_ERR;

    if (!NODE_KEYW(node, KEYW_STMT)) return ASMcmp::GLOBAL_STMTS_ARE_NOT_STMTS;

    fprintf(out, "CALL main:\n");
    fprintf(out, "HLT\n");

    while (node->left) node = node->left;
    // IM AT BOTTOM STMT

    Node_t *bottom = node;

    // Processing global variables
    while (node)
    {
        if (KEYW(node->right) == KEYW_ASSIGN)
        {
            status = GenerateAssign(node, nullptr, GlobalNT);
            CATCH_ERR;
        }
        node = node->parent;
    }

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
    if (status == LIST_IS_DESTRUCTED) status = ASMcmp::FUNC_IS_OK;
    CATCH_ERR;

    free(NT);

    CATCH_ERR;

    return status;
}

// DONE
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
    if (status == LIST_IS_DESTRUCTED) status = ASMcmp::FUNC_IS_OK;
    CATCH_ERR;

    fclose(out);
    out = nullptr;

    free(GlobalNT);

    return status;
}
