#include "ast_comp.h"

int InitEmptyNode(Node_t **node_);

int FillTree    (Tree_t *tree, Tokens_t *tokens)
{
    int status = TreeVerify(tree);
    if (status) return status;

    status = TokensVerify(tokens);
    if (status) return status;

    size_t index = 0;
    tree->root = GetG(tokens, &index);

    return status;
}

int Require     (Tokens_t *tokens, size_t *index, const int symbol)
{
    int status = TokensVerify(tokens);
    CATCH_ERR;

    Token_t *token = nullptr;
    status = TokensElem(tokens, *index, &token);
    CATCH_ERR;

    if (token->type != KEYW_TYPE || token->arg.key_w != symbol)
    {
        fprintf(stderr, "INVALID INPUT FORMAT\nEXPECTED: ");

        #define DEF_KEYW(DEF, CODE, WORD, FMT) \
            case KEYW_ ## DEF:                  \
                fprintf(stderr, "%s\n", #DEF);   \
                break;
        
        #define DEF_OPER(DEF, CODE, OPER)           \
            case KEYW_ ## DEF:                       \
                fprintf(stderr, "%s\n", #OPER);       \
                break;

        #define DEF_HELP(DEF, CODE, SIGN)                \
            case KEYW_ ## DEF:                            \
                fprintf(stderr, "%c\n", SIGN);             \
                break;

        switch (symbol)
        {
            #include "../keywords"
            #include "../operators"
            
            default:
                fprintf(stderr, "UNDEFINED SYMBOL\n");
                break;
        }

        #undef DEF_KEYW
        #undef DEF_OPER
        #undef DEF_HELP

        fprintf(stderr, "INPUT:    ");
        PrintToken(tokens, *index);
        
        return ASTree::INVALID_SYNTAX;
    }
    
    (*index)++;

    return ASTree::FUNC_IS_OK;
}

Node_t *GetG(Tokens_t *tokens, size_t *index)
{
    int status = TokensVerify(tokens);
    CATCH_ERR;
    if (index == nullptr) return nullptr;

    Node_t *ast = GetGS(tokens, index);
    assert(ast != nullptr);

    if (Require(tokens, index, KEYW_EOF)) return nullptr;

    return ast;
}

int InitEmptyNode(Node_t **node_)
{
    if ( node_ == nullptr) return ASTree::NODE_IS_NULL;
    if (*node_ != nullptr) return ASTree::DESTPTR_ISNT_NULL;

    int status = ASTree::FUNC_IS_OK;

    Node_t *node = nullptr;

    node = (Node_t *) calloc(1, sizeof(Node_t));
    if (node == nullptr)   return ASTree::BAD_ALLOC;

    node->value = (Token_t *) calloc(1, sizeof(Token_t));
    if (node->value == nullptr) return ASTree::BAD_ALLOC;

    *node_ = node;

    return status;
}

int InitKeyword(Node_t **node, const int keyword)
{
    int status = InitEmptyNode(node);
    if (status) return status;

    Token_t *token = (*node)->value;

    // CHECK THAT KEYWORD IS KEYWORD
    token->type      = KEYW_TYPE;
    token->arg.key_w = keyword;

    return status;
}

int ConnectNodes(Node_t *parent, Node_t *node, ChildNumeration child)
{
    int status = NodeVerify(parent);
    if (status) return status;

    status = NodeVerify(node);
    if (status) return status;

    if (node->parent != nullptr)
    {
        PRINT(PROGRAMMER_IMBECILE);
        return ASTree::INVALID_CONNECTION_NODE;
    }

    if (child == L_CHILD)
    {
        if (parent->left != nullptr)
        {
            PRINT(LEFT_CHILD_ISNT_NULL);
            return ASTree::INVALID_CONNECTION_PAR;
        }

        node->parent  = parent;
        parent->left  = node;
    }
    else
    if (child == R_CHILD)
    {
        if (parent->right != nullptr)
        {
            PRINT(RIGHT_CHILD_ISNT_NULL);
            return ASTree::INVALID_CONNECTION_PAR;
        }

        node->parent  = parent;
        parent->right = node;
    }
    else
    {
        PRINT(CANT_ADD_N_CHILD);
        return ASTree::CHILD_IS_INVALID;
    }

    return ASTree::FUNC_IS_OK;
}

Node_t *GetToken(Tokens_t *tokens, size_t *index)
{
    int status = TokensVerify(tokens);
    CATCH_ERR;
    if (index == nullptr) return nullptr;

    Token_t *token = nullptr;
    status = TokensElem(tokens, *index, &token);
    CATCH_ERR;

    Node_t *node = nullptr;
    status = InitEmptyNode(&node);

    memcpy(node->value, token, sizeof(Token_t));

    (*index)++;

    return node;
}

Node_t *GetGS(Tokens_t *tokens, size_t *index)
{
    int status = TokensVerify(tokens);
    CATCH_ERR;
    if (index == nullptr) return nullptr;

    Token_t *token      = nullptr;

    Node_t  *glob_stmts = nullptr;
    Node_t  *current    = nullptr;

    Node_t  *stmt       = nullptr;

    while (*index < tokens->size)
    {
        token = nullptr;

        status = TokensElem(tokens, *index, &token);
        CATCH_ERR;

        if (TOKEN_KEYW(KEYW_EOF))
        {
            return glob_stmts;
        }

        stmt = nullptr;

        status = InitKeyword(&stmt, KEYW_STMT);
        CATCH_ERR;

        if (token->type == ID_TYPE /* || TOKEN_KEYW(KEYW_CONST) */)
        {
            Node_t *var    = GetVar(tokens, index);
            CATCH_NULL(current);

            Node_t *assign = GetToken(tokens, index);
            CATCH_NULL(assign);

            Node_t *expr   = GetE(tokens, index);
            CATCH_NULL(expr);

            if (Require(tokens, index, KEYW_DOTPOT)) return nullptr;

            status = ConnectNodes(assign,  var, L_CHILD);
            CATCH_ERR;

            status = ConnectNodes(assign, expr, R_CHILD);
            CATCH_ERR;

            current = assign;
        }
        else
        if (token->type == KEYW_TYPE)
        {
            current = GetFuncDef(tokens, index);
        }
        else
        {
            PRINT(INVALID_SYNTAX);
            NodesDtor(stmt);
            PrintToken(tokens, *index);
            return nullptr;
        }

        if (current != nullptr)
        {
            status = ConnectNodes(stmt, current, R_CHILD);
            CATCH_ERR;
        }
        
        if (glob_stmts != nullptr)
        {
            status = ConnectNodes(stmt, glob_stmts, L_CHILD);
            CATCH_ERR;
        }

        glob_stmts = stmt;
    }

    PRINT(EOF_WASNT_REACHED_THOUGH);
    return glob_stmts;
}

int GetKeyword(Tokens_t *tokens, size_t *index, Node_t **node, const int keyword)
{
    int status = TokensVerify(tokens);
    CATCH_ERR;
    if (index == nullptr)                     return ASTree::PTR_IS_NULL;
    if (node  == nullptr || *node == nullptr) return ASTree::NODE_IS_NULL;

    Token_t *token = nullptr;
    status = TokensElem(tokens, *index, &token);
    CATCH_ERR;

    if (!TOKEN_KEYW(keyword)) return ASTree::WRONG_KEYWORD;

    Node_t *new_node = nullptr;
    status = InitKeyword(&new_node, keyword);
    CATCH_ERR;

    (*index)++;

    return status;
}

Node_t *GetStmts(Tokens_t *tokens, size_t *index)
{
    int status = TokensVerify(tokens);
    CATCH_ERR;
    if (index == nullptr) return nullptr;

    Token_t *token = nullptr;
    status = TokensElem(tokens, *index, &token);

    if (Require(tokens, index, KEYW_OPFIG)) return nullptr;

    status = TokensElem(tokens, *index, &token);
    CATCH_ERR;

    if ( TOKEN_KEYW(KEYW_CLFIG)) return nullptr;

    Node_t *glob_stmts = nullptr;
    Node_t *stmt       = nullptr;

    Node_t *current    = nullptr;

    while (*index < tokens->size)
    {
        token = nullptr;

        status = TokensElem(tokens, *index, &token);
        CATCH_ERR;

        if (TOKEN_KEYW(KEYW_CLFIG))
        {
            (*index)++;
            return glob_stmts;
        }

        stmt = nullptr;

        status = InitKeyword(&stmt, KEYW_STMT);
        CATCH_ERR;

        current = GetStmt(tokens, index);

        status = ConnectNodes(stmt, current, R_CHILD);
        CATCH_ERR;

        if (glob_stmts != nullptr)
        {
            status = ConnectNodes(stmt, glob_stmts, L_CHILD);
            CATCH_ERR;
        }

        glob_stmts = stmt;
    }

    PRINT(CLOSE_FIGURE_BRACKET_WASNT_REACHED_THOUGH);
    return nullptr;

    // return glob_stmts;
}

Node_t *GetLogOp(Tokens_t *tokens, size_t *index)
{
    int status = TokensVerify(tokens);
    CATCH_ERR;
    if (index == nullptr) return nullptr;

    Token_t *token = nullptr;
    status = TokensElem(tokens, *index, &token);
    CATCH_ERR;

    if (token->type != KEYW_TYPE) return nullptr;

    if (token->arg.key_w < KEYW_EQUAL ||
        token->arg.key_w > KEYW_OR) return nullptr;

    return GetToken(tokens, index);
}

Node_t *GetCondition(Tokens_t *tokens, size_t *index)
{
    int status = TokensVerify(tokens);
    CATCH_ERR;
    if (index == nullptr) return nullptr;

    // NOW IT WILL BE THAT WAY
    // L_EXPR LOGICAL_OP R_EXPR

    if (Require(tokens, index, KEYW_OPRND)) return nullptr;

    Node_t *l_expr = GetE(tokens, index);
    CATCH_NULL(l_expr);

    Node_t *log_op = GetLogOp(tokens, index);
    CATCH_NULL(log_op);

    Node_t *r_expr = GetE(tokens, index);
    CATCH_NULL(r_expr);

    status = ConnectNodes(log_op, l_expr, L_CHILD);
    CATCH_ERR;

    status = ConnectNodes(log_op, r_expr, R_CHILD);
    CATCH_ERR;

    if (Require(tokens, index, KEYW_CLRND)) return nullptr;

    return log_op;
}

Node_t *GetWhile(Tokens_t *tokens, size_t *index)
{
    int status = TokensVerify(tokens);
    CATCH_ERR;
    if (index == nullptr) return nullptr;

    Node_t *while_node = nullptr;
    status = GetKeyword(tokens, index, &while_node, KEYW_WHILE);
    CATCH_ERR;

    Node_t *condition = GetCondition(tokens, index);
    CATCH_NULL(condition);

    status = ConnectNodes(while_node,  condition, L_CHILD);
    CATCH_ERR;

    Node_t *statements = GetStmts(tokens, index);
    CATCH_NULL(statements);

    status = ConnectNodes(while_node, statements, R_CHILD);
    CATCH_ERR;

    return while_node;
}

Node_t *GetIf(Tokens_t *tokens, size_t *index)
{
    int status = TokensVerify(tokens);
    CATCH_ERR;
    if (index == nullptr) return nullptr;

    Node_t *if_node = nullptr;
    status = GetKeyword(tokens, index, &if_node, KEYW_IF);
    CATCH_ERR;

    Node_t *condition = GetCondition(tokens, index);
    CATCH_NULL(condition);

    status = ConnectNodes(if_node,   condition, L_CHILD);
    CATCH_ERR;

    Token_t *token = nullptr;
    status = TokensElem(tokens, *index, &token);
    CATCH_ERR;

    if (!TOKEN_KEYW(KEYW_THEN)) return nullptr;

    (*index)++;

    Node_t *decision = nullptr;
    status = InitKeyword(&decision, KEYW_DECISION);
    CATCH_ERR;

    status = ConnectNodes(if_node,    decision, R_CHILD);
    CATCH_ERR;

    Node_t *statements = GetStmts(tokens, index);
    CATCH_NULL(statements);

    status = ConnectNodes(decision, statements, L_CHILD);
    CATCH_ERR;

    status = TokensElem(tokens, *index, &token);
    CATCH_ERR;

    if (!TOKEN_KEYW(KEYW_ELSE))
        return if_node;

    (*index)++;

    Node_t *else_statements = GetStmts(tokens, index);
    CATCH_NULL(statements);

    status = ConnectNodes(decision, else_statements, R_CHILD);
    CATCH_ERR;

    return if_node;
}

Node_t *GetReturn(Tokens_t *tokens, size_t *index)
{
    int status = TokensVerify(tokens);
    CATCH_ERR;
    if (index == nullptr) return nullptr;

    Node_t *ret_node = nullptr;
    status = GetKeyword(tokens, index, &ret_node, KEYW_RETURN);
    CATCH_ERR;
    
    Node_t *expr = GetE(tokens, index);
    CATCH_NULL(expr);

    status = ConnectNodes(ret_node, expr, R_CHILD);
    CATCH_ERR;

    return ret_node;
}

Node_t *GetStmt (Tokens_t *tokens, size_t *index)
{
    int status = TokensVerify(tokens);
    CATCH_ERR;
    if (index == nullptr) return nullptr;

    Token_t *token = nullptr;
    status = TokensElem(tokens, *index, &token);
    CATCH_ERR;

    Node_t *stmt_child = nullptr;

    if (token->type == ID_TYPE /* || TOKEN_KEYW(KEYW_CONST) */)
    {
        Token_t *first_token = token;

        GET_NEXT_TOKEN;
        
        // CASE 1: ASSIGNMENT
        //   TODO: ADD FUNCTION OF SEARCHING SOME KEYWORD AMONG NEXT N TOKENS
        //         & THEN I CAN DO CONSTANSTS & ASSIGNS IN EASY & LACONIC WAY

        if (TOKEN_KEYW(KEYW_ASSIGN))
        {
            (*index)--;

            Node_t *variable = GetVar(tokens, index);
            CATCH_NULL(variable);

            Node_t *assign   = GetToken(tokens, index);
            CATCH_NULL(assign);
            
            Node_t *expr     = GetE(tokens, index);
            CATCH_NULL(expr);

            if (Require(tokens, index, KEYW_DOTPOT)) return nullptr;

            status = ConnectNodes(assign, variable, L_CHILD);
            CATCH_ERR;

            status = ConnectNodes(assign,     expr, R_CHILD);
            CATCH_ERR;

            stmt_child = assign;

            return stmt_child;
        }
        else
        {
            // NOT_ASSIGNMENT
            (*index)--;
            token = first_token;
        }
    }
    else
    if (token->type == KEYW_TYPE)
    {
        if (TOKEN_KEYW(KEYW_IF))
        {
            return GetIf(tokens, index);
        }
        else
        if (TOKEN_KEYW(KEYW_WHILE))
        {
            return GetWhile(tokens, index);
        }
        if (TOKEN_KEYW(KEYW_RETURN))
        {
            return GetReturn(tokens, index);
        }
        else
        {
            PRINT(UNDEFINED_KEYWORD);
            TOKEN;
            return nullptr;
        }
    }

    // CASE 2: EXPRESSION

    // TODO:
    // CASE 3: return

    // CASE 4: if

    // CASE 5: while

    stmt_child = GetE(tokens, index);
    CATCH_NULL(stmt_child);

    if (Require(tokens, index, KEYW_DOTPOT)) return nullptr;

    return stmt_child;
}

Node_t *GetVar(Tokens_t *tokens, size_t *index)
{
    int status = TokensVerify(tokens);
    CATCH_ERR;
    if (index == nullptr) return nullptr;
/*
    TODO: const type of variables
    NOW: only variable variables

    Token_t *token = nullptr;
    status = TokensElem(tokens, *index, &token);
    CATCH_ERR;

    if (TOKEN_KEYW(KEYW_CONST))
    {
        GET_NEXT_TOKEN;
        if (token->type != ID_TYPE)
        {
            PRINT(INVALID_CONST_SYNTAX);
            return nullptr;
        }
    }
*/
    Node_t *variable = GetIdentifier(tokens, index);
    CATCH_NULL(variable);

    return variable;
}

Node_t *GetFuncDef(Tokens_t *tokens, size_t *index)
{
    int status = TokensVerify(tokens);
    CATCH_ERR;
    if (index == nullptr) return nullptr;

    Token_t *token = nullptr;
    status = TokensElem(tokens, *index, &token);
    CATCH_ERR;

    int is_main = 0;

    if (KEYW != KEYW_MAIN1 && KEYW != KEYW_FUNC)
    {
        PRINT(KEYWORD_IS_INVALID);
        abort();
        return nullptr;
    }

    if (KEYW == KEYW_MAIN1)
    {
        GET_NEXT_TOKEN;
        if (KEYW != KEYW_MAIN2)
        {
            PRINT(WHERE_IS_MAIN2);
            return nullptr;
        }
        is_main = 1;
        //KEYW == KEYW_MAIN2
    }
    // else
    // KEYW == KEYW_FUNC
    
    GET_NEXT_TOKEN;

    Node_t *def    = nullptr;
    status = InitKeyword(&def,  KEYW_DEFINE);
    CATCH_ERR;

    Node_t *func   = nullptr;
    status = InitKeyword(&func, KEYW_FUNC);
    CATCH_ERR;

    Node_t *name = nullptr;
    if (is_main)
    {
        status = InitKeyword(&name, KEYW_MAIN1);
        CATCH_ERR;
    }
    else
    {
        name = GetIdentifier(tokens, index);
        CATCH_NULL(name);
    }

    Node_t *params = nullptr;
    if (!is_main)
    {
        params = GetFuncParams(tokens, index);
        CATCH_NULL(params);
    }

    Node_t *stmts = GetStmts(tokens, index);
    CATCH_NULL(stmts);

    status = ConnectNodes(func, name, L_CHILD);
    CATCH_ERR;

    if (!is_main)
    {
        status = ConnectNodes(func, params, R_CHILD);
        CATCH_ERR;
    }

    status = ConnectNodes(def,  func, L_CHILD);
    CATCH_ERR;

    status = ConnectNodes(def, stmts, R_CHILD);
    CATCH_ERR;

    // TDUMP(def);
    
    return def;
}

Node_t *GetFuncParams(Tokens_t *tokens, size_t *index)
{
    int status = TokensVerify(tokens);
    CATCH_ERR;
    if (index == nullptr) return nullptr;

    if (Require(tokens, index, KEYW_OPRND)) return nullptr;

    // TODO: many args
    // NOW : only 1 arg
    Node_t *var = GetVar(tokens, index);
    CATCH_NULL(var);

    if (Require(tokens, index, KEYW_CLRND)) return nullptr;

    Node_t *param = nullptr;
    status = InitKeyword(&param, KEYW_PARAM);
    CATCH_ERR;

    status = ConnectNodes(param, var, R_CHILD);
    CATCH_ERR;

    return param;
}

Node_t *GetCallParams(Tokens_t *tokens, size_t *index)
{
    int status = TokensVerify(tokens);
    CATCH_ERR;
    if (index == nullptr) return nullptr;

    if (Require(tokens, index, KEYW_OPRND)) return nullptr;

    // TODO: many args
    //  NOW: only 1 arg
    Node_t *expr  = GetE(tokens, index);
    CATCH_NULL(expr);

    /*
    if (Require(tokens, index, KEYW_COMMA))
    {
        (*index)--;
        if (Require(tokens, index, KEYW_CLRND))
        {
            PRINT(INVALID_SYNTAX);
            return nullptr;
        }
        // continue normal processing
    }
    */

    if (Require(tokens, index, KEYW_CLRND)) return nullptr;

    Node_t *param = nullptr;
    status = InitKeyword(&param, KEYW_PARAM);
    CATCH_ERR;

    status = ConnectNodes(param, expr, R_CHILD);
    CATCH_ERR;

    return param;
}

Node_t *GetIdentifier(Tokens_t *tokens, size_t *index)
{
    int status = TokensVerify(tokens);
    CATCH_ERR;
    if (index == nullptr) return nullptr;

    Token_t *token = nullptr;
    status = TokensElem(tokens, *index, &token);
    CATCH_ERR;

    if (token->type != ID_TYPE)
    {
        PRINT(TOKEN_IS_NOT_IDENTIFIER_THOUGH);
        PRINT(PROGRAMMER_FREE_YOURSELF);
        PrintToken(tokens, *index);
        return nullptr;
    }

    return GetToken(tokens, index);
}

Node_t *GetW(Tokens_t *tokens, size_t *index)
{
    int status = TokensVerify(tokens);
    CATCH_ERR;
    if (index == nullptr) return nullptr;
  
    Node_t *val = GetP(tokens, index);
    assert(val != nullptr);

    Token_t *token = nullptr;
    status = TokensElem(tokens, *index, &token);
    CATCH_ERR;

    if (TOKEN_KEYW(KEYW_POW))
    {
        (*index)++;

        Node_t *val2 = GetW(tokens, index);
        assert(val2 != nullptr);

        Node_t *node = nullptr;
        status = InitKeyword(&node, KEYW_POW);
        CATCH_ERR;

        status = ConnectNodes(node,  val, L_CHILD);
        CATCH_ERR;
    
        status = ConnectNodes(node, val2, R_CHILD);
        CATCH_ERR;

        val = node;

        status = TokensElem(tokens, *index, &token);
        CATCH_ERR;
    }
    
    return val;
}

Node_t *GetT(Tokens_t *tokens, size_t *index)
{
    int status = TokensVerify(tokens);
    CATCH_ERR;
    if (index == nullptr) return nullptr;

    Node_t *val = GetW(tokens, index);
    assert(val != nullptr);

    Token_t *token = nullptr;
    status = TokensElem(tokens, *index, &token);
    CATCH_ERR;

    while (TOKEN_KEYW(KEYW_MUL) || TOKEN_KEYW(KEYW_DIV))
    {
        int op = token->arg.key_w;
        (*index)++;

        Node_t *val2 = GetW(tokens, index);
        assert(val2 != nullptr);

        Node_t *node = nullptr;
        status = InitKeyword(&node, op);
        CATCH_ERR;

        status = ConnectNodes(node,  val, L_CHILD);
        CATCH_ERR;
    
        status = ConnectNodes(node, val2, R_CHILD);
        CATCH_ERR;
        
        val = node;

        status = TokensElem(tokens, *index, &token);
        CATCH_ERR;
    }

    return val;
}

Node_t *GetE(Tokens_t *tokens, size_t *index)
{
    int status = TokensVerify(tokens);
    CATCH_ERR;
    if (index == nullptr) return nullptr;

    Node_t *val = GetT(tokens, index);
    assert(val != nullptr);

    Token_t *token = nullptr;
    status = TokensElem(tokens, *index, &token);
    CATCH_ERR;   

    while (TOKEN_KEYW(KEYW_ADD) || TOKEN_KEYW(KEYW_SUB))
    {
        int op = token->arg.key_w;
        (*index)++;

        Node_t *val2 = GetT(tokens, index);
        assert(val2 != nullptr);

        Node_t *node = nullptr;
        status = InitKeyword(&node, op);
        CATCH_ERR;

        status = ConnectNodes(node,  val, L_CHILD);
        CATCH_ERR;
    
        status = ConnectNodes(node, val2, R_CHILD);
        CATCH_ERR;
        
        val = node;

        status = TokensElem(tokens, *index, &token);
        CATCH_ERR;
    }

    return val;
}

Node_t *GetN(Tokens_t *tokens, size_t *index)
{
    int status = TokensVerify(tokens);
    CATCH_ERR;
    if (index == nullptr) return nullptr;

    Token_t *token = nullptr;
    status = TokensElem(tokens, *index, &token);
    CATCH_ERR;

    if (token->type != NUM_TYPE)
    {
        PRINT(WRONG_GETN);
        return nullptr;
    }
    
    Node_t *node = nullptr;
    status = InitEmptyNode(&node);
    CATCH_ERR;

    memcpy(node->value, token, sizeof(Token_t));

    (*index)++;

    return node;
}

Node_t *GetP(Tokens_t *tokens, size_t *index)
{
    int status = TokensVerify(tokens);
    CATCH_ERR;
    if (index == nullptr) return nullptr;

    Token_t *token = nullptr;
    status = TokensElem(tokens, *index, &token);
    CATCH_ERR;

    int multiplier = 1;

    while (TOKEN_KEYW(KEYW_ADD) || TOKEN_KEYW(KEYW_SUB))
    {
        if (TOKEN_KEYW(KEYW_SUB))
        {
            multiplier *= -1;
        }
        GET_NEXT_TOKEN;
    }
    
    if (multiplier == -1)
    {
        Node_t *node = nullptr;
        status = InitKeyword(&node, KEYW_SUB);
        CATCH_ERR;
        
        Node_t *zero = nullptr;
        status = InitEmptyNode(&zero);
        CATCH_ERR;

        zero->value->type    = NUM_TYPE;
        zero->value->arg.num = 0;

        status = ConnectNodes(node,  zero, L_CHILD);
        CATCH_ERR;

        Node_t *right = GetP(tokens, index);
        CATCH_NULL(right);

        status = ConnectNodes(node, right, R_CHILD);
        CATCH_ERR;

        return node;
    }

    if (TOKEN_KEYW(KEYW_OPRND))
    {   
        (*index)++;

        Node_t *val  = GetE(tokens, index);
        assert( val != nullptr);

        if (Require(tokens, index, KEYW_CLRND)) return nullptr;

        PRINT(EXIT_GETP);

        return val;
    }
    else
        return GetF(tokens, index);
}

Node_t *GetV(Tokens_t *tokens, size_t *index)
{
    int status = TokensVerify(tokens);
    CATCH_ERR;
    if (index == nullptr) return nullptr;

    Token_t *token = nullptr;
    status = TokensElem(tokens, *index, &token);
    CATCH_ERR;

    if (token->type == ID_TYPE)
    {
        return GetVar(tokens, index);
    }
    else
    {
        return GetN(tokens, index);
    }
}

Node_t *GetF(Tokens_t *tokens, size_t *index)
{
    int status = TokensVerify(tokens);
    CATCH_ERR;
    if (index == nullptr) return nullptr;

    Token_t *token = nullptr;
    status = TokensElem(tokens, *index, &token);
    CATCH_ERR;

    if (token->type == ID_TYPE)
    {
        GET_NEXT_TOKEN;
        if (!TOKEN_KEYW(KEYW_OPRND))
        {
            (*index)--;
            return GetV(tokens, index);
        }

        (*index)--;
        Node_t *name   = GetIdentifier(tokens, index);
        CATCH_NULL(name);

        Node_t *params = GetCallParams(tokens, index);
        CATCH_NULL(params);

        Node_t *call   = nullptr;
        status = InitKeyword(&call, KEYW_CALL);
        CATCH_ERR;

        status = ConnectNodes(call,   name, L_CHILD);
        CATCH_ERR;

        status = ConnectNodes(call, params, R_CHILD);
        CATCH_ERR;

        return call;
    }
    else
    {
        return GetN(tokens, index);
    }
}

int PrintSpaces(FILE *stream, const size_t n_spaces)
{
    if (stream == nullptr) return ASTree::STREAM_IS_NULL;
    
    for (size_t j = 0; j < n_spaces; ++j) fprintf(stream, " ");

    return ASTree::FUNC_IS_OK;
}

int PoopNodes(const Node_t *node, FILE *stream /*, size_t *level */)
{
    if (stream == nullptr) return ASTree::STREAM_IS_NULL;
    int status = NodeVerify(node);
    if (status)            return status;

    fprintf(stream, "{");

    Token_t *token = node->value;

    if (token == nullptr)
    {
        PRINT(CRINGE);
        return ASTree::TOKEN_IS_NULL;
    }

    // TODO: fix this cringe...
    // if (TOKEN_KEYW(KEYW_STMT))
    // {
    //     fprintf(stderr, "Keyw = %x\n", token->arg.key_w);
    //     fprintf(stream, "\n");
    //     status = PrintSpaces(stream, 2 * (*level));
    //     if (status) return status;
    //     (*level)++;
    // }

    if (node->left ) status |= PoopNodes(node->left,  stream /*, level */);

    if (token->type == ID_TYPE)
    {
        fprintf(stream, "\'%s\'", token->arg.id);
    }
    else
    if (token->type == NUM_TYPE)
    {
        fprintf(stream, "%lg", token->arg.num);
    }
    else
    if (token->type == KEYW_TYPE)
    {
        #define DEF_KEYW(DEF, CODE, WORD, FMT) \
            case KEYW_ ## DEF:                  \
                fprintf(stream, "%s", #FMT);     \
                break;

        #define DEF_OPER(DEF, CODE, SIGN)           \
            case KEYW_ ## DEF:                       \
                fprintf(stream, "%s", #SIGN);         \
                break;

        #define DEF_HELP(DEF, CODE, HELP)               

        switch (token->arg.key_w)
        {
            #include "../keywords"
            #include "../operators"
        
            default:
                fprintf(stderr, "INVALID_KEYWORD\n");
                fprintf(stream, "DEADDEADDEAD\n");
                break;
        }

        #undef DEF_KEYW
        #undef DEF_OPER
        #undef DEF_HELP
    }
    else
    {
        PRINT(DEAD_TOKEN_TYPE);
        return ASTree::DEAD_TOKEN_TYPE;
    }

    if (node->right) status |= PoopNodes(node->right, stream/*, level */);

    // if (TOKEN_KEYW(KEYW_STMT))
    // {
    //     (*level)--;
    // }

    fprintf(stream, "}");
    
    return status;
}

int PoopTree(const char *filename, Tree_t *tree)
{
    if (filename == nullptr) return ASTree::PTR_IS_NULL;
    int status = TreeVerify(tree);
    if (status) return status;

    FILE *stream = nullptr;
    status = OpenFile(filename, &stream, "w");
    if (status) return status;

                                     // size_t level = 0;
    status = PoopNodes(tree->root, stream /*, &level */);
    if (status) return status;

    fprintf(stream, "\n");

    fclose(stream);

    return status;
}

int PrintTokens(Tokens_t *tokens)
{
    int status = TokensVerify(tokens);
    if (status) return status;

    for (size_t index = 0; index < tokens->size; index++)
        PrintToken(tokens, index);
    
    return status;
}

int PrintToken(Tokens_t *tokens, size_t index)
{
    int status = TokensVerify(tokens);
    if (status) return status;

    Token_t *token = nullptr;

    status = TokensElem(tokens, index, &token);
    if (status) return status;

    fprintf(stderr, "%lu: ", index);

    if (token == nullptr)
    {
        fprintf(stderr, "token is empty(nullptr)\n");
    }
    else
    if (token->type == ID_TYPE)
    {
        if (token->arg.id == nullptr)
            fprintf(stderr, "dead\n");
        else
            fprintf(stderr, "\'%s\'\n", token->arg.id);
    }
    else
    if (token->type == NUM_TYPE)
    {
        fprintf(stderr, "%lg\n", token->arg.num);
    }
    else
    if (token->type == KEYW_TYPE)
    {
        #define DEF_KEYW(DEF, CODE, WORD, FMT)  \
            case KEYW_ ## DEF:                   \
                fprintf(stderr, "%s\n", #DEF);    \
                break;

        #define DEF_OPER(DEF, CODE, SIGN)            \
            case KEYW_ ## DEF:                        \
                fprintf(stderr, "%s\n", #SIGN);        \
                break;

        #define DEF_HELP(DEF, CODE, HELP)                 \
            case KEYW_ ## DEF:                             \
                fprintf(stderr, "%c\n", HELP);              \
                break;
        
        switch (token->arg.key_w)
        {
            #include "../keywords"
            #include "../operators"

            default:
                fprintf(stderr, "dead\n");
                break;
        }

        #undef DEF_KEYW
        #undef DEF_OPER
        #undef DEF_HELP
    }
}
