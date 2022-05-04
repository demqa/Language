#ifndef STACK_H

#define STACK_H

#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

#define STACK_INFO   01
#define HIPPO_GUARD  02
#define HASH_GUARD   04

#include "stack_config.h"

// TODO DEBUG_MODE = 010 WHEN I PRINT EVERY SINGLE DESTRUCTED
// WITH CHECKING OF ERRNO 5 AND CHECK WHEN I CALL
// NOT MY MEMORY. IT WILL BE VERY NICE

enum StatusCode{
    STACK_IS_OK                     = 0,
    RESULT_IS_UNKNOWN               = 0,

    STACK_IS_EMPTY                  = 1 << 29,
    STACK_IS_DESTRUCTED             = 1 << 30,
    DUMP_COMMITED                   = 1 << 31,
    
    STACK_IS_NULLPTR                = 1 << 0,
    STACK_IS_ALREADY_EMPTY          = 1 << 1,
    CANT_ALLOCATE_MEMORY            = 1 << 2,

    STACK_DATA_IS_RUINED            = 1 << 3,
    STACK_DATA_IS_NULLPTR           = 1 << 4,
    STACK_SIZE_BIGGER_THAN_CAPACITY = 1 << 5,
    STACK_CAPACITY_LESS_THAN_ZERO   = 1 << 6,
    STACK_SIZE_LESS_THAN_ZERO       = 1 << 7,

    STACK_RESIZE_WRONG_PARAM        = 1 << 14,
    
#if DEBUG_MODE & STACK_INFO
    STACK_INFO_IS_OK                = 1 << 15,
    STACK_IS_ALREADY_CONSTRUCTED    = 1 << 16,

    STACK_INFO_IS_EMPTY             = 1 << 21,
    STACK_INFO_RUINED               = 1 << 22,
#endif

#if DEBUG_MODE & HIPPO_GUARD
    STACK_LEFT_HIPPO_RUINED         = 1 << 23,
    STACK_RIGHT_POTAM_RUINED        = 1 << 24,

    DATA_LEFT_HIPPO_RUINED          = 1 << 25,
    DATA_RIGHT_POTAM_RUINED         = 1 << 26,
#endif

#if DEBUG_MODE & HASH_GUARD
    STACK_HASH_STACK_RUINED         = 1 << 27,
    STACK_HASH_DATA_RUINED          = 1 << 28,
#endif
    
};

enum ResizeMode{
    INCREASE_CAPACITY = 0xD0DE,
    DECREASE_CAPACITY = 0xF0E5,
};

const int STRING_MAX_SIZE = 100;

#if DEBUG_MODE & STACK_INFO
    struct StackInfo{
        int         line;
        const char *file;
        const char *func;
        const char *name;
    };
#endif

struct stack_t{

#if DEBUG_MODE & HIPPO_GUARD
    u_int64_t HIPPO;
#endif

    Elem_t *data;
    size_t size;
    size_t capacity;
    int status;

    void (* PrintElem)(void *memory, size_t n_bytes, FILE *stream);

#if DEBUG_MODE & HASH_GUARD
    u_int64_t hash_stack;
    u_int64_t hash_data;
#endif

#if DEBUG_MODE & STACK_INFO
    StackInfo info;
#endif

#if DEBUG_MODE & HIPPO_GUARD
    u_int64_t POTAM;
#endif

};

stack_t *StackCtor_(stack_t *stack, size_t capacity, int line_created, const char file[STRING_MAX_SIZE], const char func[STRING_MAX_SIZE],
                    const char stack_name[STRING_MAX_SIZE], void (* PrintElem)(void *, size_t, FILE *));
StatusCode StackDtor(stack_t *stack);
Elem_t StackPop(stack_t *stack);
Elem_t StackTop(stack_t *stack);
StatusCode StackPush(stack_t *stack, Elem_t value);

StatusCode StackIsDestructed(stack_t *stack);
StatusCode StackIsEmpty(stack_t *stack);
int StackVerify(stack_t *stack);

void PrintHex(void *memory, size_t size, FILE *stream);

StatusCode StackDump_(stack_t *stack, int line, const char file[STRING_MAX_SIZE], const char func[STRING_MAX_SIZE]);

#define STACK_STATUS(status_code){              \
    stack->status |= status_code;                \
    StackDump(stack);                             \
    return (StatusCode) stack->status;             \
}

// TODO DO WHILE(0)

#if DEBUG_MODE & HIPPO_GUARD
    const u_int64_t HIPPO = 0xAC1DC1DA;
    const u_int64_t POTAM = 0xD1CAAC1D;
#endif

#if DEBUG_MODE & STACK_INFO
    StatusCode StackInfoStatus(stack_t *stack);
#endif

#ifdef DEBUG_MODE
    #define ASSERT_OK(stack){                      \
        if (StackVerify(stack) != STACK_IS_OK){     \
            StackDump(stack);                        \
            assert(!"ok" && "Bad stack");             \
        }                                              \
    }
#endif

#if DEBUG_MODE & STACK_INFO
    #define StackCtor(stack, capacity, func){                                               \
        StackCtor_(stack, capacity, __LINE__, __FILE__, __PRETTY_FUNCTION__, #stack, func);  \
    }
#else
    #define StackCtor(stack, capacity, func){                                               \
        StackCtor_(stack, capacity, 0, "", "", "", func);                                    \
    }
#endif


#define StackDump(stack){                                             \
    StackDump_(stack, __LINE__, __FILE__, __PRETTY_FUNCTION__);        \
}

#define PRINT_ERROR(error){   \
    if (stack_status & error)  \
        printf("%s ", #error);  \
}

#if DEBUG_MODE & HASH_GUARD
    #define PRINT_WARNING(warn){                         \
        if (stack_status & warn){                         \
            printf("%s ", #warn);                          \
            stack->status &= ~warn;                         \
            stack->hash_stack = CalculateHashStack(stack);   \
        }                                                     \
    }
#else
    #define PRINT_WARNING(warn){                       \
        if (stack_status & warn){                       \
            printf("%s ", #warn);                        \
            stack->status &= ~warn;                       \
        }                                                  \
    }
#endif     

#define ADDRESS(ptr, type) (u_int64_t)(ptr - (type *)nullptr)


#endif