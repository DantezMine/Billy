#ifndef H_LEXER
#define H_LEXER

#include <stddef.h>
#include "pcre2posix.h"

#define MAX_TOKEN_SIZE 32


typedef enum Token_Type {
    WORD,
    REG,
    IMM,
    COMMA,
    COMMENT,
    NEWLINE,
    DOT,
    COLON,
    END,
    INVALID,
} Token_Type;

typedef struct Token {
    enum Token_Type type;
    char name[MAX_TOKEN_SIZE];
    int bytes_consumed;
} Token;

typedef struct Lexer_Pattern {
    Token_Type token_type;
    regex_t regexp;
    char pattern[100];
} Lexer_Pattern;

typedef struct Lexer_Iterator {
    char* pos;
    int line_number;
    int num_patterns;
    // Lexer_Pattern patterns[];
} Lexer_Iterator;

Token Lexer_next(Lexer_Iterator* it);

Token Lexer_peek(Lexer_Iterator* it);

void Lexer_init(Lexer_Iterator* it);


#endif
