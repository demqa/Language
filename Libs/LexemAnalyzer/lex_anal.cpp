#include "lex_anal.h"

int Filesize  (FILE  *stream, size_t *filesize)  
{
    if (stream == nullptr) return Text::STREAM_IS_NULL;

    struct stat buff = {};
        
    fstat(fileno(stream), &buff);
    
    *filesize = buff.st_size;

    return 0;
}

int ReadBuffer(char **buffer, FILE   *stream)
{
    if ( stream == nullptr) return Text::STREAM_IS_NULL;
    if ( buffer == nullptr) return Text::BUFFER_IS_NULL;
    if (*buffer != nullptr) return Text::PTR_IS_NOT_NULL;

    size_t filesize = 0;

    int status = Filesize(stream, &filesize);
    if (status)
    {
        fclose(stream);
        return status;
    }

    *buffer = (char *) calloc(filesize + 1, sizeof(char));
    if (buffer == nullptr)
    {
        fclose(stream);
        return Text::BAD_ALLOC;
    }

    size_t count = fread(*buffer, sizeof(char), filesize, stream);
    if (count != filesize)
    {
        fclose(stream);
        return Text::READ_WAS_UNSUCCESSFUL;
    }

    (*buffer)[filesize] = '\0';

    fclose(stream);

    return 0;
}

int OpenFile  (const char *filename, FILE **dest_stream)
{
    if ( filename    == nullptr) return Text::FILENAME_IS_NULL;
    if ( dest_stream == nullptr) return Text::STREAM_IS_NULL;
    if (*dest_stream != nullptr) return Text::DEST_PTR_IS_NULL;

    FILE *stream = fopen(filename, "r");
    if (stream == nullptr)       return Text::CANT_OPEN_FILE;

    *dest_stream = stream;

    return Text::FUNC_IS_OK;
}

int SkipSpaces(char **ptr)
{
    if (ptr == nullptr || *ptr == nullptr) return Tokens::PTR_IS_NULL;

    while (isspace(**ptr)) (*ptr)++;

    if (**ptr == '\0') return Tokens::END_OF_FILE;

    return Tokens::FUNC_IS_OK;
}

int StrEqual(const char *l, const char *r)
{
    if (l == nullptr || r == nullptr) return 0;

    int i = 0;
    for (i = 0; l[i] != '\0' && r[i] != '\0'; i++)
        if (l[i] != r[i]) return 0;

    return (l[i] == '\0' && isspace(r[i]) ||
            r[i] == '\0' && isspace(l[i]) ||
            l[i] == '\0' && r[i] == '\0' ||
            isspace(l[i]) && isspace(r[i]));
}

int IsKeyword (char *word, int *keyword_code)
{
    if (keyword_code == nullptr || 
        word         == nullptr) return Tokens::PTR_IS_NULL;

#define DEF_KEYW(DEF, CODE, WORD)               \
    if (StrEqual(word, #WORD)) \
    {                                             \
        *keyword_code = CODE;                      \
    }                                               \
    else

    #include "../keywords"
/*  else */
    {
        *keyword_code = 0;
        return Tokens::FUNC_IS_OK;
    }

#undef DEF_KEYW

    return Tokens::FUNC_IS_OK;
}

int GetWord   (char **ptr, char *word)
{
    if (ptr == nullptr || *ptr == nullptr || word == nullptr) return Tokens::PTR_IS_NULL;

    int status = Tokens::FUNC_IS_OK;

    size_t size = 0;

    do
    {
        word[size++] = *((*ptr)++);
    }
    while ((**ptr >= 'a' && **ptr <= 'z'  ||
            **ptr >= 'A' && **ptr <= 'Z'  || 
            **ptr >= '0' && **ptr <= '9'  ||
            **ptr == '_' || **ptr == '$') && size != WORD_MAX_LEN);

    if (size == WORD_MAX_LEN && isspace(**ptr) != 0) return Tokens::IDENTIFIER_MAX_LEN_REACHED;
    
    return status;
}

int GetToken  (char **ptr, Tokens_t *tokens)
{
    int status = TokensVerify(tokens);
    if (status) return status;

    if (ptr == nullptr || *ptr == nullptr) return Tokens::PTR_IS_NULL;

    status = SkipSpaces(ptr);
    if (status) return status;

    Token_t token = {};

    assert(**ptr != '\0');

    if (isalpha(**ptr) || **ptr == '$')
    {
        char *word = (char *) calloc(WORD_MAX_LEN + 1, sizeof(char));
        if (word == nullptr) return Tokens::BAD_ALLOC;

        int length = 0;

        status = GetWord(ptr, word);
        if (status) return status;

        int keyword_code = 0;
        status = IsKeyword(word, &keyword_code);
        if (status) return status;

        if (keyword_code)
        {
            // keyword
            token.type      = KEYW_TYPE;
            token.arg.key_w = keyword_code;
        }
        else
        {
            // identifier
            token.type      = ID_TYPE;
            memcpy(token.arg.id, word, WORD_MAX_LEN * sizeof(char));
        }

        free(word);
    }
    else
    if (isalnum(**ptr))
    {
        token.type = NUM_TYPE;

        double num = NAN;
        int length = 0;

        sscanf(*ptr, "%lf%n", &num, &length);
        if (length == 0)   return Tokens::SSCANF_CANT_SCAN;
        if (num    == NAN) return Tokens::SSCANF_BAD_ALLOC;
        *ptr += length;

        token.arg.num = num;
    }
    else
    {
        token.type      = KEYW_TYPE;

        // operator
        #define DEF_OPER(DEF, CODE, SIGN)                \
            if (StrEqual(*ptr, #SIGN))                    \
            {                                              \
                sign = CODE;                                \
                *ptr += strlen(#SIGN);                       \
            }                                                 \
            else

        #define DEF_HELP(DEF, CODE, HELP)                        \
            if (**ptr == HELP)                                    \
            {                                                      \
                sign = CODE;                                        \
                *ptr += 1;                                           \
            }                                                         \
            else

        int sign = 0;

        #include "../operators"
     /* else */
        {
            sign = 0;
        }

        if (sign == 0)
        {
            fprintf(stderr, "%s", *ptr);
            return Tokens::UNEXPECTED_LEXEM;
        }
        
        #undef DEF_OPER
        #undef DEF_HELP

        token.arg.key_w = sign;
    }
    
    status = TokensPush(tokens, &token);
    if (status) return status;

    return Tokens::FUNC_IS_OK;
}

int GetTokens (const char *filename, Tokens_t *tokens)
{
    if (filename  == nullptr) return Tokens::FILENAME_IS_NULL;
    if (tokens    == nullptr) return Tokens::TOKENS_ARE_NULL;

    int status = TokensVerify(tokens);
    if (status) return status;

    FILE *stream = nullptr;

    status = OpenFile(filename, &stream);
    if (status) return status;
    
    char *buffer = nullptr;
    status = ReadBuffer(&buffer, stream);
    // PRINT_X(status);
    if (status) return status;
    
    char *ptr = buffer;

    do
    {
        status = GetToken(&ptr, tokens);
        // PRINT_X(status);
    }
    while (status == Tokens::FUNC_IS_OK);

    if (status == Tokens::END_OF_FILE)
    {
        Token_t token   = {};
        token.type      = KEYW_TYPE;
        token.arg.key_w = KEYW_EOF;
        TokensPush(tokens, &token);
    }
    
    free(buffer);

    return status;
}
