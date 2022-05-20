#ifndef LIST_H
#define LIST_H

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "list_config.h"

enum StatusCode
{
    LIST_IS_OK = 0,

    // STATUS_CODES

    LIST_STATUS_UNKNOWN        = 1 << 28,
    LIST_IS_EMPTY              = 1 << 29,
    LIST_ISNT_EMPTY            = 1 << 30,
    LIST_IS_DESTRUCTED         = 1 << 31,

    // ERROR_CODES

    LIST_PTR_IS_NULL           = 1 << 1,
    DATA_PTR_IS_NULL           = 1 << 2,

    LIST_BAD_ALLOC             = 1 << 3,

    LIST_CTY_CANT_BE_ZERO      = 1 << 4,

    LIST_FRONT_BIGGER_THAN_CTY = 1 << 5,
    LIST_BACK__BIGGER_THAN_CTY = 1 << 6,

    LIST_SIZE_RUINED           = 1 << 7,
    LIST_SORTED_RUINED         = 1 << 8,
    LIST_FREE_RUINED           = 1 << 9,

    LIST_ZERO_INDEX_VAL_RUINED = 1 << 10,

    LIST_INDEXES_RUINED        = 1 << 11,
    LIST_FRONT_RUINED          = 1 << 12,
    LIST_BACK_RUINED           = 1 << 13,

    LIST_DATA_RUINED           = 1 << 14,

    // PROCESSING_ERRORS

    INVALID_INSERT_INDEX       = 1 << 15,
    LIST_WRONG_RESIZE_MODE     = 1 << 16,


    DUMPED                     = 1 << 28,

};

enum ResizeMode
{
    INCREASE_CTY  = 0xEA71,
    DECREASE_CTY  = 0xDE7A,
};

struct Elem_t__
{
    Val_t value;

    size_t prev;
    size_t next;
};

struct List_t
{
    Elem_t__ *data;

    size_t front;
    size_t back;

    size_t size;
    size_t capacity;
    size_t free;

    u_int8_t sorted;

    int error;
};

const size_t FREE_INDEX = (size_t)(-1);

StatusCode ListCtor  (List_t *list, size_t initial_capacity);
/* invisible 4 user */
// Elem_t__ *ListResize   (List_t *list);
/*                  */
StatusCode ListDtor  (List_t *list);
StatusCode ListVerify(List_t *list);
StatusCode ListDump  (List_t *list);

StatusCode ListLinearize(List_t *list);

/* this function will not be called from list itself */
StatusCode ListClear    (List_t *list);

/* O(logical_index) when unsorted, O(1) when sorted */
size_t     ListReturnPhysIndex(List_t *list, size_t logical_index);

StatusCode ListInsertAfter (List_t *list, size_t physical_index, Val_t value);
StatusCode ListInsertBefore(List_t *list, size_t physical_index, Val_t value);

StatusCode ListPushBack (List_t *list, Val_t value);
StatusCode ListPushFront(List_t *list, Val_t value);

size_t     ListFront(List_t *list);
size_t     ListBack (List_t *list);

size_t     ListNext  (List_t *list, size_t physical_index);
size_t     ListPrev  (List_t *list, size_t physical_index);

Val_t      ListRemove(List_t *list, size_t physical_index);

Val_t      ListPopBack  (List_t *list);
Val_t      ListPopFront (List_t *list);

StatusCode ListIsEmpty     (List_t *list);
StatusCode ListIsDestructed(List_t *list);

StatusCode ListPrintErr  (List_t *list);
size_t     ListValueIndex(List_t *list, int64_t hash);

#endif // LIST_H
