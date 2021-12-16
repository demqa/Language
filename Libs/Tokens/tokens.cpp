#include "tokens.h"

int TokensCtor    (Tokens_t *tokens)
{
    if (TokensAreClear(tokens) != Tokens::TOKENS_ARE_CLEAR) return Tokens::TOKENS_CANT_CONSTRUCT;

    tokens->size     = 0;
    tokens->capacity = Tokens::INITIAL_CAPACITY;

    tokens->token    = (Token_t *) calloc(tokens->capacity, sizeof(Token_t));
    if (tokens->token == nullptr) return Tokens::BAD_ALLOC;

    return Tokens::FUNC_IS_OK;
}

int TokensVerify  (Tokens_t *tokens)
{
    if (tokens == nullptr) return Tokens::TOKENS_ARE_NULL;
    
    if (TokensAreClear(tokens) == Tokens::TOKENS_ARE_CLEAR) return Tokens::TOKENS_ARE_CLEAR;

    if (tokens->size > tokens->capacity) return Tokens::SIZE_GREATER_CTY;

    if (tokens->token == nullptr)        return Tokens::DATA_IS_NULL;

    return Tokens::FUNC_IS_OK;
}

int TokensAreClear(Tokens_t *tokens)
{
    if (tokens == nullptr)     return Tokens::TOKENS_ARE_NULL;

    if (tokens->token    == nullptr   &&
        tokens->size     == 0         &&
        tokens->capacity == 0) return Tokens::TOKENS_ARE_CLEAR;

    return Tokens::FUNC_IS_OK;
}

int TokensDtor    (Tokens_t *tokens)
{
    int status = TokensVerify(tokens);
    if (status) return status;

    tokens->capacity = 0;
    tokens->size     = 0;

    free(tokens->token);
    tokens->token    = nullptr;

    return TokensAreClear(tokens);
}

int TokensPush    (Tokens_t *tokens, const Token_t *token)
{
    int status = TokensVerify(tokens);
    if (status) return status;

    if (token == nullptr) return Tokens::PUSHABLE_TOKEN_NULL;

    if (tokens->size == tokens->capacity)
    {
        status = TokensResize(tokens);
        if (status) return status;

        assert(tokens->size < tokens->capacity);
    }

    memcpy(tokens->token + tokens->size++, token, sizeof(Token_t));

    return status;
}

int TokensElem    (Tokens_t *tokens, size_t index, Token_t **token)
{
    int status = TokensVerify(tokens);
    if (status) return status;

    if ( token == nullptr)    return Tokens::TOKEN_IS_NULL;
    if (*token != nullptr)    return Tokens::TOKEN_DEST_ISNT_NULL;

    if (index > tokens->size) return Tokens::UNAVAILABLE_ELEM;

    *token = tokens->token + index;

    return status;
}

int TokensResize  (Tokens_t *tokens)
{
    int status = TokensVerify(tokens);
    if (status) return status;

    assert(Tokens::CAPACITY_MULTIPLIER > 1);

    size_t new_capacity = Tokens::INITIAL_CAPACITY;

    if (tokens->size <= tokens->capacity / (int) pow(Tokens::CAPACITY_MULTIPLIER, 2))
    {
        new_capacity = tokens->capacity / Tokens::CAPACITY_MULTIPLIER;
    }
    else
    if (tokens->size == tokens->capacity)
    {
        new_capacity = tokens->capacity * Tokens::CAPACITY_MULTIPLIER;
    }
    else
        return Tokens::DONT_NEED_RESIZE;

    #define max(a,b) ((a) > (b)) ? (a) : (b)
    new_capacity = max(new_capacity, Tokens::INITIAL_CAPACITY);
    #undef max

    if (new_capacity == tokens->capacity) return Tokens::MIN_CTY_REACHED;

    Token_t *token = (Token_t *) realloc(tokens->token, sizeof(Token_t) * new_capacity);
    if (token == nullptr) return Tokens::REALLOC_ERROR;

    tokens->token = token;

    if (new_capacity > tokens->capacity)
    {
        memset(tokens->token + tokens->capacity, 0, sizeof(Token_t) * (new_capacity - tokens->capacity));
        //  TODO: check this... i'm not sure in (new_capacity - tokens->capacity);
        // ... lol i'm clown ...
    }

    tokens->capacity = new_capacity;

    return Tokens::FUNC_IS_OK;
}
