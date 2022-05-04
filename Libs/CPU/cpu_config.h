#ifndef CPU_CONFIG_H

#define CPU_CONFIG_H

#include "debug_lib.h"

#include <math.h>

u_int32_t signature = 'DSS';
u_int32_t version   = 7;

typedef double    Elem_t;
typedef u_int64_t Command;
typedef u_int64_t RegNum;

const size_t labels_capacity = 10;

const u_int8_t IMM = 0x20;
const u_int8_t REG = 0x40;
const u_int8_t OSU = 0x80;

const size_t RamSize = 0x4040UL;

struct Header
{
    u_int32_t signature;
    u_int32_t version;
    size_t    buffsize;
};

enum Commands
{
    #define DEF_CMD(cmd_name, cmd_num, cmd_n_args, cmd_code) \
        CMD_ ## cmd_name = (cmd_num),

    #define DEF_JMP(jmp_name, jmp_num, jmp_sign)                 \
        JMP_ ## jmp_name = (jmp_num),

    #include "commands"

    #undef DEF_CMD
    #undef DEF_JMP

};

#endif
