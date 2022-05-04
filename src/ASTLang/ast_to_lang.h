#ifndef AST_TO_LANG_H
#define AST_TO_LANG_H

#include "../Asm/asm_comp.h"

int FillTree(const char *filename, Tree_t *tree);
int OpenFile(const char *filename, FILE **dest_stream, const char *mode);

int PoopLang(const char *filename, Tree_t *tree);
int PoopReturn    (Node_t *node);
int PoopPrint     (Node_t *node);
int PoopWhile     (Node_t *node);
int PoopLogOp     (Node_t *node);
int PoopCondition (Node_t *node);
int PoopIf        (Node_t *node);
int PoopStmt      (Node_t *node);
int PoopStmts     (Node_t *node);
int PoopCall      (Node_t *node);
int PoopCallParams(Node_t *node);
int PoopFuncDef   (Node_t *node);
int PoopDefParams (Node_t *node);
int PoopVar       (Node_t *node);
int PoopNum       (Node_t *node);
int PoopOper      (Node_t *node);
int PoopExpr      (Node_t *node);
int PoopAssign    (Node_t *node);
int PoopGlobStmts (Node_t *node);



namespace ASToLang
{
    enum Errors
    {
        FUNC_IS_OK              = 0,
        BAD_ALLOC               = 0xBADA110C,

        FILENAME_IS_NULL        = 0x900,
        INVALID_GLOB_STMT       = 0x901,
        OUT_STREAM_IS_NULL      = 0x902,
        UNEXPECTED_KEYW_IN_EXPR = 0x903,

        NON_OPER_KEYW           = 0x904,
        NON_LOG_OPER            = 0x905,
    };

}


#endif