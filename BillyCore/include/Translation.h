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
    size_t num_instr;
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


ByteCode Translation_translate(char* assm);
Token* Translation_tokenize(char* line);
#endif
