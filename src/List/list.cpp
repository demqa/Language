#include "list.h"

StatusCode ListCtor(List_t *list, size_t capacity)
{
    if (list == nullptr)
        return LIST_PTR_IS_NULL;

    if (capacity == 0)
        return LIST_CTY_CANT_BE_ZERO;

    if (ListIsEmpty(list)      != LIST_IS_EMPTY && 
        ListIsDestructed(list) != LIST_IS_DESTRUCTED)
        return LIST_ISNT_EMPTY;

    list->data = (Elem_t__ *) calloc(capacity + 1, sizeof(Elem_t__));
    if (list->data == nullptr)
        return LIST_BAD_ALLOC;
    
    list->capacity = capacity;
    list->front    = 1;
    list->back     = 1;
    list->free     = 1;
    list->sorted   = 1;
    list->size     = 0;

    for (size_t index = 1; index < capacity; ++index)
    {
        list->data[index].next  = index + 1;
        list->data[index].prev  = FREE_INDEX;
        list->data[index].value = DEAD_VALUE;
    }

    list->data[capacity].next  = 0;
    list->data[capacity].prev  = FREE_INDEX;
    list->data[capacity].value = DEAD_VALUE;

    return LIST_IS_OK;
}

StatusCode ListDtor(List_t *list)
{
    if (list == nullptr)
        return LIST_PTR_IS_NULL;

    if (ListIsDestructed(list) == LIST_IS_DESTRUCTED)
        return LIST_IS_DESTRUCTED;

    if (ListVerify(list) == LIST_IS_OK)
        for (size_t index = 0; index < list->capacity + 1; ++index)
        {
            list->data[index].next  = 0xDED32DED;
            list->data[index].prev  = 0xDED32DED;
            list->data[index].value = DEAD_VALUE;
        }

    free(list->data);

    list->data = (Elem_t__ *) (1000 - 7);

    list->back     = 0xEBA1;
    list->capacity = 0xDEAD;

    list->free     = 0xFAAC;
    list->front    = 0xF1FA;

    list->size     = 0xBABE;
    list->sorted   = 0x77;

    list->error    = (StatusCode) 0xD70D;

    return LIST_IS_DESTRUCTED;
}

StatusCode ListIsDestructed(List_t *list)
{
    if (list == nullptr)
        return LIST_PTR_IS_NULL;

    if (list->data   == (Elem_t__ *)(1000 - 7)             &&
        list->back   == 0xEBA1 && list->capacity == 0xDEAD &&
        list->free   == 0xFAAC && list->front    == 0xF1FA &&
        list->size   == 0xBABE && list->sorted   == 0x77   &&
        list->error    == (StatusCode) 0xD70D)
        return LIST_IS_DESTRUCTED;
    
    return LIST_STATUS_UNKNOWN;
}

StatusCode ListIsEmpty(List_t *list)
{
    if (list == nullptr)
        return LIST_PTR_IS_NULL;
    
    if (list->back == 0 && list->front == 0 && list->sorted   == 0 &&
        list->free == 0 && list->error == 0 && list->capacity == 0 &&
        list->size == 0 && list->data == nullptr)
        return LIST_IS_EMPTY;
    
    return LIST_STATUS_UNKNOWN;
}

// logical first elem has index 1
size_t     ListReturnPhysIndex(List_t *list, size_t logical_index)
{
    if (ListVerify(list) != LIST_IS_OK  ||
        logical_index >  list->capacity ||
        logical_index == 0)
        return 0;

    if (list->sorted)
        return list->front + logical_index - 1;

    size_t counter = 1;
    size_t index = list->front;

    while (counter < logical_index)
    {
        index = list->data[index].next;
        counter++;
    }

    return index;
}       

size_t     ListFront (List_t *list)
{
    if (ListVerify(list) != LIST_IS_OK)
        return 0;
    
    return list->front;
}

size_t     ListBack  (List_t *list)
{
    if (ListVerify(list) != LIST_IS_OK)
        return 0;
    
    return list->back;
}

size_t     ListNext  (List_t *list, size_t physical_index)
{
    if (ListVerify(list) != LIST_IS_OK ||
        physical_index == 0            ||
        physical_index > list->capacity)
        return 0;
    
    return list->data[physical_index].next;
}

size_t     ListPrev  (List_t *list, size_t physical_index)
{
    if (ListVerify(list) != LIST_IS_OK ||
        physical_index == 0            ||
        physical_index > list->capacity)
        return 0;
    
    return list->data[physical_index].prev;
}

static Elem_t__ *ListResize(List_t *list, ResizeMode mode)
{
    if (ListVerify(list) != LIST_IS_OK)
        return nullptr;
    
    size_t new_capacity = 0;
    
    if (mode == INCREASE_CTY)
    {
        new_capacity = list->capacity * 2;

        Elem_t__ *new_data = (Elem_t__ *) realloc(list->data, (1 + new_capacity) * sizeof(Elem_t__));
        if (new_data == nullptr)
        {
            return nullptr;
        }

        size_t new_free = list->capacity + 1;

        size_t elem = list->free;
        while (new_data[elem].next != 0)
        {
            elem = list->data[elem].next;
        }

        if (elem == list->free)
            list->free = new_free;
        else
            new_data[elem].next = new_free;

        for (size_t index = new_free; index < new_free + list->capacity; ++index)
        {
            new_data[index].prev  = FREE_INDEX;
            new_data[index].next  = index + 1;
            new_data[index].value = DEAD_VALUE;
        }

        new_data[new_capacity].next = 0;

        list->capacity = new_capacity;

        return new_data;
    }
    else
    if (mode == DECREASE_CTY)
    {
        new_capacity = list->capacity / 2;
        assert(list->size <= new_capacity);

        if (new_capacity < 4) new_capacity = 4;

        if (new_capacity == list->capacity)
            return list->data;

        if (list->sorted == 0 ||
            list->sorted == 1 && list->back > new_capacity)
        {   
            StatusCode status = ListLinearize(list);
            if (status != LIST_IS_OK)
                return nullptr;
        }

        list->free = 0;
        for (size_t index = new_capacity; index >= 1; --index)
        {
            if (list->data[index].prev == FREE_INDEX)
            {
                if (list->free == 0)
                {
                    list->free = index;
                    list->data[index].next = 0;
                }
                else
                {
                    list->data[index].next = list->free;
                    list->free = index;
                }
            }
        }

        Elem_t__ *new_data = (Elem_t__ *) realloc(list->data, (1 + new_capacity) * sizeof(Elem_t__));
        if (new_data == nullptr)
        {
            list->error |= LIST_BAD_ALLOC;
            return nullptr;
        }

        list->capacity = new_capacity;

        return new_data;
    }
    else
    {
        list->error |= LIST_WRONG_RESIZE_MODE;
        return nullptr;
    }

    return nullptr;
}

StatusCode ListLinearize(List_t *list)
{
    StatusCode status = ListVerify(list);
    if (status != LIST_IS_OK)
        return status;

    if (list->sorted == 1 && list->data[1].prev != FREE_INDEX)
        return LIST_IS_OK;
        
    Elem_t__ *new_data = (Elem_t__ *) calloc(1 + list->capacity, sizeof(Elem_t__));
    if (new_data == nullptr)
        return LIST_BAD_ALLOC;

    new_data[0].prev  = 0;
    new_data[0].next  = 0;
    new_data[0].value = EMPTY_VALUE;

    size_t cur_ptr = list->front;
    if (list->size == 0)
        cur_ptr = 0;

    list->free = 0;
    for (size_t index = 1; index <= list->capacity; ++index)
    {
        if (cur_ptr != 0)
        {
            new_data[index].prev  = (index == 1          ? 0 : index - 1);
            new_data[index].next  = (index == list->size ? 0 : index + 1);
            new_data[index].value = list->data[cur_ptr].value;
        }
        else
        {
            if (list->free == 0) list->free = index;
            
            new_data[index].prev  = FREE_INDEX;
            new_data[index].next  = (index == list->capacity ? 0 : index + 1);
            new_data[index].value = DEAD_VALUE;
        }

        cur_ptr = list->data[cur_ptr].next;
    }

    free(list->data);

    list->sorted = 1;
    list->front  = 1;
    list->back   = (list->size == 0 ? 1 : list->size);
    list->data   = new_data;

    return LIST_IS_OK;
}

StatusCode ListInsertAfter (List_t *list, size_t physical_index, Val_t value)
{
    StatusCode status = ListVerify(list);
    if (status != LIST_IS_OK)
        return status;

    if (physical_index == 0 || physical_index > list->capacity
        ||
        list->data[physical_index].prev == FREE_INDEX &&
       (list->size != 0 || physical_index != list->back))
        return INVALID_INSERT_INDEX;

    if (physical_index != list->back)
        list->sorted = 0;

    if (list->free == 0)
    {
        Elem_t__ *ptr = ListResize(list, INCREASE_CTY);
        if (ptr == nullptr)
        {
            return LIST_BAD_ALLOC;
        }

        list->data = ptr;
    }

    assert(list->free != 0);

    if (list->back == physical_index &&
        list->size == 0)
    {
        assert(list->free == list->back);

        list->free = list->data[list->free].next;

        list->data[list->back].value = value;
        list->data[list->back].next  = 0;
        list->data[list->back].prev  = 0;
    }
    else
    if (list->back == physical_index)
    {
        if (list->free != list->back + 1)
            list->sorted = 0;

        list->data[list->free].prev  = list->back;
        list->data[list->free].value = value;

        size_t index_new = list->free;

        list->data[list->back].next  = index_new;
        list->back = index_new;

        list->free = list->data[index_new].next;
        list->data[index_new].next   = 0;
    }
    else
    {
        list->sorted = 0;

        size_t new_free = list->data[list->free].next;

        list->data[list->free].next  = list->data[physical_index].next;
        list->data[list->free].prev  = physical_index;
        list->data[list->free].value = value;

        list->data[list->data[list->free].prev].next = list->free;
        list->data[list->data[list->free].next].prev = list->free;

        list->free = new_free;
    }

    list->size++;

    status = ListVerify(list);
    if (status != LIST_IS_OK)
        return status;

    return LIST_IS_OK;
}

StatusCode ListInsertBefore(List_t *list, size_t physical_index, Val_t value)
{
    StatusCode status = ListVerify(list);
    if (status != LIST_IS_OK)
        return status;

    if (physical_index > list->capacity + 1 ||
        physical_index == 0)
        return INVALID_INSERT_INDEX;

    // i can maximize linearization, but i will not
    if (physical_index != list->front ||
        list->front != list->free + 1 || list->free == 0)
        list->sorted = 0;
    
    if (list->front == list->back)
        return ListInsertAfter(list, physical_index, value);

    if (list->free == 0)
    {
        Elem_t__ *ptr = ListResize(list, INCREASE_CTY);
        if (ptr == nullptr)
        {
            return LIST_BAD_ALLOC;
        }

        list->data = ptr;
    }

    assert(list->free != 0);

    if (physical_index == list->front)
    {
        size_t new_free = list->data[list->free].next;

        list->data[list->front].prev = list->free;
        
        list->data[list->free].next  = list->front;
        list->data[list->free].prev  = 0;
        list->data[list->free].value = value;

        list->front = list->free;
        list->free  = new_free;

        list->size++;
    }
    else
    {
        return ListInsertAfter(list, list->data[physical_index].prev, value);
    }

    status = ListVerify(list);
    if (status != LIST_IS_OK)
        return status;

    return LIST_IS_OK;
}

StatusCode ListPushBack(List_t *list, Val_t value)
{
    return ListInsertAfter(list, ListBack(list), value);
}

StatusCode ListPushFront(List_t *list, Val_t value)
{
    return ListInsertBefore(list, ListFront(list), value);
}

Val_t      ListRemove(List_t *list, size_t physical_index)
{
    if (ListVerify(list) != LIST_IS_OK)
        return DEAD_VALUE;
    
    if (physical_index == 0 || physical_index > list->capacity + 1 ||
        list->data[physical_index].prev == FREE_INDEX)
        return DEAD_VALUE;
    
    list->size--;

    Val_t value = DEAD_VALUE;

    if (physical_index == list->front)
    {
        if (physical_index == list->back)
            if (list->data[physical_index].prev == FREE_INDEX)
            {
                //  i do list->size-- for all cases
                //  so I have to increment in this case
                list->size++;
                
                assert(list->size == 0);
                return DEAD_VALUE;
            }
            else
            {
                value = list->data[physical_index].value;
                
                list->data[physical_index].prev  = FREE_INDEX;
                list->data[physical_index].next  = list->free;
                list->data[physical_index].value = DEAD_VALUE;

                list->free = physical_index;
            }
        else
        {
            value = list->data[list->front].value;

            size_t new_front = list->data[list->front].next;
                
            list->data[new_front].prev  = 0;
                
            list->data[list->front].prev  = FREE_INDEX;
            list->data[list->front].next  = list->free;
            list->data[list->front].value = DEAD_VALUE;

            list->free  = list->front;

            list->front = new_front;
        }
    }
    else
    if (physical_index == list->back)
    {
        value = list->data[list->back].value;

        size_t new_back = list->data[list->back].prev;

        list->data[new_back].next = 0;
        
        list->data[list->back].prev  = FREE_INDEX;
        list->data[list->back].next  = list->free;
        list->data[list->back].value = DEAD_VALUE;

        list->free = list->back;

        list->back = new_back;
    }
    else
    {
        list->sorted = 0;

        size_t next = list->data[physical_index].next;
        size_t prev = list->data[physical_index].prev;
    
        value = list->data[physical_index].value;

        list->data[physical_index].prev  = FREE_INDEX;
        list->data[physical_index].next  = list->free;
        list->data[physical_index].value = DEAD_VALUE;

        list->free = physical_index;

        list->data[next].prev = prev;
        list->data[prev].next = next;
    }

    if (list->size <= list->capacity / 4)
    {
        Elem_t__ *ptr = ListResize(list, DECREASE_CTY);
        if (ptr == nullptr)
        {
            return DEAD_VALUE;
        }

        list->data = ptr;
    }

    return value;
}

Val_t      ListPopBack (List_t *list)
{
    return ListRemove(list, ListBack(list));
}

Val_t      ListPopFront(List_t *list)
{
    return ListRemove(list, ListFront(list));
}

StatusCode ListClear   (List_t *list)
{
    int status = ListVerify(list);
    if (status != LIST_IS_OK)
        return (StatusCode) status;

    while (list->size != 0)
        ListPopBack(list);

    status |= ListLinearize(list);

    return (StatusCode) status;
}

// returns physical index when found
// returns 0 if not found or there is some error
size_t     ListValueIndex(List_t *list, int64_t hash)
{
    if (ListVerify(list) != LIST_IS_OK)
        return 0;
    
    size_t elem = list->front;

    while (elem)
    {
        if (list->data[elem].value.hash == hash)
        {
            // PRINT(NOT_EXACT_COMPARING_HASH);
            return elem;
        }

        elem = list->data[elem].next;
    }

    return elem;
}

// size_t     ListValueIndex(List_t *list, const char *str)
// {
//     if (ListVerify(list) != LIST_IS_OK)
//         return 0;
//     if (str == nullptr) return 0;

//     size_t elem = list->front;
//     // TODO
//     // FUNC EQUAL THAT GIVES USER
//     while (elem != 0 && strcmp(list->data[elem].value.name, str) != 0)
//         elem = list->data[elem].next;

//     return elem;
// }

StatusCode ListVerify(List_t *list)
{
    if (list == nullptr)
        return LIST_PTR_IS_NULL;

    if (ListIsEmpty(list) == LIST_IS_EMPTY)
        return LIST_IS_EMPTY;
    
    if (ListIsDestructed(list) == LIST_IS_DESTRUCTED)
        return LIST_IS_DESTRUCTED;

    int error = LIST_IS_OK;

    if (list->data == nullptr)
        error |= DATA_PTR_IS_NULL;

    if (list->front > list->capacity)
        error |= (LIST_FRONT_BIGGER_THAN_CTY | LIST_INDEXES_RUINED);

    if (list->back  > list->capacity)
        error |= (LIST_BACK__BIGGER_THAN_CTY | LIST_INDEXES_RUINED);

    if (list->size > list->capacity)
        error |= LIST_SIZE_RUINED;
    
    if (list->sorted > 1)
        error |= LIST_SORTED_RUINED;
    
    if (list->free   > list->capacity)
        error |= (LIST_FREE_RUINED | LIST_INDEXES_RUINED);

    if (list->data[0].next  != 0 ||
        list->data[0].prev  != 0 ||
        // func empty elem
        list->data[0].value.type != 0 && list->data[0].value.offset != 0)
        error |= LIST_ZERO_INDEX_VAL_RUINED;

    if (error != LIST_IS_OK)
        return (StatusCode) error;

    if (list->front == list->back)
    {   
        if (!(list->data[list->front].next  == 0 &&
              list->data[list->front].prev  == 0 && 
              list->size == 1
              ||
              list->data[list->front].prev  == FREE_INDEX &&
              list->data[list->front].value.type == DEAD_VALUE.type &&
              list->size == 0))
            error |= LIST_DATA_RUINED;
        
        for (size_t index = 1; index <= list->capacity; ++index)
        {
            if (index != list->front)
            {   
                // function compare
                if (list->data[index].value.type != DEAD_VALUE.type ||
                    list->data[index].prev  != FREE_INDEX)
                {
                    error |= LIST_DATA_RUINED;
                    break;
                }
            }
        }
    }

    if (error != LIST_IS_OK)
        return (StatusCode) error;

    if (list->front != list->back &&
        list->data[list->front].prev != 0)
        error |= LIST_FRONT_RUINED;
    
    if (list->front != list->back && 
        list->data[list->back].next != 0)
        error |= LIST_BACK_RUINED;

    if (error != LIST_IS_OK)
        return (StatusCode) error;

    for (size_t index = 1; index <= list->capacity; ++index)
    {
        if (list->data[index].next  >  list->capacity
            ||
           (list->data[index].prev  >  list->capacity &&
            list->data[index].prev  != FREE_INDEX)
            ||
           (list->data[index].prev  == FREE_INDEX     && 
            list->data[index].value.type != DEAD_VALUE.type))
        {
            error |= LIST_INDEXES_RUINED;
            break;
        }
    }

    if (error != LIST_IS_OK)
        return (StatusCode) error;

    size_t size  = 0;
    size_t place = list->front;
    
    while (list->front != list->back && size < list->capacity)
    {
        if (list->data[place].next == 0)
        {
            if (place != list->back)
                error |= LIST_DATA_RUINED;
            size++;
            break;
        }
        else
        if (list->data[list->data[place].next].prev != place
            ||
            list->data[place].prev != 0 && 
            list->data[list->data[place].prev].next != place)
        {
            error |= LIST_DATA_RUINED;
            break;
        }

        place = list->data[place].next;
        size++;
    }

    if (list->front == list->back && list->data[list->front].prev != FREE_INDEX)
        size++;

    if (size  != list->size)
        error |= LIST_SIZE_RUINED;
    
    place = list->free;
    
    while (size < list->capacity && place != 0)
    {
        if (list->data[place].next == 0)
        {
            if (list->data[place].prev  != FREE_INDEX ||
                list->data[place].value.type != DEAD_VALUE.type)
                error |= LIST_DATA_RUINED;
            size++;
            break;
        }
        else
        if (list->data[place].prev  != FREE_INDEX ||
            list->data[place].value.type != DEAD_VALUE.type)
        {
            error |= LIST_DATA_RUINED;
            break;
        }
        
        place = list->data[place].next;
        size++;
    }

    if (size != list->capacity)
        error |= LIST_DATA_RUINED;

    return (StatusCode) error;
}

static const char *ColorPicker(List_t *list, size_t index)
{
    const char *colors[] =
    {
        "#E6A8D7", // [0] pink
        "#42AAFF", // [1] cyan
        "#1CD3A2", // [2] green
        "#FF7538", // [3] orange
        "#E34234", // [4] red
        "#5035DE", // [5] (almost) royal blue
        "#EED202", // [6] YELLOW WARNING
    };

    if ((ListVerify(list) & LIST_INDEXES_RUINED) != 0)
        return colors[6];

    if (index <= list->capacity + 1 && list->data[index].prev == FREE_INDEX)
        return colors[1];
    else
    if (index == 0)
        return colors[5];
    else
    if (index == list->back)
        return colors[3];
    else
    if (index == list->front)
        return colors[2];
    else
    if (index > list->capacity + 1)
        return colors[4];
    else
        return colors[0];
}

StatusCode ListDump(List_t *list)
{
    if ((ListVerify(list) & LIST_INDEXES_RUINED) != 0)
    {
        fprintf(stderr, "CANT DUMP. LIST INDEXES ARE RUINED\n");
        return LIST_INDEXES_RUINED;
    }

    FILE *dump_file = fopen("dumpList", "w");

    fputs("digraph structs {\n", dump_file);

    fputs("    node [color=black, shape=box, style=\"rounded, filled\"];\n", dump_file);
    
    fputs("    rankdir=LR;\n", dump_file);
  
    fprintf(dump_file, "    front [fillcolor=\"%s\", "
                       "    label=\"FRONT = %lu\"];\n",
                       ColorPicker(list, list->front), list->front);

    fprintf(dump_file, "    back  [fillcolor=\"%s\", "
                       "    label=\"BACK = %lu\"];\n",
                       ColorPicker(list, list->back), list->back);

    fprintf(dump_file, "    free  [fillcolor=\"%s\", "
                       "    label=\"FREE = %lu\"];\n",
                       ColorPicker(list, list->free), list->free);

    fprintf(dump_file, "    sorted [fillcolor=\"%s\","
                       "    label=\"SORTED = %d\"];\n",
                       ColorPicker(list, FREE_INDEX / 2), list->sorted);

    fprintf(dump_file, "    size   [fillcolor=\"%s\","
                       "    label=\"SIZE = %lu\"];\n",
                       ColorPicker(list, FREE_INDEX / 2), list->size);                     

    fputs("    node [color=black, shape=record, style=\"rounded, filled\"];\n", dump_file);
    fputs("\n", dump_file);
    fputs("    edge [style=invis, constraint=true];\n", dump_file);

    for (size_t index = 0; index <= list->capacity; ++index)
    {
        // Yes it is bad, but I dont care right now.
        auto value = list->data[index].value;

        fprintf(dump_file, "    node%lu [fillcolor=\"%s\","
                           "label=\" %lu | { <p> %ld | ",
                                index, ColorPicker(list, index), index,
                                (list->data[index].prev == FREE_INDEX) ? -1 : list->data[index].prev);

        if (value.type == Variable_t)
            fprintf(dump_file, "VAR  %s ",    value.elem.variable);
        else
        if (value.type == Function_t)
            fprintf(dump_file, "FUNC %s ",    value.elem.func_ptr);
        else
        if (value.type == Keyword_t)
            fprintf(dump_file, "KEYW %x %d ", value.elem.keyword_id, value.elem.number);
        else
            fprintf(dump_file, "DEAD ");

        fprintf(dump_file, "%ld | <n> %lu}\"];\n", value.offset, list->data[index].next);
        
        if (index > 0) fprintf(dump_file, "    node%lu -> node%lu;\n", index - 1, index);
    }

    fputs("\n    edge [style=solid, constraint=false];\n", dump_file);

    for (size_t index = 1; index <= list->capacity; ++index)
    {
        if (list->data[index].next != 0)
            fprintf(dump_file, "    node%lu: <n> -> node%lu;\n", index, list->data[index].next);
        if (list->data[index].prev != FREE_INDEX && list->data[index].prev != 0)
            fprintf(dump_file, "    node%lu: <p> -> node%lu;\n", index, list->data[index].prev);
        fputs("\n", dump_file);
    }

    fputs("\n    edge [style=bold, constraint=false];\n", dump_file);

    fprintf(dump_file, "    front -> node%lu; \n", list->front);
    fprintf(dump_file, "    back  -> node%lu; \n", list->back );
    fprintf(dump_file, "    free  -> node%lu; \n", list->free );

    fputs("}\n", dump_file);

    fclose(dump_file);

    system("dot dumpList -T png -o dumpList.png");
    system("gwenview dumpList.png");

    return DUMPED;
}
