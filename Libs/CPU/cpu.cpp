#include <string.h>
#include <time.h>

#include "cpu_config.h"
#include "stack/stack.h"
#include "stack/print_func.cpp"

#include "debug_lib.h"

#define PROCESSING_ERROR(msg)                                                                                           \
{                                                                                                                        \
    printf("cur_cmd #%lu %lf [%s:%d] message = %s\n", cpu->cur_cmd, *(Elem_t *)ip, __PRETTY_FUNCTION__, __LINE__, #msg);  \
    DestructCPU(cpu);                                                                                                      \
    return -1;                                                                                                              \
}

void sleep(int milliseconds)
{
    clock_t time_end = clock() + milliseconds * CLOCKS_PER_SEC / 1000;
    while (clock() < time_end) ;
}

struct CPU
{
    stack_t stack;
    // stack_t stack_adr;

    Header file_info;

    char *code;
    size_t code_size;

    size_t cur_cmd;

    Elem_t regs[5];
    Elem_t *RAM;
};

int DestructCPU(CPU *cpu)
{
    StackDtor(&cpu->stack);
    // StackDtor(&cpu->stack_adr);
    free(cpu->code); cpu->code = nullptr;
    free(cpu->RAM);  cpu->RAM  = nullptr;

    return 0;
}

int ConstructCPU(CPU *cpu, const char *binary_name)
{
    FILE *stream = fopen(binary_name, "rb");
    if (stream == nullptr)
    {
        printf("FATAL ERROR, CANT OPEN FILE\n");
        return 0;
    }

    char *header = (char *) calloc(sizeof(Header), sizeof(char));
    if (header == nullptr)
    {
        printf("bad alloc.. not enough memory i think...\n");
        return -1;
    }

    size_t n_bytes = fread(header, sizeof(char), sizeof(Header), stream);
    if (ferror(stream))
    {
        perror("THERE IS SOME ERROR IN FILE READING");
    }
    if (feof(stream))
    {
        printf("EOF REACHED\n");
    }
    if (n_bytes != sizeof(Header))
    {
        printf("THIS BINARY HAS AN ERROR. TOO SHORT TO BE MY FILE\n");
        fclose(stream);
        free(header);
        return -1;
    }

    if (*((u_int32_t *)header) == signature)
    {
        cpu->file_info = *(Header *)header;
    }
    else
    {
        printf("THIS BINARY ISNT MINE\n");
        fclose(stream);
        free(header);
        return -1;
    }

    free(header);

    if (cpu->file_info.version != version)
    {
        printf("THIS BINARY IS OLDER THAN PROGRAM, RECOMPILE ASM\n");
        fclose(stream);
        return -1;
    }

    cpu->code_size = cpu->file_info.buffsize;
    cpu->code = (char *) calloc(cpu->file_info.buffsize + 1, sizeof(char));
    if (cpu->code == nullptr)
    {
        printf("the 3rd way to achieve bad alloc. give me memory pls\n");
        return -1;
    }

    n_bytes = fread(cpu->code, sizeof(char), cpu->file_info.buffsize, stream);
    if (n_bytes != cpu->file_info.buffsize)
    {
        printf("WRONG BUFFSIZE or smth, i dont really know how to call this error (file hasnt reached eof)\n");
        return -1;
    }

    fclose(stream);

    StackCtor(&cpu->stack,     0, PrintDouble);
    // StackCtor(&cpu->stack_adr, 0, nullptr);

    cpu->RAM = (Elem_t *) calloc(RamSize, sizeof(Elem_t));
    if (cpu->RAM == nullptr)
    {
        DestructCPU(cpu);
        printf("NO RAM IN PROGRAM\n");
        return -1;
    }

    return 0;
}

#define ASSIGN_AND_GO_NEXT(name, type)   \
    type name = *(type *)ip;              \
    ip += sizeof(type);

int ProcessingCPU(CPU *cpu)
{
    char *ip = nullptr;

    for (ip = cpu->code; ip < cpu->code + cpu->file_info.buffsize; ++cpu->cur_cmd)
    {
        ASSIGN_AND_GO_NEXT(command, Command);

        switch (command & ~(IMM | REG | OSU))
        {

        #define DEF_CMD(cmd_name, cmd_num, cmd_n_args, cmd_code) \
        {                                                         \
            case CMD_ ## cmd_name:                                 \
                cmd_code                                            \
                break;                                               \
        }

        #define DEF_JMP(jmp_name, jmp_num, jmp_sign)  \
        {                                              \
            case JMP_ ## jmp_name:                      \
                if (jmp_num == JMP_CALL)                              \
                    PUSH((size_t)(ip - cpu->code) + sizeof(size_t));   \
                if (jmp_num == JMP_JMP ||                               \
                    jmp_num == JMP_CALL)                    \
                {                                            \
                    ASSIGN_AND_GO_NEXT(index, size_t);        \
                    ip = cpu->code + index;                    \
                    continue;                                   \
                }                                                \
                                                                  \
                Elem_t x = POP;                                    \
                Elem_t y = POP;                                     \
                if (y jmp_sign x)                                    \
                {                                                     \
                    ASSIGN_AND_GO_NEXT(index, size_t);                 \
                                                                        \
                    ip = cpu->code + index;                              \
                }                                                         \
                else                                                       \
                {                                                           \
                    ip += sizeof(size_t);                                    \
                }                                                             \
                                                                               \
                break;                                                          \
        }

            #include "commands"

            default:
                PROCESSING_ERROR(UNKNOWN_CMD);
        }
    }

    DestructCPU(cpu);
    printf("WRONG_ASSEMBLER_CODE, NO HLT\n");
    
    return -1;
}

#undef ASSIGN_AND_GO_NEXT

int main(int argc, char *argv[])
{
    CPU cpu = {};

    if (argc == 2)
    {
        if (ConstructCPU(&cpu, argv[1]) == -1)
            return 0;
    }
    else
    {
        printf("give me nice arguments, pls\n");
        return 0;
    }

    ProcessingCPU(&cpu);

    DestructCPU(&cpu);

    return 0;
}
