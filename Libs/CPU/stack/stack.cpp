#include "stack.h"

#if DEBUG_MODE & HASH_GUARD

    u_int64_t static Hash(void *memory, size_t number_of_bytes){
        u_int64_t sum = 0;

        for (size_t i = 0; i < number_of_bytes; ++i){
            sum += (*((char *)memory + i) + 12757342) * (383299 * number_of_bytes * i);
        }

        return sum;
    }

    u_int64_t static CalculateHashStack(stack_t *stack){
        if (stack == nullptr){
            return (u_int64_t)STACK_IS_NULLPTR;
        }

        u_int64_t hash_stack_old = stack->hash_stack;
        u_int64_t hash_data_old  = stack->hash_data;
        stack->hash_stack = 0UL;   stack->hash_data = 0UL;

        u_int64_t hash = Hash((void *)stack, sizeof(*stack));
        hash += ADDRESS(stack->data, Elem_t);

        stack->hash_stack = hash_stack_old;
        stack->hash_data  = hash_data_old;
        
        return hash;
    }

    u_int64_t static CalculateHashData(stack_t *stack){
        if (stack == nullptr){
            return (u_int64_t)STACK_IS_NULLPTR;
        }

        u_int64_t hash = 0UL;
        if (stack->data != nullptr && StackIsDestructed(stack) != STACK_IS_DESTRUCTED && stack->hash_stack == CalculateHashStack(stack)){
            hash += Hash((void *)stack->data, stack->capacity * sizeof(Elem_t));
        }
        else{
            stack->status |= STACK_DATA_IS_RUINED;
            StackDump(stack);
        }
            
        return hash;
    }

#endif

stack_t *StackCtor_(stack_t *stack, size_t capacity, int line_created, const char file[STRING_MAX_SIZE], const char func[STRING_MAX_SIZE],
                    const char stack_name[STRING_MAX_SIZE], void (* PrintElem)(void *, size_t, FILE *)){
    if (stack == nullptr){
        return nullptr;
    }

    if (StackIsEmpty(stack)       != STACK_IS_EMPTY &&
        StackIsDestructed(stack)  != STACK_IS_DESTRUCTED){
        return nullptr;
    }

#if DEBUG_MODE & STACK_INFO
    stack->info.file = file;
    stack->info.func = func;
    stack->info.line = line_created;
    stack->info.name = stack_name;
#endif

    if (PrintElem != nullptr) stack->PrintElem = PrintElem;

    if (capacity == 0){
        stack->capacity = 0;
        stack->size     = 0;
        stack->status   = STACK_IS_OK;
        stack->data     = nullptr;

    #if DEBUG_MODE & HIPPO_GUARD
        stack->HIPPO = HIPPO ^ ADDRESS(stack, stack_t);
        stack->POTAM = POTAM ^ ADDRESS(stack, stack_t);
    #endif

    #if DEBUG_MODE & HASH_GUARD
        stack->hash_stack = CalculateHashStack(stack);
        stack->hash_data  = 0;
    #endif

        return stack;
    }

    if (capacity & 07 != 0) capacity = (capacity / 8 + 1) * 8;

#if DEBUG_MODE & HIPPO_GUARD
    stack->HIPPO = HIPPO ^ ADDRESS(stack, stack_t);
    stack->POTAM = POTAM ^ ADDRESS(stack, stack_t);
    
    void *ptr = calloc(capacity * sizeof(Elem_t) + 2 * sizeof(u_int64_t), sizeof(char));

    if (ptr == nullptr){
        stack->status |= CANT_ALLOCATE_MEMORY;
        return stack;
    }
        
    *((u_int64_t *)ptr) = stack->HIPPO;
    ptr = ptr + sizeof(u_int64_t) + sizeof(Elem_t) * capacity;

    *((u_int64_t *)ptr) = stack->POTAM;
    ptr = ptr - sizeof(Elem_t) * capacity;
        
    stack->data = (Elem_t *) ptr;
#else
    stack->data = (Elem_t *) calloc(capacity, sizeof(Elem_t));
    if (stack->data == nullptr){
        stack->status |= CANT_ALLOCATE_MEMORY;
        return stack;
    }
#endif

    for (size_t i = 0; i < capacity; i++){
        stack->data[i] = POISONED_ELEM;
    }

    stack->size     = 0;
    stack->capacity = capacity;

    stack->status   = STACK_IS_OK;

#if DEBUG_MODE & HASH_GUARD
    stack->hash_stack = CalculateHashStack(stack);
    stack->hash_data  = CalculateHashData(stack);
#endif

    ASSERT_OK(stack);

    return stack;
}

Elem_t *StackResize(stack_t *stack, ResizeMode mode){
    
    ASSERT_OK(stack);

    size_t new_capacity = 0;

    if (mode == INCREASE_CAPACITY){
        new_capacity = stack->capacity * 2;
    }
    else 
    if (mode == DECREASE_CAPACITY){
        new_capacity = stack->capacity / 2;
    }
    else{
        stack->status |= STACK_RESIZE_WRONG_PARAM;
        StackDump(stack);
        return nullptr;
    }

    if (new_capacity < 8) new_capacity = 8;

    if (new_capacity == stack->capacity) return stack->data;

    if (mode == DECREASE_CAPACITY){
        for (size_t i = new_capacity; i < stack->capacity; ++i){
            stack->data[i] = POISONED_ELEM;
        }
    }

#if DEBUG_MODE & HIPPO_GUARD
    void *try_realloc = realloc((void *)stack->data - sizeof(u_int64_t), sizeof(Elem_t) * new_capacity + 2 * sizeof(u_int64_t));
#else
    void *try_realloc = realloc((void *)stack->data, new_capacity * sizeof(Elem_t));
#endif

    if (try_realloc == nullptr){
        stack->status |= CANT_ALLOCATE_MEMORY;
        StackDump(stack);
        return nullptr;
    }

#if DEBUG_MODE & HIPPO_GUARD
    *((u_int64_t *)try_realloc) = stack->HIPPO;
    try_realloc = try_realloc + sizeof(u_int64_t) + sizeof(Elem_t) * new_capacity;

    *((u_int64_t *)try_realloc) = stack->POTAM;
    try_realloc = try_realloc - sizeof(Elem_t) * new_capacity;
#endif

    stack->data = (Elem_t *) try_realloc;

    if (mode == INCREASE_CAPACITY){
        for (size_t i = stack->capacity; i < new_capacity; ++i){
            stack->data[i] = POISONED_ELEM;
        }
    }

    stack->capacity = new_capacity;

#if DEBUG_MODE & HASH_GUARD
    stack->hash_stack = CalculateHashStack(stack);
    stack->hash_data  = CalculateHashData(stack);
#endif

    ASSERT_OK(stack);

    return stack->data;
}

StatusCode StackPush(stack_t *stack, Elem_t value){
    if (stack == nullptr){
        StackDump(stack);
        return STACK_IS_NULLPTR;
    }

    ASSERT_OK(stack);

    if (stack->data == nullptr && stack->capacity == 0 ||
        stack->size == stack->capacity){

        Elem_t *try_resize = StackResize(stack, INCREASE_CAPACITY);
        if (try_resize == nullptr){
            stack->status |= CANT_ALLOCATE_MEMORY;
            StackDump(stack);
            return (StatusCode) stack->status;
        }

        stack->data = try_resize;
    }

    stack->data[stack->size++] = value;

#if DEBUG_MODE & HASH_GUARD
    stack->hash_stack = CalculateHashStack(stack);
    stack->hash_data  = CalculateHashData(stack);
#endif

    ASSERT_OK(stack);

    return STACK_IS_OK;
}

Elem_t StackPop(stack_t *stack){
    if (stack == nullptr){
        StackDump(stack);
        return POISONED_ELEM;
    }

    ASSERT_OK(stack);

    if (stack->size == 0){
        stack->status |= STACK_IS_ALREADY_EMPTY;

    #if DEBUG_MODE & HASH_GUARD
        stack->hash_stack = CalculateHashStack(stack);
    #endif

        return POISONED_ELEM;
    }
    
    Elem_t data_elem = stack->data[--stack->size];
    stack->data[stack->size] = POISONED_ELEM;

#if DEBUG_MODE & HASH_GUARD
    stack->hash_stack = CalculateHashStack(stack);
    stack->hash_data  = CalculateHashData(stack);
#endif

    if (stack->size <= stack->capacity / 4){
        Elem_t *try_resize = StackResize(stack, DECREASE_CAPACITY);
        if (try_resize == nullptr){
            stack->status |= CANT_ALLOCATE_MEMORY;
            StackDump(stack);
            return (StatusCode) stack->status;
        }

        stack->data = try_resize;
    }

#if DEBUG_MODE & HASH_GUARD
    stack->hash_stack = CalculateHashStack(stack);
    stack->hash_data  = CalculateHashData(stack);
#endif

    ASSERT_OK(stack);

    return data_elem;
}

Elem_t StackTop(stack_t *stack){
    if (stack == nullptr){
        StackDump(stack);
        return POISONED_ELEM;
    }

    ASSERT_OK(stack);

    if (stack->size == 0){
        stack->status |= STACK_IS_ALREADY_EMPTY;
        StackDump(stack);
        return POISONED_ELEM;
    }
    
    Elem_t data_elem = stack->data[stack->size - 1];

    return data_elem;
}

StatusCode StackDtor(stack_t *stack){
    if (stack == nullptr){
        StackDump(stack);
        return STACK_IS_NULLPTR;
    }

    if (StackIsDestructed(stack) == STACK_IS_DESTRUCTED){
        // StackDump(stack);
        return STACK_IS_DESTRUCTED;
    }

#if DEBUG_MODE & HIPPO_GUARD
    stack->HIPPO = 0xDEADF1DC ^ ADDRESS(stack, stack_t);
    stack->POTAM = 0xAC47AC47 ^ ADDRESS(stack, stack_t);
#endif

#if DEBUG_MODE & STACK_INFO
    stack->info.file = (char *)(300);
    stack->info.func = (char *)(228 - 127);
    stack->info.name = (char *)(999 - 123);
    stack->info.line = 0xEBAAL;
#endif

#if DEBUG_MODE & HASH_GUARD
    stack->hash_stack = 0xFA1EBACAUL;
    stack->hash_data  = 0xAFE1ABACUL;
#endif

    stack->PrintElem = nullptr;

    if (stack->data == nullptr){
        if (stack->capacity == 0){
            stack->capacity = 0xBEBA;
            stack->size     = 0xDEDA;
            
            stack->status = STACK_IS_DESTRUCTED;
            return (StatusCode) stack->status;
        }
        STACK_STATUS(STACK_DATA_IS_NULLPTR);
    }

    while (stack->size < size_t(-1)){
        stack->data[stack->size--] = POISONED_ELEM;
    }
    stack->size = 0;

    #if DEBUG_MODE & HIPPO_GUARD
        free((void *)stack->data - sizeof(u_int64_t));
    #else
        free(stack->data);
    #endif
    
    stack->data = (Elem_t *)(1000 - 7);

    stack->capacity = 0xD1ED;
    stack->size     = 0xF1FA;

    stack->status = STACK_IS_DESTRUCTED;

    return (StatusCode) stack->status;
}

int StackVerify(stack_t *stack){
    if (stack == nullptr){
        return STACK_IS_NULLPTR;
    }

    if (StackIsEmpty(stack)      == STACK_IS_EMPTY)
        return STACK_IS_EMPTY;
    if (StackIsDestructed(stack) == STACK_IS_DESTRUCTED)
        return STACK_IS_DESTRUCTED;
    
    int status = STACK_IS_OK;

#if DEBUG_MODE & HASH_GUARD
    if (stack->hash_stack != CalculateHashStack(stack)){
        status |= STACK_DATA_IS_RUINED | STACK_HASH_STACK_RUINED;
    }

    if ((status & STACK_DATA_IS_RUINED) == 0 && stack->hash_data != CalculateHashData(stack)){
        status |= STACK_DATA_IS_RUINED | STACK_HASH_DATA_RUINED;
    }
#endif

#if DEBUG_MODE & STACK_INFO
    if (StackInfoStatus(stack) != STACK_INFO_IS_OK){
        status |= STACK_DATA_IS_RUINED | STACK_INFO_RUINED;
    }
#endif
    
    if (stack->size > stack->capacity)
        status |= STACK_DATA_IS_RUINED | STACK_SIZE_BIGGER_THAN_CAPACITY;
    if (stack->capacity < 0)
        status |= STACK_DATA_IS_RUINED | STACK_CAPACITY_LESS_THAN_ZERO;
    if (stack->size < 0)
        status |= STACK_DATA_IS_RUINED | STACK_SIZE_LESS_THAN_ZERO;
    if (stack->data == nullptr && stack->capacity != 0)
        status |= STACK_DATA_IS_RUINED | STACK_DATA_IS_NULLPTR;

#if DEBUG_MODE & HIPPO_GUARD
    if (stack->HIPPO != (u_int64_t) (HIPPO ^ ADDRESS(stack, stack_t)))
        status |= STACK_DATA_IS_RUINED | STACK_LEFT_HIPPO_RUINED;
    if (stack->POTAM != (u_int64_t) (POTAM ^ ADDRESS(stack, stack_t)))
        status |= STACK_DATA_IS_RUINED | STACK_RIGHT_POTAM_RUINED;
            
    if (stack->data != nullptr && status & STACK_DATA_IS_RUINED == 0 && status & STACK_IS_DESTRUCTED == 0){
        void *ptr = (void *) stack->data - sizeof(u_int64_t);
            
        if (*((u_int64_t *)ptr) != HIPPO ^ ADDRESS(stack, stack_t))
            status |= STACK_DATA_IS_RUINED | DATA_LEFT_HIPPO_RUINED;

        ptr = ptr + sizeof(u_int64_t) + sizeof(Elem_t) * stack->capacity;

        if (*((u_int64_t *)ptr) != POTAM ^ ADDRESS(stack, stack_t))
            status |= STACK_DATA_IS_RUINED | DATA_RIGHT_POTAM_RUINED;

        ptr = ptr - sizeof(Elem_t) * stack->capacity;

        stack->data = (Elem_t *) ptr;
    }
#endif

    stack->status = status;

#if DEBUG_MODE & HASH_GUARD
    stack->hash_stack = CalculateHashStack(stack);
#endif

    return status;
}

StatusCode StackIsEmpty(stack_t *stack){
    if (stack == nullptr)
        return STACK_IS_NULLPTR;

    if (stack->data != nullptr || stack->size   != 0 ||
        stack->capacity != 0   || stack->status != STACK_IS_OK){
        return RESULT_IS_UNKNOWN;
    }

#if DEBUG_MODE & STACK_INFO
    if (StackInfoStatus(stack) != STACK_INFO_IS_EMPTY){
        return RESULT_IS_UNKNOWN;
    }
#endif

#if DEBUG_MODE & HIPPO_GUARD
    if (stack->HIPPO != 0 || stack->POTAM != 0){
        return RESULT_IS_UNKNOWN;
    }
#endif

#if DEBUG_MODE & HASH_GUARD
    if (stack->hash_stack != 0 || stack->hash_data != 0){
        return RESULT_IS_UNKNOWN;
    }
#endif

    return STACK_IS_EMPTY;
}

#if DEBUG_MODE & STACK_INFO
    StatusCode StackInfoStatus(stack_t *stack){
        if (stack == nullptr)
            return STACK_IS_NULLPTR;

        if (stack->info.line == 0       && stack->info.file == nullptr &&
            stack->info.func == nullptr && stack->info.name == nullptr)
            return STACK_INFO_IS_EMPTY;
        else
        if (stack->info.line == 0       || stack->info.file == nullptr ||
            stack->info.func == nullptr || stack->info.name == nullptr)
            return STACK_INFO_RUINED;
        else{
            return STACK_INFO_IS_OK;
        }
    }
#endif

StatusCode StackIsDestructed(stack_t *stack){
    if (((stack->data == (Elem_t *)(1000 - 7) && stack->capacity == 0xD1ED && stack->size == 0xF1FA) ||
         (stack->data == nullptr              && stack->capacity == 0xBEBA && stack->size == 0xDEDA))
                
        && stack->PrintElem == nullptr

    #if DEBUG_MODE & STACK_INFO
        &&
          stack->info.file == (char *)(300)       && stack->info.func == (char *)(228 - 127) &&
          stack->info.name == (char *)(999 - 123) && stack->info.line == 0xEBAAL
        #endif

    #if DEBUG_MODE & HIPPO_GUARD
        &&
          stack->HIPPO == 0xDEADF1DC ^ ADDRESS(stack, stack_t) && 
          stack->POTAM == 0xAC47AC47 ^ ADDRESS(stack, stack_t)
    #endif

    #if DEBUG_MODE & HASH_GUARD
        &&
          stack->hash_stack == 0xFA1EBACAUL &&
          stack->hash_data  == 0xAFE1ABACUL
    #endif

        ){
        return STACK_IS_DESTRUCTED;
    }

    return RESULT_IS_UNKNOWN;
}

// TODO printf to fprintf
void PrintHex(void *memory, size_t size, FILE *stream){
    char *mem = (char *) memory;
    if (mem != nullptr && mem != (char *)(1000 - 7)){
        for (char *c = mem; c <= mem + size; ++c){
            fprintf(stream, "%02x ", *((u_int8_t *)c));
        }
    }
    fprintf(stream, "\n");
}

int NumberOfCharacters(int edge){
    int num = 1;
    for (int p = 1; p * 10 <= edge; p *= 10, ++num);
    return num;
}

StatusCode StackDump_(stack_t *stack, int line, const char file[STRING_MAX_SIZE], const char func[STRING_MAX_SIZE]){
    static int r = 0;
    if (r++ == 0){
        printf(".....$*$*....................\n"
               "...$*....$*............$*....\n"
               "..$*.......$*.......$*....$*.\n"
               ".$*.........$*....$*.......$*\n"
               ".$*...........$*.$*........$*\n"
               ".$*.............*.........$*.\n"
               "..$*....................$*...\n"
               "...$*.................$*.....\n"
               "... $*..............$*.......\n"
               ".....$*...........$*.........\n"
               "......$*........$*...........\n"
               "........$*....$*.............\n"
               ".........$*.$*...............\n"
               "..........*$*....<3..<3......\n"
               "..........$.....<3.<3.<3.....\n"
               ".........*......<3....<3.....\n"
               "........$........<3..<3......\n"
               ".......*...........<3........\n\n");
    }

    if (stack == nullptr){
        printf("STACK_IS_NULLPTR\n");
        return DUMP_COMMITED;
    }

    printf("stack<Elem_t>[%p] ", stack);

    int stack_status       = 0;
    int stack_has_errors   = 0;
    int stack_has_warnings = 0;

    if (StackIsEmpty(stack)      == STACK_IS_EMPTY){
        printf("ok, empty(not constructed),");
    }
    else
    if (StackIsDestructed(stack) == STACK_IS_DESTRUCTED){
        printf("ok, destructed,");
    }
    else
    if (StackVerify(stack) == STACK_IS_OK){
        printf("ok,");
        if (stack->status  != STACK_IS_OK){
            printf(" WARNING,");
            stack_has_warnings = (int)'YES';
        }
    }
    else{
        printf("ERROR,");
        stack_has_errors = (int)'YES';
    }

    printf(" called from %s at %s \b(%d)", func, file, line);

#if DEBUG_MODE & STACK_INFO
    if (StackIsDestructed(stack) != STACK_IS_DESTRUCTED && StackInfoStatus(stack) == STACK_INFO_IS_OK)
        printf(", \"%s\" created at %s at %s \b(%d)", stack->info.name, stack->info.func, stack->info.file, stack->info.line);
#endif

    stack_status |= stack->status;

    if (stack_has_warnings){
        printf("\n");
        PRINT_WARNING(STACK_IS_ALREADY_EMPTY);
        PRINT_WARNING(STACK_IS_ALREADY_CONSTRUCTED);
        PRINT_WARNING(CANT_ALLOCATE_MEMORY);
    }
    else
    if (stack_has_errors){
        printf("\n");
        PRINT_ERROR(STACK_DATA_IS_RUINED);

        PRINT_ERROR(STACK_DATA_IS_NULLPTR);
        PRINT_ERROR(STACK_SIZE_BIGGER_THAN_CAPACITY);
        PRINT_ERROR(STACK_CAPACITY_LESS_THAN_ZERO);
        PRINT_ERROR(STACK_SIZE_LESS_THAN_ZERO);
        PRINT_ERROR(STACK_RESIZE_WRONG_PARAM);

    #if DEBUG_MODE & STACK_INFO
        PRINT_ERROR(STACK_INFO_RUINED);
    #endif

    #if DEBUG_MODE & HIPPO_GUARD
        PRINT_ERROR(STACK_LEFT_HIPPO_RUINED);
        PRINT_ERROR(STACK_RIGHT_POTAM_RUINED);

        PRINT_ERROR(DATA_LEFT_HIPPO_RUINED);
        PRINT_ERROR(DATA_RIGHT_POTAM_RUINED);
    #endif

    #if DEBUG_MODE & HASH_GUARD
        PRINT_ERROR(STACK_HASH_STACK_RUINED);
        PRINT_ERROR(STACK_HASH_DATA_RUINED);
    #endif
    }

    printf("\n");

    printf("    {\n");
    printf("    size = %lu\n",     stack->size);
    printf("    capacity = %lu\n", stack->capacity);
#if DEBUG_MODE & HASH_GUARD
    printf("    hash_stack = "); PrintHex((void *)&stack->hash_stack, sizeof(u_int64_t), stdout);
    printf("    hash_data  = "); PrintHex((void *)&stack->hash_data,  sizeof(u_int64_t), stdout);
#endif
    printf("    data[%p]\n",           stack->data);

    if (!stack_has_errors && StackIsDestructed(stack) != STACK_IS_DESTRUCTED){
        printf("        {\n");
        int number_of_characters = NumberOfCharacters((int)stack->capacity - 1);
        for (size_t i = 0; i < stack->capacity; ++i){
            printf("         data[%*lu] = ", number_of_characters, i);
            if (stack->PrintElem != nullptr)
                stack->PrintElem((void *)(&stack->data[i]), sizeof(Elem_t), stdout);
            else
                PrintHex((void *)(&stack->data[i]), sizeof(Elem_t), stdout);
        }
        printf("        }\n");
    }

    printf("    }\n");

    printf("_________________DDDuMpEDDD_________________\n\n");

    return DUMP_COMMITED;
}
