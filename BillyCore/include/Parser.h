#ifndef H_PARSER
#define H_PARSER

#include "Lexer.h"
#include "Component.h"
#include "CPU.h"


typedef struct ByteCode {
    int num_instr;
    int instr_size;
    Instruction* instr;
} ByteCode;


typedef struct Label {
    int value;
    char name[MAX_TOKEN_SIZE];
} Label;


ByteCode Parser_translate(char* src);

ByteCode Parser_translate_from_file(char* path);

void Parser_free(ByteCode* bc);

#endif
