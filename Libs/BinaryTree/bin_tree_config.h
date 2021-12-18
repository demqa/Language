#ifndef BIN_TREE_CONFIG_H
#define BIN_TREE_CONFIG_H

#include <assert.h>

#include "../Debug/debug_lib.h"
#include "../Tokens/tokens.h"

enum ValueType
{
    ID_TYPE   = 0x1000, // IDENTIFIER
    NUM_TYPE  = 0x2000, // NUMBER
    KEYW_TYPE = 0x3000, // KEYWORD
};

typedef Token_t Value_t;

const Token_t DEAD_ELEM = {300};

// FILE *TREE_LOG_FILE_DEFAULT = stderr;

const size_t COLOR_MAX_LEN = 10 + 1;
const size_t SHAPE_MAX_LEN = 14 + 1;

const char  left_color [COLOR_MAX_LEN] = "#7100c7";
const char right_color [COLOR_MAX_LEN] = "#7100c7";

const char colors[]    [COLOR_MAX_LEN] =
{
    "#ff002f",
    "#f6fa00",
    "#14d4ff",
    "#00ed14",
    "#ff8b1f",
};

const char shapes[]    [SHAPE_MAX_LEN] = 
{
    "octagon",
    "hexagon",
    "invtriangle",
    "parallelogram",
    "ellipse",
};

#endif // BIN_TREE_CONFIG_H
