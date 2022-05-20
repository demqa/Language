#include "asm_comp.h"

// rework all the backend I have there
// there is so much issues.
// I thought that these scopes that I did there with my NameTable is nice, but this is real BULLSHIT,
// because it does not work. So next thing is that I can use some struct Backend to deal with different
// functions, but I have not done it. So that why code in this file is shit, despite the beauty.

#define NODE_KEYW(NODE, KEYW) (NODE->value->type == KEYW_TYPE && NODE->value->arg.key_w == KEYW)
#define NODE_ID(NODE) (NODE->value->type == ID_TYPE)
#define KEYW(NODE) ((NODE->value->type == KEYW_TYPE) ? NODE->value->arg.key_w : 0)

#define CHECK_NODES status = NodeVerify(node); CATCH_ERR

// DEBUG TREE
Tree_t tr33 = {};
#define DUMP(NODE) do { tr33.root = NODE; TreeDump(&tr33); } while (0)

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

int __EmitDword(Backend *back, uint32_t dword) // this function is usually used to store dw address
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

int __EmitQword(Backend *back, uint64_t qword) // this function is usually used to store qword immediate
{
    int status = BackendVerify(back);
    CATCH_ERR;

    if (back->buff_ptr >= back->buff_size)
    {
        PRINT(END_OF_BUFFER_CANT_EMIT);
        return ASMcmp::BUFF_OVERFLOW;
    }

    *((uint64_t *)(back->code_buff + back->buff_ptr)) = qword;
    back->buff_ptr += 8;

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

// DONE
int __EmitLog(Backend *back, const char *msg)
{
    int status = BackendVerify(back);
    CATCH_ERR;

    if (msg == nullptr)
    {
        PRINT(MSG_IS_NULLPTR);
        return ASMcmp::MSG_IS_NULLPTR;
    }

    fprintf(back->asm_log, "    %s\n", msg);

    return status;
}

// DONE
int __EmitLogFmt(Backend *back, const char *fmt, ...)
{
    int status = BackendVerify(back);
    CATCH_ERR;

    if (fmt == nullptr)
    {
        PRINT(FMT_IS_NULLPTR);
        return ASMcmp::FMT_IS_NULLPTR;
    }

    va_list args = {};
    va_start(args, fmt);

     fprintf(back->asm_log, "    ");
    vfprintf(back->asm_log, fmt, args);
     fprintf(back->asm_log, "\n");

    va_end(args);

    return status;
}


#define __VA_COUNT__(...)    (sizeof ((int[]) {0, ##__VA_ARGS__}) / sizeof (int) - 1)
#define EmitBytes(...)       do { __EmitBytes(back, __VA_COUNT__(__VA_ARGS__), __VA_ARGS__); } while (0)
#define EmitDword(ARG)       do { __EmitDword(back, ARG);                                    } while (0)
#define EmitQword(ARG)       do { __EmitQword(back, ARG);                                    } while (0)

int __EmitLabelKeyw(Backend *back, int keyword_id, size_t counter, int labels_id)
{
    int status = BackendVerify(back);
    CATCH_ERR;

    Val_t value = {};
    value.type = Keyword_t;

    value.elem.keyword_id = keyword_id;
    value.elem.number     = counter;
    value.hash            = HashCRC32(&value.elem, sizeof(value.elem));

    value.offset          = back->buff_ptr;

    if (labels_id == DST_ID)
    {
        status = ListPushBack(back->LabelsDst, value);
        CATCH_ERR;
        EmitDword(0);
    }
    else
    if (labels_id == SRC_ID)
    {
        status = ListPushBack(back->LabelsSrc, value);
        CATCH_ERR;
    }
    else
    {
        status = ASMcmp::INVALID_LABEL_ID;
        CATCH_ERR;
    }

    return status;
}

int __EmitLabelCall(Backend *back, const char *func_ptr, int labels_id)
{
    int status = BackendVerify(back);
    CATCH_ERR;
    if (func_ptr == nullptr){ status = ASMcmp::PTR_IS_NULL; CATCH_ERR; }

    Val_t value = {};
    value.type = Function_t;

    value.hash = HashCRC32((void *) func_ptr, strlen(func_ptr));
    value.elem.func_ptr =  (char *) func_ptr;

    value.offset = back->buff_ptr;

    if (labels_id == DST_ID)
    {
        status = ListPushBack(back->LabelsDst, value);
        CATCH_ERR;
        EmitDword(0);
    }
    else
    if (labels_id == SRC_ID)
    {
        status = ListPushBack(back->LabelsSrc, value);
        CATCH_ERR;
    }
    else
    {
        status = ASMcmp::INVALID_LABEL_ID;
        CATCH_ERR;
    }

    return status;
}

#define EmitDstCall(FUNC)    do { __EmitLabelCall(back, FUNC,    DST_ID);                    } while (0)
#define EmitSrcCall(FUNC)    do { __EmitLabelCall(back, FUNC,    SRC_ID);                    } while (0)
#define EmitDstKeyw(KEYW, N) do { __EmitLabelKeyw(back, KEYW, N, DST_ID);                    } while (0)
#define EmitSrcKeyw(KEYW, N) do { __EmitLabelKeyw(back, KEYW, N, SRC_ID);                    } while (0)

#define EmitLog(MSG)         do { __EmitLog(back, MSG);                                      } while (0)
#define EmitLogFmt(FMT, ...) do { __EmitLogFmt(back, FMT, __VA_ARGS__);                      } while (0)

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

int EmitStackFrame(Node_t *node, Backend *back);
int EmitMark      (Node_t *node, Backend *back);

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

// returns ASMcmp::VARIABLE_FOUND if found
// returns 0 if not found
size_t SearchVariable(Node_t *node, Backend *back)
{
    int status = BackendVerify(back);
    CATCH_ERR;
    CHECK_NODES;

    if (!IsVar(node))
    {
        status = ASMcmp::IS_NOT_A_VARIABLE;
        CATCH_ERR;
    }

    int64_t hash = HashCRC32(node->value->arg.id, WORD_MAX_LEN);

    // Now I think that collision chance is equal to zero, so
    // I will compare only hashes.
    size_t index = ListValueIndex(back->NT, hash);

    return index;
}

int PushVariable(Node_t *node, Backend *back, int64_t offset)
{
    int status = BackendVerify(back);
    CATCH_ERR;
    CHECK_NODES;

    int64_t hash = HashCRC32(node->value->arg.id, WORD_MAX_LEN);

    List_t *NT   = back->NT;

    Val_t value = {};
    value.type  = Variable_t;
    value.hash  = hash;

    // TODO: make global variables, but now it is shit
    value.offset        = offset;
    value.elem.variable = node->value->arg.id;

    status = ListPushBack(NT, value);
    CATCH_ERR;

    return status;
}

int RemoveVariables(Backend *back, size_t number_of_variables)
{
    int status = BackendVerify(back);
    CATCH_ERR;

    for (size_t index = 0; index < number_of_variables; ++index)
    {
        Val_t value = ListPopBack(back->NT);
        if (value.type == DEAD_VALUE.type)
        {
            status = ASMcmp::DEAD_VALUE_REMOVED;
            CATCH_ERR;
        }
    }

    return status;
}

// DONE
int EmitStmt(Node_t *node, Backend *back)
{
    int status = NodeVerify(node);
    CATCH_ERR;
    CHECK_NODES;

    switch (KEYW(node))
    {
        case KEYW_ASSIGN: status = EmitAssign(node, back); break;
        case KEYW_IF:     status = EmitIf    (node, back); break;
        case KEYW_WHILE:  status = EmitWhile (node, back); break;
        case KEYW_CALL:   status = EmitCall  (node, back); break;
        case KEYW_RETURN: status = EmitReturn(node, back); break;
        // case KEYW_SCAN:   status = EmitScan  (node, back); break;
        // case KEYW_PRINT:  status = EmitPrint (node, back); break;
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
    CHECK_NODES;

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
// int EmitGlobExpr  (Node_t *node, Backend *back);

int EmitMark      (Node_t *node);

int EmitSysHeader(Backend *back);
int EmitGS       (Node_t *node, Backend *back);
int EmitStdLib   (Node_t *node, Backend *back);

int EmitASM      (const char *filename, Tree_t *tree);

int EmitSavingRegisters(Backend *back);
int EmitRestoringRegisters(Backend *back);

int EmitSavingRegisters(Backend *back)
{
    int status = BackendVerify(back);
    CATCH_ERR;

    EmitLog("push rcx"); EmitBytes(PUSH | RCX);
    EmitLog("push rbx"); EmitBytes(PUSH | RBX);

    return status;
}

int EmitRestoringRegisters(Backend *back)
{
    int status = BackendVerify(back);
    CATCH_ERR;

    EmitLog("pop rbx"); EmitBytes(POP | RBX);
    EmitLog("pop rcx"); EmitBytes(POP | RCX);

    return status;
}

int EmitJump(Node_t *node, Backend *back, int keyword_id, size_t counter)
{
    int status = BackendVerify(back);
    CATCH_ERR;
    CHECK_NODES;

    if (!IsLogOper(node) && !NODE_KEYW(node, KEYW_COMMA))
    {
        status = ASMcmp::NOT_LOGICAL_OPER;
        CATCH_ERR;
    }

    const char *jump_type[] = {"jmp", "jae", "ja", "je", "jne", "jb", "jbe"};

    size_t jump_index = 0;
    int    jump_code  = 0;

    switch (KEYW(node))
    {
        case KEYW_LESS:      jump_index = 1; jump_code = JAE; break;
        case KEYW_LESSOREQ:  jump_index = 2; jump_code = JA;  break;
        case KEYW_NOTEQUAL:  jump_index = 3; jump_code = JE;  break;
        case KEYW_EQUAL:     jump_index = 4; jump_code = JNE; break;
        case KEYW_GREATOREQ: jump_index = 5; jump_code = JB;  break;
        case KEYW_GREAT:     jump_index = 6; jump_code = JBE; break;

        case KEYW_COMMA:     jump_index = 0; jump_code = JMP; break;

        default:
            status = ASMcmp::UNDEFINED_OPERATOR;
            CATCH_ERR;
            break;
    }

    const char * appendix[] = {"if", "else", "if_end", "while", "while_end"};

    size_t appendix_index = 0;

    switch (keyword_id)
    {
        case Keyword_if:        appendix_index = 0; break;
        case Keyword_else:      appendix_index = 1; break;
        case Keyword_if_end:    appendix_index = 2; break;

        case Keyword_while:     appendix_index = 3; break;
        case Keyword_while_end: appendix_index = 4; break;

        default:
            status = ASMcmp::INVALID_JUMP_KEYW_ID;
            CATCH_ERR;
            break;
    }

    if (jump_code == JMP) EmitBytes(JMP);
    else                  EmitBytes(JCOND, jump_code);

    EmitDstKeyw(keyword_id, counter);

    EmitLogFmt("%s %s_%lu", jump_type[jump_index], appendix[appendix_index], counter);
    EmitLog("\n");

    return status;
}

int EmitCond(Node_t *node, Backend *back, int keyword_id, size_t counter)
{
    int status = BackendVerify(back);
    CATCH_ERR;
    CHECK_NODES;

    status = EmitExpr(node->left,  back);
    CATCH_ERR;
    EmitLog("mov rcx, rax"); EmitBytes(REX_W, MOV_RM, MOD_RR | RCX_DST | RAX_SRC);

    status = EmitExpr(node->right, back);
    CATCH_ERR;
    EmitLog("mov rbx, rax"); EmitBytes(REX_W, MOV_RM, MOD_RR | RBX_DST | RAX_SRC);

    EmitLog("cmp rcx, rbx"); EmitBytes(REX_W, CMP_RM, MOD_RR | RCX_DST | RBX_SRC);
    status = EmitJump(node, back, keyword_id, counter);
    CATCH_ERR;

    return status;
}

int EmitIf(Node_t *node, Backend *back)
{
    int status = BackendVerify(back);
    CATCH_ERR;
    CHECK_NODES;

    Node_t *decision   = node->right;
    CATCH_NULL(decision);

    Node_t *condition  = node->left;
    CATCH_NULL(condition);

    Node_t *if_stmts   = decision->left;
    CATCH_NULL(if_stmts);

    Node_t *else_stmts = decision->right;

    size_t counter = ++back->if_counter;

    EmitLogFmt("\n;; empty mark IF number %lu", counter);

    if (else_stmts)
    {
        status = EmitCond(condition, back, Keyword_else, counter);
        CATCH_ERR;

        status = EmitStmts(if_stmts, back);
        CATCH_ERR;

        // NOTE: very nice trick... )
        if_stmts->value->arg.key_w = KEYW_COMMA;
        status = EmitJump(if_stmts, back, Keyword_if_end, counter);
        CATCH_ERR;

        EmitSrcKeyw(Keyword_else, counter);
        EmitLogFmt("\nelse_%lu:", counter);

        status = EmitStmts(else_stmts, back);
        CATCH_ERR;
    }
    else
    {
        status = EmitCond(condition, back, Keyword_if_end, counter);
        CATCH_ERR;

        status = EmitStmts(if_stmts, back);
        CATCH_ERR;
    }

    EmitSrcKeyw(Keyword_if_end, counter);
    EmitLogFmt("\nif_end_%lu:", counter);

    return status;
}

int EmitCallParams(Node_t *node, Backend *back)
{
    int status = BackendVerify(back);
    CATCH_ERR;

    if (node == nullptr) return status;

    if (!NODE_KEYW(node, KEYW_PARAM))
    {
        status = ASMcmp::INVALID_CALL_PARAMS;
        CATCH_ERR;
    }

    while (node != nullptr)
    {
        status = EmitExpr(node->right, back);
        CATCH_ERR;

        EmitLog("push rax"); EmitBytes(PUSH | RAX);

        node = node->left;
    }

    return status;
}

int EmitPopParams(Node_t *node, Backend *back)
{
    int status = BackendVerify(back);
    CATCH_ERR;

    while (node)
    {
        EmitLog("pop rbx"); EmitBytes(POP | RBX);

        node = node->left;
    }

    return status;
}

int EmitCall(Node_t *node, Backend *back)
{
    int status = BackendVerify(back);
    CATCH_ERR;
    CHECK_NODES;

    int64_t IM8 = back->number_of_locals * 8;
    if (IM8 > 127 || IM8 < -128)
    {
        status = ASMcmp::ARG_OVERFLOW;
        CATCH_ERR;
    }

    EmitLogFmt("sub rsp, %ld", IM8); EmitBytes(REX_W, SUB_RI8, MOD_RR | 0b00000000 | RSP_DST, (int) IM8);
    EmitLog("");

    status = EmitSavingRegisters(back);
    CATCH_ERR;

    Node_t *params = node->right;

    if (params != nullptr) EmitLog("\n");

    status = EmitCallParams(params, back);
    CATCH_ERR;

    EmitLog("\n");

    Node_t *name = node->left;
    CATCH_NULL(name); assert(NODE_ID(name));

    EmitLogFmt("call %s", name->value->arg.id); EmitBytes(CALL); EmitDstCall(name->value->arg.id);
    EmitLog("\n");

    status = EmitPopParams(params, back);
    CATCH_ERR;

    EmitLog("\n");

    status = EmitRestoringRegisters(back);
    CATCH_ERR;

    EmitLog("");
    EmitLogFmt("add rsp, %ld", IM8); EmitBytes(REX_W, ADD_RI8, MOD_RR | 0b00101000 | RSP_DST, (int) IM8);
    EmitLog("\n");

    return status;
}

int EmitWhile(Node_t *node, Backend *back)
{
    int status = BackendVerify(back);
    CATCH_ERR;
    CHECK_NODES;

    Node_t *condition = node->left;
    CATCH_NULL(condition);

    Node_t *while_stmts = node->right;
    CATCH_NULL(while_stmts);

    int counter = ++back->while_counter;

    EmitSrcKeyw(Keyword_while, counter);
    EmitLogFmt("\nwhile_%lu:", counter);

    status = EmitCond(condition, back, Keyword_while_end, counter);
    CATCH_ERR;

    status = EmitStmts(while_stmts, back);
    CATCH_ERR;

    while_stmts->value->arg.key_w = KEYW_COMMA;
    status = EmitJump(while_stmts, back, Keyword_while, counter);
    CATCH_ERR;

    EmitSrcKeyw(Keyword_while_end, counter);
    EmitLogFmt("\nwhile_end_%lu:", counter);

    return status;
}

// NOTE: this function is called only when params are not empty.
int EmitDefParams(Node_t *node, Backend *back)
{
    int status = BackendVerify(back);
    CATCH_ERR;
    CHECK_NODES;

    // TODO: delete it when I will make globals
    if (back->NT->size != 0)
    {
        // ListDump(back->NT);
        status = ASMcmp::NAMETABLE_ISNT_CLEAR;
        CATCH_ERR;
    }

    int number_of_params = 1;
    for ( ; node->left; node = node->left, ++number_of_params);

    // pushing parameters in nametable in direct order
    while (NODE_KEYW(node, KEYW_PARAM))
    {
        assert(node->right != nullptr);

        size_t index = SearchVariable(node->right, back);
        if (index != 0) status = ASMcmp::VARIABLE_IS_ENGAGED;
        CATCH_ERR;

        int64_t offset = 16 + 8 * (back->NT->size - back->number_of_locals);

        status = PushVariable(node->right, back, offset);
        CATCH_ERR;

        assert(node->parent != nullptr);
        node = node->parent;
    }

    back->number_of_params = number_of_params;

    return status;
}

// TODO: one day I will add global variables
// DONE
int EmitAssign(Node_t *node, Backend *back)
{
    int status = BackendVerify(back);
    CATCH_ERR;
    CHECK_NODES;

    if (!NODE_KEYW(node, KEYW_ASSIGN))
    {
        status = ASMcmp::INVALID_ASSIGN;
        CATCH_ERR;
    }

    int64_t offset = 0;
    size_t index = SearchVariable(node->left, back);
    if (index != 0)
        offset = back->NT->data[index].value.offset;
    else
        offset = -8 + (back->number_of_locals++) * -8;

    status = EmitExpr(node->right, back);
    CATCH_ERR;

    if (offset < 0)
        EmitLogFmt("mov [rbp%ld], rax",  offset);
    else
        EmitLogFmt("mov [rbp+%ld], rax", offset);

    EmitBytes(REX_W, MOV_RM, RBP_MR, (int) offset);

    EmitLog("\n");

    if (index == 0)
    {
        status = PushVariable(node->left, back, offset);
        CATCH_ERR;
    }

    return status;
}

int EmitMathOper(Node_t *node, Backend *back)
{
    int status = BackendVerify(back);
    CATCH_ERR;
    CHECK_NODES;

    if (!IsMathOper(node))
    {
        status = ASMcmp::IS_NOT_A_MATH_OPER;
        CATCH_ERR;
    }

    EmitLog("mov rax, rcx"); EmitBytes(REX_W, MOV_RM, MOD_RR | RAX_DST | RCX_SRC);

    switch (KEYW(node))
    {
        case KEYW_ADD: EmitLog("add rax, rbx"); EmitBytes(REX_W, ADD_RM, MOD_RR | RAX_SRC | RBX_DST); break;
        case KEYW_SUB: EmitLog("sub rax, rbx"); EmitBytes(REX_W, SUB_RM, MOD_RR | RAX_SRC | RBX_DST); break;
        case KEYW_MUL: EmitLog("imul rbx");     EmitBytes(REX_W, MUL_RM, MOD_RR | IMUL_AP | RBX_DST); break;
        case KEYW_DIV: EmitLog("idiv rbx");     EmitBytes(REX_W, DIV_RM, MOD_RR | IDIV_AP | RBX_DST); break;

        // case KEYW_POW: return EmitPow(node);

        default:
            status = ASMcmp::UNEXPECTED_MATH_OPER;
            CATCH_ERR;
            break;
    }

    EmitLog("\n");

    return status;
}

int EmitNum(Node_t *node, Backend *back)
{
    int status = BackendVerify(back);
    CATCH_ERR;
    CHECK_NODES;

    if (!IsNum(node))
    {
        status = ASMcmp::IS_NOT_A_NUMBER;
        CATCH_ERR;
    }

    EmitLogFmt("mov rax, %ld", node->value->arg.num); EmitBytes(REX_W, MOV_RI); EmitQword(node->value->arg.num);

    return status;
}

int EmitVar(Node_t *node, Backend *back)
{
    int status = BackendVerify(back);
    CATCH_ERR;
    CHECK_NODES;

    if (!IsVar(node))
    {
        status = ASMcmp::IS_NOT_A_VARIABLE;
        CATCH_ERR;
    }

    size_t index = SearchVariable(node, back);
    if (index == 0)
    {
        DUMP(node);
        status = ASMcmp::VARIABLE_NOT_FOUND;
        CATCH_ERR;
    }

    int64_t offset = back->NT->data[index].value.offset;
    if (offset < 0)
        EmitLogFmt("mov rax, [rbp%ld]", offset);
    else
        EmitLogFmt("mov rax, [rbp+%ld]", offset);

    EmitBytes(REX_W, MOV_MR, RBP_MR, (int) offset);

    return status;
}

int EmitExpr(Node_t *node, Backend *back)
{
    int status = BackendVerify(back);
    CATCH_ERR;
    CHECK_NODES;

    if (KEYW(node) == KEYW_CALL)
    {
        status = EmitCall(node, back);
        CATCH_ERR;

        return status;
    }

    if (node->left && node->right)
    {
        status = EmitExpr(node->left, back);
        CATCH_ERR;

        EmitLog("mov rcx, rax"); EmitBytes(REX_W, MOV_RM, MOD_RR | RCX_DST | RAX_SRC);

        status = EmitExpr(node->right, back);
        CATCH_ERR;

        EmitLog("mov rbx, rax"); EmitBytes(REX_W, MOV_RM, MOD_RR | RBX_DST | RAX_SRC);

        if (!IsMathOper(node))
        {
            status = ASMcmp::INVALID_EXPRESSION;
            CATCH_ERR;
        }

        status = EmitMathOper(node, back);
        CATCH_ERR;
    }
    else
    {
        assert(node->left == nullptr && node->right == nullptr);

        if (IsNum(node))
        {
            status = EmitNum(node, back);
            CATCH_ERR;
        }
        else
        if (IsVar(node))
        {
            status = EmitVar(node, back);
            CATCH_ERR;
        }
        else
        {
            status = ASMcmp::UNDEFINED_OPERATOR;
            CATCH_ERR;
        }
    }

    return status;
}

int EmitReturn(Node_t *node, Backend *back)
{
    int status = BackendVerify(back);
    CATCH_ERR;
    CHECK_NODES;

    if (!NODE_KEYW(node, KEYW_RETURN))
    {
        status = ASMcmp::RETURN_ISNT_RETURN;
        CATCH_ERR;
    }

    status = EmitExpr(node->right, back);
    CATCH_ERR;

    EmitLog("pop rbp"); EmitBytes(POP | RBP);
    EmitLog("ret");     EmitBytes(RET);

    return status;
}

int EmitStackFrame(Backend *back)
{
    int status = BackendVerify(back);
    CATCH_ERR;

    EmitLog("push rbp");     EmitBytes(PUSH | RBP);
    EmitLog("mov rbp, rsp"); EmitBytes(REX_W, MOV_RM, MOD_RR | RBP_DST | RSP_SRC);
    EmitLog("\n");

    return status;
}

int EmitMark(Node_t *node, Backend *back)
{
    int status = BackendVerify(back);
    CATCH_ERR;
    CHECK_NODES;

    if (NODE_ID(node))
    {
        EmitLogFmt("\n%s:", node->value->arg.id); EmitSrcCall(node->value->arg.id);
        return status;
    }
    else
    if (NODE_KEYW(node, KEYW_MAIN1))
    {
        EmitLog("\nmain:"); EmitSrcCall("main");
        return status;
    }

    PRINT(ERROR_INVALID_TYPE_FOR_MARK);
    return ASMcmp::INVALID_TYPE_FOR_MARK;
}

int EmitFuncDef(Node_t *node, Backend *back)
{
    int status = BackendVerify(back);
    CATCH_ERR;
    CHECK_NODES;

    Node_t *func = node->left;
    CATCH_NULL(func);

    Node_t *mark = func->left;
    CATCH_NULL(mark);

    if (KEYW(func->left) == KEYW_MAIN1)
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

    status = EmitMark(mark, back);
    CATCH_ERR;

    Node_t *params = func->right;

    if (params != nullptr)
    {
        status = EmitDefParams(params, back);
        CATCH_ERR;
    }

    Node_t *stmts  = node->right;
    CATCH_NULL(stmts);

    status = EmitStackFrame(back);
    CATCH_ERR;

    status = EmitStmts(stmts, back);
    CATCH_ERR;

    status = RemoveVariables(back, back->number_of_params + back->number_of_locals);
    CATCH_ERR;

    back->number_of_params = 0;
    back->number_of_locals = 0;

    return status;
}

int EmitMain(Node_t *node, Backend *back)
{
    int status = BackendVerify(back);
    CATCH_ERR;
    CHECK_NODES;

    if (back->main == nullptr)
    {
        status = ASMcmp::NO_MAIN_IN_PROGRAM;
        CATCH_ERR;
    }

    Node_t *func = node->left;
    CATCH_NULL(func);

    Node_t *main = func->left;
    CATCH_NULL(main);

    status = EmitMark(main, back);
    CATCH_ERR;

    Node_t *stmts  = node->right;
    CATCH_NULL(stmts);

    status = EmitStackFrame(back);
    CATCH_ERR;

    status = EmitStmts(stmts, back);
    CATCH_ERR;

    status = RemoveVariables(back, back->number_of_locals);
    CATCH_ERR;

    back->number_of_locals = 0;

    return status;
}

int EmitGS(Node_t *node, Backend *back)
{
    int status = BackendVerify(back);
    CATCH_ERR;
    CHECK_NODES;

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

int BufferDump(Backend *back)
{
    int status = BackendVerify(back);
    CATCH_ERR;

    static int number = 0;

    fprintf(stderr, "\n------------BUFFER DUMP BEG №%d-------------\n", number);

    for (size_t index = 0; index < back->buff_ptr; ++index)
    {
        fprintf(stderr, "%02x", back->code_buff[index]);

        if (index % 4  == 0) fprintf(stderr, "  ");

        if (index % 16 == 0) fprintf(stderr, "\n");
    }

    fprintf(stderr, "\n------------BUFFER DUMP END №%d-------------\n", number);

    number++;

    return status;
}

int EmitElf(Backend *back)
{
    int status = BackendVerify(back);
    CATCH_ERR;

    //  Now there is no translation into elf. So.

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

    status = mprotect(code, pagesize, PROT_EXEC | PROT_WRITE | PROT_READ);
    if (status)
    {
        PRINT(MPROTECT_RETURNED_SHIT);
        return ASMcmp::MPROTECT_FAILED;
    }

    BufferDump(back);

    int x = ((int(*)())code)();
    PRINT_X(x);

    return status;
}

int EmitSysHeader(Backend *back)
{
    int status = BackendVerify(back);
    CATCH_ERR;

    // NOTE: these commands are only
    // for compilation with NASM.
    EmitLog("\nGLOBAL _start\n");
    EmitLog("SECTION .text");
    EmitLog("\n_start:");

    EmitLog("call main"); EmitBytes(CALL); EmitDstCall("main");


    EmitLog("\n");
    EmitLog("mov rax, 0x3C"); EmitBytes(MOV_RI); EmitDword(0x3C);
    EmitLog("xor rdi, rdi");  EmitBytes(REX_W, XOR_RM, MOD_RR | RDI_DST | RDI_SRC);
    EmitLog("syscall");       EmitBytes(0x0F, 0x05);

    EmitLog("\n;;--------------------------\n");

    return status;
}

int EmitLabels(Backend *back)
{
    int status = BackendVerify(back);
    CATCH_ERR;

    size_t elem = back->LabelsDst->front;

    while (elem)
    {
        size_t index = ListValueIndex(back->LabelsSrc, back->LabelsDst->data[elem].value.hash);
        if (index == 0)
        {
            status = ASMcmp::CANT_FIND_LABEL;
            CATCH_ERR;
        }

        int64_t dst_offset = back->LabelsDst->data[elem].value.offset;

        int64_t src_offset = back->LabelsSrc->data[index].value.offset;

        *((int32_t *) (back->code_buff + dst_offset)) = src_offset - dst_offset - 4;

        elem = ListNext(back->LabelsDst, elem);
    }

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

    back->code_buff = (uint8_t *) calloc(back->buff_size, sizeof(uint8_t));
    if (back->code_buff == nullptr) return ASMcmp::BAD_ALLOC;

    back->asm_log = fopen(filename, "w");
    if (back->asm_log == nullptr)
    {
        PRINT(FILE_CANT_BE_OPENED);
        return ASMcmp::FILE_CANT_BE_OPENED;
    }

    back->LabelsDst = (List_t *) calloc(1, sizeof(List_t));
    if (back->LabelsDst == nullptr) return ASMcmp::BAD_ALLOC;

    back->LabelsSrc = (List_t *) calloc(1, sizeof(List_t));
    if (back->LabelsSrc == nullptr) return ASMcmp::BAD_ALLOC;

    back->NT        = (List_t *) calloc(1, sizeof(List_t));
    if (back->NT        == nullptr) return ASMcmp::BAD_ALLOC;

    status = ListCtor(back->NT, ASMcmp::INITIAL_CAPACITY);
    CATCH_ERR;

    status = ListCtor(back->LabelsDst, ASMcmp::INITIAL_CAPACITY);
    CATCH_ERR;

    status = ListCtor(back->LabelsSrc, ASMcmp::INITIAL_CAPACITY);
    CATCH_ERR;


    status = EmitSysHeader(back);
    CATCH_ERR;

    BufferDump(back);

    status = EmitGS(tree->root, back);
    CATCH_ERR;

    BufferDump(back);

    status = EmitLabels(back);
    CATCH_ERR;

    BufferDump(back);

    status = EmitElf(back);
    CATCH_ERR;


    status = ListDtor(back->NT);
    if (status == LIST_IS_DESTRUCTED) status = ASMcmp::FUNC_IS_OK;
    CATCH_ERR;

    status = ListDtor(back->LabelsDst);
    if (status == LIST_IS_DESTRUCTED) status = ASMcmp::FUNC_IS_OK;
    CATCH_ERR;

    status = ListDtor(back->LabelsSrc);
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
#undef EmitDword
#undef EmitLog

#undef NODE_KEYW
#undef NODE_ID
#undef KEYW
