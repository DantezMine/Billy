#ifndef TRANSLATION_H
#define TRANSLATION_H

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Component.h"

#define MAX_TOKEN_SIZE 32

typedef struct ByteCode {
    int num_instr;
    uint16_t* instr;
} ByteCode;


typedef struct Label {
    size_t value;
    char name[MAX_TOKEN_SIZE];
} Label;

enum Token_Type {
    LABEL,
    INSTR,
    REG,
    IMM,
    END,
    INVALID,
};

typedef struct Token {
    enum Token_Type type;
    union {
        char name[MAX_TOKEN_SIZE];
        int immediate;
        int reg;
    };
} Token;


ByteCode Translation_translate(char* path);
Token* Translation_tokenize(char* line);
void Translation_token_to_str(Token* token, char* out);
int Translation_token_cmpeq(Token* token, Token* other);
ByteCode Translation_translate_str(char* source);
#endif
