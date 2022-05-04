#include "string_lib/string_lib.h"
#include "cpu_config.h"
#include <math.h>

#define COMPILE_ERROR(msg)                                      \
{                                                                \
    printf("[%s:%d] %s\n", __PRETTY_FUNCTION__, __LINE__, #msg);  \
    DestructText(asm_code);                                        \
    return -1;                                                      \
}

#define ADD_CMD_FLAGS(flags)                      \
{                                                  \
    *(Command *)(*ip - sizeof(Command)) |= (flags); \
}
 
#define ASSIGN_CMD_ARG(arg, type, ptr)                 \
{                                                       \
    *(type *)(ptr ip) = (arg);                           \
    (ptr ip) += sizeof(type);                             \
}

const size_t DEAD_INDEX = (size_t)(-1);

struct Label
{
    size_t index;
    char name[20];
};

struct Labels
{
    size_t size;
    size_t capacity;
    Label *labels;
};

void PrintHex(void *ptr, size_t size, FILE *stream)
{
    if (ptr == nullptr || stream == nullptr)
    {
        perror("PrintHex ptr is null or stream is null");
        return;
    }

    char *mem = (char *) ptr;
    for (char *c = mem + size - 1; c >= mem; c--)
        fprintf(stream, "%02x ", *((u_int8_t *)c));
}

RegNum RegNumber(char c)
{
    switch (c)
    {
        case 'a':
            return 1;
            break;
        case 'b':
            return 2;
            break;
        case 'c':
            return 3;
            break;
        case 'd':
            return 4;
            break;
        
        default:
            perror("You shouldn't be there");
            break;
    }
}

size_t FindLabel(Labels *labels, char *label)
{
    if (labels == nullptr || labels->labels == nullptr || label == nullptr)
        return DEAD_INDEX;

    for (size_t i = 0; i < labels_capacity; ++i)
    {
        if (strcmp(label, labels->labels[i].name) == 0)
        {
            return i;
        }
    }
    return labels_capacity;
}

int LabelIsInitialisedAgain(Labels *labels, char *label, size_t index)
{
    if (labels == nullptr || label == nullptr || labels->labels == nullptr)
        return -1;

    for (size_t j = 0; j < labels->size; ++j)
    {
        if (strcmp(labels->labels[j].name, label) == 0 &&
            labels->labels[j].index != index           &&
            labels->labels[j].index != DEAD_INDEX)
        {
            return -1;
        }
    }
}

int GetArg(char *asm_line, char **ip)
{
    if (ip == nullptr || asm_line == nullptr)
        return -1;

    char reg_letter[20] = {};          
    int  first          = 0;            
    int  last           = 0;             
    int  count          = 0;              
    int  check          = 0;               
    Elem_t x            = NAN;             
    size_t index        = 0;
    char command[20]    = {};
                                            
    if (sscanf(asm_line, "%s %lf", command, &x) == 2)
    {                                                            
        ADD_CMD_FLAGS(IMM);                      
                                                    
        ASSIGN_CMD_ARG(x, Elem_t, *);                 
    }                                               
    else                                                                                       
    if (sscanf(asm_line, "%s %n%1[abcd]x%n", command, &first, reg_letter, &last) == 2  
        && last - first == 2)                                                                    
    {                                                   
        ADD_CMD_FLAGS(REG);                              
                                                            
        ASSIGN_CMD_ARG(RegNumber(*reg_letter), RegNum, *);    
    }                                                       
    else                                                                      
    if (sscanf(asm_line, "%s [%lu]%n", command, &index, &check) == 2  
        && check != 0)                                                          
    {                                                           
        ADD_CMD_FLAGS(OSU);                                      
                                                                    
        ASSIGN_CMD_ARG(index, size_t, *);                             
    }                                                               
    else                                                                                         
    if (sscanf(asm_line, "%s %n[%1[abcd]x]%n", command, &first, reg_letter, &last) == 2
        && last - first == 4)
    {
        ADD_CMD_FLAGS(REG | OSU);
        
        ASSIGN_CMD_ARG(RegNumber(*reg_letter), RegNum, *);
    }
    else
    if (sscanf(asm_line, "%s %n[%1[abcd]x+%n%lu]%n", command, &first, reg_letter, &last, &index, &check) == 3  
        && last - first == 4 && check != 0)
    {
        ADD_CMD_FLAGS(IMM | OSU | REG);
        
        ASSIGN_CMD_ARG(RegNumber(*reg_letter), RegNum, *);
        ASSIGN_CMD_ARG(index, size_t, *);
    }
    else                                                                              
    {
        return -1;
    }

    return 0;
}

int ConstructLabels(Labels *labels, size_t capacity)
{
    if (labels == nullptr)
        return -1;

    labels->labels = (Label *) calloc(capacity, sizeof(Label));
    if (labels->labels == nullptr)
        return -1;

    for (size_t j_ = 0; j_ < capacity; ++j_)
        labels->labels[j_].index = DEAD_INDEX;
    
    return 0;
}

void WriteLabelName(Labels *labels, char *label)
{
    if (labels == nullptr || label == nullptr || labels->labels == nullptr)
        return;

    memcpy(labels->labels[labels->size++].name, label, 20);

    return;
}

int LabelsPush(Labels *labels, Label *label)
{
    if (labels == nullptr || label == nullptr || labels->labels == nullptr) return -1;

    if (labels->size > labels->capacity) return -2;

    if (label->name == nullptr) return -3;

    WriteLabelName(labels, label->name);

    return 0;
}

void DestructLabels(Labels *labels)
{
    if (labels == nullptr)
        return;
    
    labels->size     = 0;
    labels->capacity = 0;

    free(labels->labels);
    labels->labels = nullptr;
}

int LabelsVerify(Labels *labels)
{
    if (labels         == nullptr) return -1;
    if (labels->labels == nullptr) return -1;

    if (labels->size > labels->capacity) return -2;

    for (size_t i = 0; i < labels->size; i++)
    {
        if (labels->labels[i].index == DEAD_INDEX)
            return -3;
    }

    return 0;
}

int LabelsAreFull(Labels *labels)
{
    if (labels == nullptr)
        return -1;
    
    if (labels->size == labels->capacity)
        return -2;
    
    return 0;
}

int GetLabel(char *asm_line, char **ip, Labels *labels)
{
    if (asm_line == nullptr || ip == nullptr || *ip == nullptr || labels == nullptr)
        return -1;

    char   command[20]  = {};
    char   label  [20]  = {};
    int    check        = 0;
    size_t index_n      = 0;

    char *ptr = *ip;

    if (sscanf(asm_line, "%s %[A-Za-z0-9_]:%n", command, label, &check) == 2
        && check != 0)
    {
        size_t label_number = FindLabel(labels, label);
        
        if (label_number == labels_capacity)
        {
            // fprintf(stderr, "label %s not found\n", label);
            if (LabelsAreFull(labels)) return -2;

            WriteLabelName(labels, label);
            *ip += sizeof(size_t);
        }                                                                    
        else                            
        {
            // fprintf(stderr, "label %s found\n", label);
            ASSIGN_CMD_ARG(labels->labels[label_number].index, size_t, *);
        }       
    }          
    else      
    if (sscanf(asm_line, "%s %lu%n", command, &index_n, &check) == 2
        && check != 0)
    {                                                                                       
        ASSIGN_CMD_ARG(index_n, size_t, *);                                                     
    }
    else
    {
        return -1;
    }

    // fprintf(stderr, "ptr - *ip = %d\n", ptr - *ip);

    return 0;
}

int Assemble(char **header_ptr, Text *asm_code, size_t *buffsize)
{
    if (header_ptr == nullptr || *header_ptr == nullptr || 
        asm_code   == nullptr ||  buffsize   == nullptr)
        return -1;
    
    FILE *listing = fopen("listing", "w");
    if (listing == nullptr)
    {
        printf("OOPS, FATAL ERROR(CANT OPEN LISTING)\n");
        return -1;
    }

    size_t label_n = 0;

    Labels labels  = {};
    if (ConstructLabels(&labels, labels_capacity))
    {
        printf("OOPS, FATAL ERROR(CANT ALLOC LABELS)");
        return -1;
    }

    char *binary = *header_ptr;
    binary += sizeof(Header);

    char *ip = nullptr;
    int error_code = 0;

    for (int i_ = 0; i_ < 3; ++i_) // rewrite code
    {        // it should ^ be 2   so this is bullshit, labels dont work
        ip = binary;
        label_n = 0;

        for (int i = 0; i < asm_code->nlines; ++i)
        {
            char command[20] = {};

            int count = sscanf(asm_code->lines[i].ptr, "%s", command);

            if (count == 0)
            {
                COMPILE_ERROR(REWRITE_ASSEMBLER_CANT_READ_CMD);
            }

            int check = 0;

            char label[20] = {};
            count = sscanf(asm_code->lines[i].ptr, "%[A-Za-z0-9_]:%n", label, &check);

            if (count == 1 && check != 0)
            {
                if (label_n >= labels_capacity)
                {
                    COMPILE_ERROR(TOO_MUCH_LABELS);
                }

                size_t index = ip - binary;
                if (LabelIsInitialisedAgain(&labels, label, index))
                {
                    COMPILE_ERROR(REPEATING_LABEL);
                }

                Label lbl = {};

                lbl.index = index;
                // fprintf(stderr, "index = %x\n", index);

                memcpy(lbl.name, label, 20);

                size_t label_number = FindLabel(&labels, label);
                
                if (label_number == labels_capacity)
                {
                    labels.labels[label_n++]    = lbl;
                    // fprintf(stderr, "case 1\n");
                }
                else
                {
                    labels.labels[label_number] = lbl;
                    // fprintf(stderr, "case 2\n");
                }
                // labels.size++;

                continue;
            }

            #define DEF_JMP(jmp_name, jmp_num, jmp_sign)                \
                if (strcmp(command, #jmp_name) == 0)                     \
                {                                                         \
                    ASSIGN_CMD_ARG(jmp_num, Command, );                    \
                                                                                        \
                    if (GetLabel(asm_code->lines[i].ptr, &ip, &labels) == -1)            \
                        COMPILE_ERROR(CANT_READ_LABEL)                                    \
                }                                                              \
                else
            
            #define DEF_CMD(cmd_name, cmd_num, cmd_n_args, cmd_code)              \
                if (strcmp(command, #cmd_name) == 0)                               \
                {                                                                   \
                    ASSIGN_CMD_ARG(cmd_num, Command, );                              \
                                                                                      \
                    if (cmd_n_args == 1 && GetArg(asm_code->lines[i].ptr, &ip) == -1)  \
                        COMPILE_ERROR(UNRECOGNISABLE_CMD_ARG_FORMAT_ASM)                \
                }                                                                        \
                else

            #include "commands"
            /* else */
            {
                COMPILE_ERROR(ASM_UNKNOWN_COMMAND);
            }

            #undef DEF_JMP
            #undef DEF_CMD

            #undef ADD_CMD_FLAGS
            #undef ASSIGN_CMD_ARG


            if (strcmp(command, "HLT") == 0)
            {
                error_code = -1;
            }
        }
    }

    if (error_code != -1)
    {
        COMPILE_ERROR(REWRITE_ASM_THERE_IS_NO_HLT);
    }

    if (LabelsVerify(&labels))
        COMPILE_ERROR(ASM_LABEL_DOESNT_HAVE_INDEX);

    DestructLabels(&labels);

    *buffsize          = ip - binary;

    Header header      = {};

    header.signature   =  signature;
    header.version     =  version;
    header.buffsize    = *buffsize;

    *((Header *)(*header_ptr)) = header;

    fclose(listing);

    return 0;
}

int main(int argc, char *argv[])
{
    Text asm_code = {};
    int error_code = 0;

    if (argc == 3)
    {
        error_code = ConstructText(argv[1], &asm_code);
        CheckError(&error_code);
    }
    else
    {
        printf("give me nice arguments, pls\n");
        return -1;
    }

    #define max(a, b) ((a) > (b) ? (a) : (b))
    size_t max_elem_size_candidate_1 = sizeof(Command) + sizeof(Elem_t);
    size_t max_elem_size_candidate_2 = sizeof(Command) + sizeof(RegNum) + sizeof(u_int64_t);
    size_t max_elem_size = max(max_elem_size_candidate_1, max_elem_size_candidate_2);
    #undef max

    char *header_ptr = (char *) calloc(asm_code.nlines * max_elem_size + sizeof(Header) + 1, sizeof(char));
    if (header_ptr == nullptr)
    {
        printf("bad alloc... not enough memory... i dont want to describe this error... pls... aaahhh...\n");
        return -1;
    }

    size_t buffsize = 0;

    int error = Assemble(&header_ptr, &asm_code, &buffsize);
    if (error == -1)
    {
        printf("ASSEMBLING ERROR\n");
        return -1;
    }

    FILE *stream  = fopen(argv[2],  "wb");
    if (stream == nullptr)
    {
        printf("OOPS, FATAL ERROR(CANT OPEN BINARY)\n");
        return -1;
    }

    size_t nbytes = fwrite(header_ptr, sizeof(char), sizeof(Header) + buffsize, stream);
    if (nbytes != sizeof(Header) + buffsize)
    {
        printf("CANT WRITE BINARY TO FILE\n");
        return -1;
    }

    free(header_ptr);
    fclose(stream);
    
    return 0;
}
