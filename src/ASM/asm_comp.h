#ifndef ASM_COMP_H
#define ASM_COMP_H

#include "../LexemAnalyzer/lex_anal.h"
#include "../List/list.h"
#include "../ELF/elf_gen.h"

#include <math.h>
#include <stdint.h>

#include <sys/mman.h>
#include <unistd.h>
#include <stdarg.h>

#include <immintrin.h>

namespace ASMcmp
{
    enum Errors
    {   
        FUNC_IS_OK                 = 0,
        BAD_ALLOC                  = 0xBADA110C,

        PTR_IS_NULL                = 0x800,
        NODE_IS_NULL               = 0x801,
        STREAM_IS_NULL             = 0x802,
        FILENAME_IS_NULL           = 0x803,
        DEST_PTR_ISNT_NULL         = 0x804,
        TREE_ROOT_ISNT_NULL        = 0x805,
        NODE_HAS_CHILDREN          = 0x806,
        INVALID_BRACKETS           = 0x807,
        NO_CHILD                   = 0x808,
        EMPTY_VALUE                = 0x809,
        // NODE_VALUE_ISNT_NULL       = 0x80A,
        // WRONG_VALUE_FORMAT         = 0x80B,
        TOKEN_IS_NULL              = 0x80C,
        UNDEFINED_KEYWORD          = 0x80D,
        WORD_MAX_LEN_REACHED       = 0x80E,
        WRONG_IDENTIFIER_FMT       = 0x80F,
        WRONG_NUM_FMT              = 0x810,
        TREE_FORMAT_ISNT_RIGHT     = 0x811,

        GLOBAL_STMTS_ARE_NOT_STMTS = 0x812,
        // STMT_HASNT_GOT_STMT        = 0x813,
        // REPEATING_VARIABLE         = 0x814,


// Nametable flags
        VARIABLE_FOUND             = 0x815,

        INVALID_GLOBAL_STMT        = 0x816,
        IS_NOT_A_NUMBER            = 0x817,
        IS_NOT_A_MATH_OPER         = 0x818,
        INVALID_STMT               = 0x819,
        IS_NOT_A_VARIABLE          = 0x81A,
        // CANT_FIND_NON_ID_IN_NT     = 0x81B,
        VARIABLE_NOT_FOUND         = 0x81C,
        UNDEFINED_OPERATOR         = 0x81D,
        REPEATING_MAIN             = 0x81E,
        INVALID_TYPE_FOR_MARK      = 0x81F,
        NAMETABLE_ISNT_CLEAR       = 0x820,
        VARIABLE_IS_ENGAGED        = 0x821,
        NO_MAIN_IN_PROGRAM         = 0x822,
        UNEXPECTED_MATH_OPER       = 0x823,
        // PRINT_DOESNT_HAVE_ARG      = 0x824,
        NOT_LOGICAL_OPER              = 0x825,
        // PARAM_ISNT_PARAM           = 0x826,
        // PARAM_ISNT_THE_ONLY        = 0x827,
        // NO_PARAMS                  = 0x828,
        RETURN_ISNT_RETURN         = 0x829,

        BACK_NULL                  = 0x82A,
        BUFF_OVERFLOW              = 0x82B,

        POSIX_MEMALIGN_FAILED      = 0x82C,
        MPROTECT_FAILED            = 0x82D,

        FILE_CANT_BE_OPENED        = 0x82E,

        MSG_IS_NULLPTR             = 0x82F,
        FMT_IS_NULLPTR             = 0x830,
        INVALID_EXPRESSION         = 0x831,


        INVALID_ASSIGN             = 0x832,
        DEAD_VALUE_REMOVED         = 0x833,

        INVALID_CALL_PARAMS        = 0x834,
        INVALID_JUMP_KEYW_ID       = 0x835,

        INVALID_LABEL_ID           = 0x836,
        ARG_OVERFLOW               = 0x837,

        CANT_FIND_LABEL            = 0x838,
        UNDEFINED_CALL_MOD         = 0x839,

    };

    const size_t INITIAL_CAPACITY = 16;
    const size_t STACK_BUFF_SIZE  = 64;
}

struct Backend
{
    List_t *NT;
    // List_t *GlobalNT; // not used for now

    FILE *asm_log;

    List_t *LabelsDst;
    List_t *LabelsSrc;

    Node_t *main;
    Node_t *last_return;

    uint8_t *code_buff;
    size_t   buff_ptr;
    size_t   buff_size;

    size_t stack_buff; // <= 120

    size_t if_counter;
    size_t while_counter;

    size_t number_of_params;
    size_t number_of_locals;
    size_t current_local;

};


int InitEmptyNode(Node_t **node_);
int ReadBuffer   (char **buffer, FILE *stream);
int FillNodes    (char **ptr, Node_t *node);
int FillTree     (const char *filename, Tree_t *tree);

int IsNum        (Node_t *nodes);
int IsVar        (Node_t *nodes);
int IsMathOper   (Node_t *nodes);
int IsLogOper    (Node_t *nodes);

int EmitASM      (const char *filename, Tree_t *tree);
int EmitElf      (Backend *back);
int EmitLog      (Backend *back, const char *msg);

enum LabelsId
{
    DST_ID = 1,
    SRC_ID = 2,
};

enum CallMods
{
    PRINT_MOD = 1,
    SCAN_MOD  = 2,
};

enum OpCodes
{
    RAX   = 0b000,
    RCX   = 0b001,
    RDX   = 0b010,
    RBX   = 0b011,
    RSP   = 0b100,
    RBP   = 0b101,
    RSI   = 0b110,
    RDI   = 0b011,

    RAX_SRC = 0b00000000,
    RCX_SRC = 0b00001000,
    RDX_SRC = 0b00010000,
    RBX_SRC = 0b00011000,
    RSP_SRC = 0b00100000,
    RBP_SRC = 0b00101000,
    RSI_SRC = 0b00110000,
    RDI_SRC = 0b00111000,

    RAX_DST = 0b00000000,
    RCX_DST = 0b00000001,
    RDX_DST = 0b00000010,
    RBX_DST = 0b00000011,
    RSP_DST = 0b00000100,
    RBP_DST = 0b00000101,
    RSI_DST = 0b00000110,
    RDI_DST = 0b00000111,

    RET   = 0xC3,
    REX   = 0x40,
    REX_W = REX | 0b1000,
    REX_B = REX | 0b0001,
    REX_R = REX | 0b0100,

    REX_WB = REX_W | REX_B,
    REX_WR = REX_W | REX_R,
    REX_RB = REX_R | REX_B,

    MOD_RR  = 0b11000000,

    CQO     = 0x99,

    PUSH    = 0x50,
    POP     = 0x58,

    CALL    = 0xE8,

    JCOND   = 0x0F,
    JAE     = 0x83,
    JA      = 0x87,
    JE      = 0x84,
    JNE     = 0x85,
    JB      = 0x82,
    JBE     = 0x86,

    JMP     = 0xE9,

    MOV_RI  = 0xB8,
    MOV_RM  = 0x89,
    MOV_MR  = 0x8B,

    RBP_MR  = 0x45,

    XOR_RM  = 0x33,

    CMP_RM  = 0x39,

    ADD_RM  = 0x01,
    SUB_RM  = 0x29,

    MUL_RM  = 0xF7,
    DIV_RM  = 0xF7,

    IDIV_AP = 0b00111000,
     DIV_AP = 0b00110000,

    IMUL_AP = 0b00101000,
     MUL_AP = 0b00100000,

    SUB_RI8 = 0x83,
    ADD_RI8 = 0x83,

};

#ifdef DEBUG_LIB_H
#define CATCH_ERR  \
do                  \
{                    \
 /* PRINT_(1234_);*/  \
    if (status)        \
    {                   \
        PRINT(ERR);      \
        PRINT_X(status);  \
        return status;     \
    }                       \
}                            \
while (0)
#else
#define CATCH_ERR               \
do                               \
{                                 \
    if (status) return status;     \
}                                   \
while (0)
#endif

#ifdef DEBUG_LIB_H
#define CATCH_NULL(PTR)                  \
do                                        \
{                                          \
    if (PTR == nullptr)                     \
    {                                        \
        PRINT_PTR(PTR);                       \
        return ASMcmp::PTR_IS_NULL;            \
    }                                           \
}                                                \
while (0)
#else
#define CATCH_NULL(PTR)                             \
do                                                   \
{                                                     \
    if (PTR == nullptr) return ASMcmp::PTR_IS_NULL;    \
}                                                       \
while (0)
#endif

#endif // ASM_COMP_H
