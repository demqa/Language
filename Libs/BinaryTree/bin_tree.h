#ifndef BIN_TREE_H
#define BIN_TREE_H

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>

#include <sys/stat.h>

#include "bin_tree_config.h"

enum TreeStatus
{
    // statuses
    TREE_IS_OK                    = 0,
    NODE_IS_OK                    = 0,
 
    TREE_IS_EMPTY                 = 0,

    NODE_VALUE_IS_NULL            = 1 << 1,
    TREE_VALUE_IS_NULL            = 1 << 1,

    TREE_IS_NOT_EMPTY             = 1 << 2,
    TREE_IS_DTORED                = 1 << 3,
 
    NODE_IS_DTORED                = 1 << 4,
    NODE_IS_TERMINAL              = 1 << 5,

    // errors             
    BAD_ALLOC                     = 1 << 9,
 
    TREE_IS_NULL                  = 1 << 10,
    TREE_ROOT_IS_NULL             = 1 << 11,
    NODE_PTR_IS_NULL              = 1 << 12,
      
    NODE_PARENT_IS_INVALID        = 1 << 13,
    NODE_LEFT___IS_INVALID        = 1 << 14,
    NODE_RIGHT__IS_INVALID        = 1 << 15,
      
    TREE_ROOT_IS_RUINED           = 1 << 16,
    TREE_CANT_HAVE_THIS_CHILD     = 1 << 17,
 
    NUMBER_OF_NODE_NULL           = 1 << 18,
    DUMP_FILE_IS_NULL             = 1 << 19,
 
    CANT_REMOVE_TREE_ROOT         = 1 << 20,
    CANT_REMOVE_NON_TERMINAL_NODE = 1 << 21,

    UNBELIEVABLE_CASE             = 1 << 30,
    RESULT_IS_UNKNOWN             = 1 << 31,
};

struct Node_t
{
    Value_t *value;

    Node_t  *parent;
    Node_t  *left;
    Node_t  *right;

};

struct Tree_t
{
    Node_t    *root;

    size_t     size;
    TreeStatus status;
};

enum ChildNumeration
// very tolerant naming
{
    L_CHILD = 0,
    R_CHILD = 1,
};

TreeStatus TreeCtor      (Tree_t *tree);
TreeStatus TreeIsEmpty   (Tree_t *tree);

TreeStatus TreeDtor      (Tree_t *tree);
TreeStatus TreeIsDtored  (Tree_t *tree);
  
TreeStatus NodeInsert    (Tree_t *tree, Node_t *node, const ChildNumeration n_child, const Value_t *value);
TreeStatus NodeRemove    (Tree_t *tree, Node_t *node);
TreeStatus NodesDtor     (Node_t *node);

TreeStatus NodeIsTerminal(const Node_t *node);

TreeStatus NodeVerify    (const Node_t *node);
TreeStatus TreeVerify    (Tree_t *tree);
  
TreeStatus TreeDump      (Tree_t *tree);

#endif // BIN_TREE_H
