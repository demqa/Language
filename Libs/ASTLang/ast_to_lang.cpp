#include "ast_to_lang.h"

// This file was created in exhausted condition. It is not really my code I believe.
// This is real shit. All the keywords are HARDCODED. Not a single structure in functions.

// place this in struct
static size_t SPACES_LEVEL = 0;

// place this in header
int PoopFuncDef(Node_t *node);
int PoopStmts(Node_t *node);
int PoopVar(Node_t *node);
int PoopGlobStmts(Node_t *node);

#define NODE_KEYW(NODE, KEYW) (NODE->value->type == KEYW_TYPE && NODE->value->arg.key_w == KEYW)
#define NODE_ID(NODE)         (NODE->value->type == ID_TYPE)
#define KEYW(NODE)           ((NODE->value->type == KEYW_TYPE) ? NODE->value->arg.key_w : 0)

static FILE *out = nullptr;

void PoopSpaces(size_t level)
{
    if (out == nullptr) return;
    for (size_t i = 0; i < level; i++)
    {
        fprintf(out, "    ");
    }
}

int PoopReturn(Node_t *node)
{
    if (out == nullptr) return ASToLang::OUT_STREAM_IS_NULL;
    int status = NodeVerify(node);
    CATCH_ERR;

    fprintf(out, "\n");
    PoopSpaces(SPACES_LEVEL);
    fprintf(out, "Fine "); PoopExpr(node->right);  fprintf(out, ";\n");

    return status;
}

int PoopPrint(Node_t *node)
{
    if (out == nullptr) return ASToLang::OUT_STREAM_IS_NULL;
    int status = NodeVerify(node);
    CATCH_ERR;

    PoopSpaces(SPACES_LEVEL);

    fprintf(out, "Forte "); PoopExpr(node->right); fprintf(out, ";\n");

    return status;
}

int PoopWhile(Node_t *node)
{
    if (out == nullptr) return ASToLang::OUT_STREAM_IS_NULL;
    int status = NodeVerify(node);
    CATCH_ERR;

    PoopSpaces(SPACES_LEVEL);

    fprintf(out, "Repetizione "); PoopCondition(node->left); fprintf(out, "\n");

    status = PoopStmts(node->right);
    CATCH_ERR;

    return status;
}

int PoopLogOp(Node_t *node)
{
    if (out == nullptr) return ASToLang::OUT_STREAM_IS_NULL;
    int status = NodeVerify(node);
    CATCH_ERR;

    switch (KEYW(node))
    {
        case KEYW_EQUAL:     fprintf(out, " == "); break;
        case KEYW_NOTEQUAL:  fprintf(out, " != "); break;
        case KEYW_GREATOREQ: fprintf(out, " >= "); break;
        case KEYW_GREAT:     fprintf(out, " >  "); break;
        case KEYW_LESS:      fprintf(out, " <  "); break;
        case KEYW_LESSOREQ:  fprintf(out, " <= "); break;
    
        default:
            status = ASToLang::NON_LOG_OPER;
            CATCH_ERR;
            break;
    }

    return status;
}

int PoopCondition(Node_t *node)
{
    if (out == nullptr) return ASToLang::OUT_STREAM_IS_NULL;
    int status = NodeVerify(node);
    CATCH_ERR;

    fprintf(out, "("); PoopExpr(node->left); PoopLogOp(node); PoopExpr(node->right); fprintf(out, ")");

    return status;
}

int PoopIf(Node_t *node)
{
    if (out == nullptr) return ASToLang::OUT_STREAM_IS_NULL;
    int status = NodeVerify(node);
    CATCH_ERR;

    PoopSpaces(SPACES_LEVEL);

    // why this is not KEYWords... I dont know
    fprintf(out, "Piacere "); PoopCondition(node->left); fprintf(out, " Deciso\n");

    status = PoopStmts(node->right);
    CATCH_ERR;

    return status;
}

int PoopStmt(Node_t *node)
{
    if (out == nullptr) return ASToLang::OUT_STREAM_IS_NULL;
    int status = NodeVerify(node);
    CATCH_ERR;

    switch (KEYW(node))
    {
        case KEYW_RETURN: status = PoopReturn(node); break;
        case KEYW_PRINT:  status = PoopPrint (node); break;
        case KEYW_WHILE:  status = PoopWhile (node); break;
        case KEYW_IF:     status = PoopIf    (node); break;
        case KEYW_ASSIGN: status = PoopAssign(node); break;
    
       default:
            break;
    }

    return status;
}

int PoopStmts(Node_t *node)
{
    if (out == nullptr) return ASToLang::OUT_STREAM_IS_NULL;
    int status = NodeVerify(node);
    CATCH_ERR;

    PoopSpaces(SPACES_LEVEL);
    fprintf(out, "{\n");

    SPACES_LEVEL++;

    while (node->left) node = node->left;

    while (NODE_KEYW(node, KEYW_STMT))
    {
        status = PoopStmt(node->right);
        CATCH_ERR;

        node = node->parent;
    }

    SPACES_LEVEL--;

    PoopSpaces(SPACES_LEVEL);
    fprintf(out, "}\n");

    return status;
}

int PoopCallParams(Node_t *node)
{
    if (out == nullptr) return ASToLang::OUT_STREAM_IS_NULL;
    int status = NodeVerify(node);
    CATCH_ERR;

    while (node->left) node = node->left;

    while (NODE_KEYW(node, KEYW_PARAM))
    {
        status = PoopExpr(node->right);
        CATCH_ERR;

        node = node->parent;
    }

    return status;
}

int PoopDefParams(Node_t *node)
{
    if (out == nullptr) return ASToLang::OUT_STREAM_IS_NULL;
    int status = NodeVerify(node);
    CATCH_ERR;

    while (node->left) node = node->left;

    while (NODE_KEYW(node, KEYW_PARAM))
    {
        status = PoopVar(node->right);
        CATCH_ERR;

        node = node->parent;
    }

    return status;
}

int PoopCall(Node_t *node)
{
    if (out == nullptr) return ASToLang::OUT_STREAM_IS_NULL;
    int status = NodeVerify(node);
    CATCH_ERR;

    status = PoopVar(node->left);
    CATCH_ERR;

    fprintf(out, "(");

    status = PoopCallParams(node->right);
    CATCH_ERR;

    fprintf(out, ")");

    return status;
}

int PoopFuncDef(Node_t *node)
{
    if (out == nullptr) return ASToLang::OUT_STREAM_IS_NULL;
    int status = NodeVerify(node);
    CATCH_ERR;

    if (node->left && node->left->left && KEYW(node->left->left) == KEYW_MAIN1)
    {
        fprintf(out, "Danse Macabre\n");
    }
    else
    {
        fprintf(out, "Opus %s(", node->left->left->value->arg.id);
        status = PoopDefParams(node->right);
        CATCH_ERR;
        fprintf(out, ")\n");
    }
    
    PoopSpaces(SPACES_LEVEL);
    status = PoopStmts(node->right);

    fprintf(out, "\n");

    return status;
}

int PoopVar(Node_t *node)
{
    if (out == nullptr) return ASToLang::OUT_STREAM_IS_NULL;
    int status = NodeVerify(node);
    CATCH_ERR;

    fprintf(out, "%s", node->value->arg.id);

    return status;
}

int PoopNum(Node_t *node)
{
    if (out == nullptr) return ASToLang::OUT_STREAM_IS_NULL;
    int status = NodeVerify(node);
    CATCH_ERR;

    assert(node->value->type == NUM_TYPE);

    fprintf(out, "%lg", node->value->arg.num);

    return status;
}

int PoopOper(Node_t *node)
{
    if (out == nullptr) return ASToLang::OUT_STREAM_IS_NULL;
    int status = NodeVerify(node);
    CATCH_ERR;

    switch (KEYW(node))
    {
        case KEYW_ADD: fprintf(out, "+"); break;
        case KEYW_SUB: fprintf(out, "-"); break;
        case KEYW_MUL: fprintf(out, "*"); break;
        case KEYW_DIV: fprintf(out, "/"); break;
        case KEYW_POW: fprintf(out, "^"); break;
        
        default:
            status = ASToLang::NON_OPER_KEYW;
            CATCH_ERR;
            break;
    }

    return status;
}

int PoopExpr(Node_t *node)
{
    if (out == nullptr) return ASToLang::OUT_STREAM_IS_NULL;
    int status = NodeVerify(node);
    CATCH_ERR;
    
    fprintf(out, "(");

    if (NODE_KEYW(node, KEYW_CALL))
    {
        status = PoopCall(node);
        CATCH_ERR;
    }
    else
    if (KEYW(node) >= KEYW_ADD && KEYW(node) <= KEYW_POW)
    {
        status = PoopExpr(node->left);
        CATCH_ERR;

        status = PoopOper(node);
        CATCH_ERR;

        status = PoopExpr(node->right);
        CATCH_ERR;
    }
    else
    if (node->value->type == ID_TYPE)
    {
        status = PoopVar(node);
        CATCH_ERR;
    }
    else
    if (node->value->type == NUM_TYPE)
    {
        status = PoopNum(node);
        CATCH_ERR;
    }
    else
    if (NODE_KEYW(node, KEYW_ASSIGN))
    {
        status = PoopAssign(node);
        CATCH_ERR;
    }
    else
    {
        status = ASToLang::UNEXPECTED_KEYW_IN_EXPR;
        CATCH_ERR;
    }

    fprintf(out, ")");

    return status;
}

int PoopAssign(Node_t *node)
{
    if (out == nullptr) return ASToLang::OUT_STREAM_IS_NULL;
    int status = NodeVerify(node);
    CATCH_ERR;

    PoopSpaces(SPACES_LEVEL);

    fprintf(out, "%s = ", node->left->value->arg.id);

    PoopExpr(node->right);

    fprintf(out, ";\n");

    return status;
}

int PoopGlobStmts(Node_t *node)
{
    int status = NodeVerify(node);
    CATCH_ERR;

    while (node->left) node = node->left;

    status = NodeVerify(node);
    CATCH_ERR;

    while (node)
    {
        if (NODE_KEYW(node->right, KEYW_DEFINE))
        {
            status = PoopFuncDef(node->right);
            CATCH_ERR;
        }
        else
        if (NODE_KEYW(node->right, KEYW_ASSIGN))
        {
            status = PoopAssign(node->right);
            CATCH_ERR;
        }
        else
        {
            return ASToLang::INVALID_GLOB_STMT;
        }
        node = node->parent;
    }

    return status;
}

// DONE
int PoopLang(const char *filename, Tree_t *tree)
{
    int status = TreeVerify(tree);
    if (status) return status;
    if (filename   == nullptr) return ASToLang::FILENAME_IS_NULL;

    FILE *stream = nullptr;
    status = OpenFile(filename, &stream, "w");
    CATCH_ERR;

    out = stream;

    TreeDump(tree);

    Node_t *GlobalStmts = tree->root;
    status = NodeVerify(GlobalStmts);
    CATCH_ERR;

    status = PoopGlobStmts(GlobalStmts);
    CATCH_ERR;

    fclose(stream);

    out = nullptr;

    return status;
}
