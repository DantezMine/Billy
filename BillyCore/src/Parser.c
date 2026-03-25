#include "Parser.h"
#include "CPU.h"
#include "Component.h"
#include "Lexer.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ERROR_INSTR(...) { printf(__VA_ARGS__); return 0; }
#define ERROR_TRANS(...) { printf(__VA_ARGS__); bc_out.instr_size = -1; free(bc_out.instr); free(labels); free(label_refs); return bc_out; }


static void* grow_array(void* arr, int* size, size_t elem_size);
static char* src_to_str(char* path);
static uint16_t swap_endian(uint16_t in);
static void append_ref(Label** label_refs, int* label_refs_size, int* label_refs_indx, char* name, int instr_num);

static int find_label(const char* name, Label* labels, int num_labels);
static int parse_reg(const char* reg_name);
static int parse_imm(const char* imm_name);
static uint16_t parse_instr(Token* start_token, Lexer_Iterator* iter, int line_num, int instr_num,
        Label** label_refs, int* label_refs_size, int* label_refs_indx);

static const char* register_labels[] = {
    "r00",
    "rax",
    "rbx",
    "rcx",
    "rdx",
    "rex",
    "rsp",
    "rbp",
};


ByteCode Parser_translate(char* src) {
    ByteCode bc_out = (ByteCode) {.instr = calloc(sizeof(Instruction),5), .num_instr = -1, .instr_size=5};
    
    Lexer_Iterator iter = (Lexer_Iterator) {.pos = src};
    Lexer_init(&iter);

    int labels_size = 5;
    int label_indx = -1; // points to last
    Label* labels = calloc(sizeof(Label),labels_size);

    int label_refs_size = 5;
    int label_refs_indx = -1; // points to last
    Label* label_refs = calloc(sizeof(Label),label_refs_size);

    int line_num = 0;

    Token start_token;
    while ((start_token = Lexer_next(&iter)).type != END) {
        for (int i=0; i<bc_out.instr_size; i++) {
            // printf("%d: %d\n",i,bc_out.instr[i].instr);
        }
        switch(start_token.type) {
            case WORD:
            {
                uint16_t instr = parse_instr(&start_token, &iter, line_num, bc_out.num_instr,
                        &label_refs, &label_refs_size, &label_refs_indx);
                if (instr == 0)
                    ERROR_TRANS("PARSER::TRANSLATE::Bad instruction at line %d\n", line_num);
                if (bc_out.num_instr == bc_out.instr_size-1) {
                    bc_out.instr = grow_array(bc_out.instr, &bc_out.instr_size, sizeof(Instruction));
                }
                bc_out.instr[++bc_out.num_instr].instr = swap_endian(instr);
                break;
            }
            case COMMENT:
                break;
            case NEWLINE:
                line_num++;
                break;
            case DOT: 
                {
                    Token tok_label;
                    if ((tok_label = Lexer_next(&iter)).type != WORD)
                        ERROR_TRANS("PARSER::TRANSLATE::Expected identifier after strt of label declaration\n");
                    if (Lexer_next(&iter).type != COLON)
                        ERROR_TRANS("PARSER::TRANSLATE::Expected ':' at end of label declaration\n");
                    if (find_label(tok_label.name, labels, label_indx+1) != -1) {
                        ERROR_TRANS("PARSER::TRANSLATE::Redeclaration of label \"%s\" at line %d\n", tok_label.name, line_num);
                    }

                    if (label_indx == labels_size-1)
                        labels = grow_array(labels, &labels_size,sizeof(Label));

                    labels[++label_indx] = (Label) {.value = bc_out.num_instr+1};
                    strncpy(labels[label_indx].name, tok_label.name, MAX_TOKEN_SIZE);
                    break;
                }
            case INVALID:
                ERROR_TRANS("PARSER::TRANSLATE::Encountered invalid token at line %d\n",line_num);
                break;
            default:
                ERROR_TRANS("PARSER::TRANSLATE::Encountered illegal token at line %d\n",line_num);
                break;
        }
    }

    // Patch all label references
    for (int i=0; i<label_refs_indx+1; i++) {
        int indx = find_label(label_refs[i].name,labels, label_indx+1);
        if (indx == -1)
            ERROR_TRANS("PARSER::TRANSLATE::Encountered undefined label reference \"%s\"\n",label_refs[i].name);
        bc_out.instr[label_refs[i].value].high |= ((labels[indx].value * 2) & 0xff);
    }

    free(labels);
    free(label_refs);

    return bc_out;
}


ByteCode Parser_translate_from_file(char* path) {
    char* src_str = src_to_str(path);
    ByteCode result = Parser_translate(src_str);
    free(src_str);
    return result;
}

void Parser_free(ByteCode* bc) {
    // free(bc->instr);
}


static uint16_t parse_instr(Token* start_token, Lexer_Iterator* iter, int line_num, int instr_num,
        Label** label_refs, int* label_refs_size, int* label_refs_indx) {
    int opcode = -1;
    for (int i=0; i<16; i++) {
        if (strcmp(start_token->name,instr_table[i])==0) {
            opcode = i;
            break;
        }
    }
    if (opcode == -1) {
        ERROR_INSTR("PARSER::TRANSLATE::Non-existent instruction %s on line %d\n",start_token->name,line_num);
    }
    uint16_t instr = 0;
    switch(instr_type[opcode]) {
    case M_TYPE: {
        Token tok_reg_src; 
        Token tok_reg_dst;
        Token tok_imm;
        Token tok_temp;
        if ((tok_reg_dst = Lexer_next(iter)).type != REG)
            ERROR_INSTR("PARSER::TRANSLATE::Expected register, but got \"%s\", on line %d\n",tok_reg_dst.name,line_num);
        if ((tok_temp = Lexer_next(iter)).type != COMMA)
            ERROR_INSTR("PARSER::TRANSLATE::Expected ',', but got \"%s\", on line %d\n",tok_temp.name,line_num);
        if ((tok_reg_src = Lexer_next(iter)).type != REG)
            ERROR_INSTR("PARSER::TRANSLATE::Expected register, but got \"%s\", on line %d\n",tok_reg_src.name,line_num);
        if ((tok_temp = Lexer_next(iter)).type != COMMA)
            ERROR_INSTR("PARSER::TRANSLATE::Expected ',', but got \"%s\", on line %d\n",tok_temp.name,line_num);
        if ((tok_imm = Lexer_next(iter)).type != IMM)
            ERROR_INSTR("PARSER::TRANSLATE::Expected immediate, but got \"%s\", on line %d\n",tok_imm.name,line_num);
        int reg_dst = parse_reg(tok_reg_dst.name);
        int reg_src = parse_reg(tok_reg_src.name);
        int imm = parse_imm(tok_imm.name);
        if (reg_dst == -1)
            ERROR_INSTR("PARSER::TRANSLATE::Register label \"%s\" doesn't match possible registers.\n",tok_reg_dst.name);
        if (reg_src == -1)
            ERROR_INSTR("PARSER::TRANSLATE::Register label \"%s\" doesn't match possible registers.\n",tok_reg_src.name);

        instr |= (opcode & 0xf) << 12;
        instr |= (reg_dst & 0x7) << 9;
        instr |= (reg_src & 0x7) << 6;
        instr |= imm & 0x3f;
        break;
        }
    case I_TYPE: {
        Token tok_reg_dst;
        Token tok_imm;
        Token tok_temp;
        tok_reg_dst = Lexer_next(iter);
        if (tok_reg_dst.type == WORD)
        {
            append_ref(label_refs,label_refs_size,label_refs_indx,tok_reg_dst.name,instr_num);
            tok_imm.type = IMM;
            strcpy(tok_imm.name, "0");
            tok_reg_dst = (Token) {.type = REG, .name = "r00"};
        }
        else if (tok_reg_dst.type == IMM)
        {
            tok_imm.type = tok_reg_dst.type;
            strncpy(tok_imm.name, tok_reg_dst.name, MAX_TOKEN_SIZE);
            tok_reg_dst = (Token) {.type = REG, .name = "r00"};
        }
        else if (tok_reg_dst.type == REG)
        {
            if ((tok_temp = Lexer_next(iter)).type == COMMA) {
                tok_imm = Lexer_next(iter);

                if (tok_imm.type == WORD) {
                    append_ref(label_refs,label_refs_size,label_refs_indx,tok_imm.name,instr_num);
                    tok_imm.type = IMM;
                    strcpy(tok_imm.name, "0");
                }
                else if (tok_imm.type != IMM) {
                    ERROR_INSTR("PARSER::TRANSLATE::Expected immediate or label, but got \"%s\", on line %d\n",tok_imm.name,line_num);
                }
            }
        } else {
            ERROR_INSTR("PARSER::TRANSLATE::Expected register, immediate or label, but got \"%s\", on line %d\n",
                    tok_reg_dst.name,line_num);
        }
        int reg_dst = parse_reg(tok_reg_dst.name);
        int imm = parse_imm(tok_imm.name);
        if (reg_dst == -1)
            ERROR_INSTR("PARSER::TRANSLATE::Register label \"%s\" doesn't match possible registers.\n",tok_reg_dst.name);
        instr |= (opcode & 0xf) << 12;
        instr |= (reg_dst & 0x7) << 9;
        instr |= imm & 0xff;
        break;
        }
    case R_TYPE: {
        Token tok_reg_dst;
        Token tok_reg_src1; 
        Token tok_reg_src2; 
        Token tok_temp;
        if ((tok_reg_dst = Lexer_next(iter)).type != REG)
            ERROR_INSTR("PARSER::TRANSLATE::Expected register, but got \"%s\", on line %d\n",tok_reg_dst.name,line_num);
        if ((tok_temp = Lexer_next(iter)).type != COMMA)
            ERROR_INSTR("PARSER::TRANSLATE::Expected ',', but got \"%s\", on line %d\n",tok_temp.name,line_num);
        if ((tok_reg_src1 = Lexer_next(iter)).type != REG)
            ERROR_INSTR("PARSER::TRANSLATE::Expected register, but got \"%s\", on line %d\n",tok_reg_src1.name,line_num);
        // can accept only two registers, third set to zero
        if (Lexer_peek(iter).type == COMMA) {
            Lexer_next(iter);
            if ((tok_reg_src2 = Lexer_next(iter)).type != REG)
                ERROR_INSTR("PARSER::TRANSLATE::Expected register, but got \"%s\", on line %d\n",tok_reg_src2.name,line_num);
        } else {
            strcpy(tok_reg_src2.name, "r00");
        }
        int reg_dst = parse_reg(tok_reg_dst.name);
        int reg_src1 = parse_reg(tok_reg_src1.name);
        int reg_src2 = parse_reg(tok_reg_src2.name);
        if (reg_dst == -1)
            ERROR_INSTR("PARSER::TRANSLATE::Register label \"%s\" doesn't match possible registers.\n",tok_reg_dst.name);
        if (reg_src1 == -1)
            ERROR_INSTR("PARSER::TRANSLATE::Register label \"%s\" doesn't match possible registers.\n",tok_reg_src1.name);
        if (reg_src2 == -1)
            ERROR_INSTR("PARSER::TRANSLATE::Register label \"%s\" doesn't match possible registers.\n",tok_reg_src2.name);
        instr |= (opcode & 0xf) << 12;
        instr |= (reg_dst & 0x7) << 9;
        instr |= (reg_src1 & 0x7) << 6;
        instr |= (reg_src2 & 0x7) << 3;
        break;
        }
    default:
        instr = 0;
        break;
    }
    return instr;
}

static void* grow_array(void* arr, int* size, size_t elem_size) {
    *size *= 2;
    void* new_arr = realloc(arr, *size*elem_size);
    return new_arr;
}

static int parse_reg(const char* reg_name) {
    int reg = -1;
    for (int i=0; i<8; i++) {
        if (!strcmp(reg_name,register_labels[i])) {
            reg = i;
            break;
        }
    }
    return reg;
}

static int find_label(const char* name, Label* labels, int num_labels) {
    int label_indx = -1;
    for (int i=0; i<num_labels; i++) {
        if (strcmp(name,labels[i].name) == 0) {
            label_indx = i;
            break;
        }
    }
    return label_indx;
}

static int parse_imm(const char* imm_name) {
    if (imm_name[0] != '0') {
        return atoi(imm_name);
    } else if (imm_name[1] == 'x') {
        return strtol(imm_name, NULL, 16);
    } else if (imm_name[1] == 'b') {
        return strtol(imm_name, NULL, 2);
    }
    return 0;
}


static void append_ref(Label** label_refs, int* label_refs_size, int* label_refs_indx, char* name, int instr_num) {
    if (*label_refs_indx == (*label_refs_size)-1) {
        *label_refs = grow_array(*label_refs, label_refs_size, sizeof(Label));
    }

    (*label_refs)[++(*label_refs_indx)] = (Label) {.value = instr_num+1};

    strncpy((*label_refs)[*label_refs_indx].name, name, MAX_TOKEN_SIZE);
}


static uint16_t swap_endian(uint16_t in) {
    uint8_t low = in;
    uint8_t high = (in>>8);
    return (low<<8) | high;
}

static char* src_to_str(char* path) {
    FILE* file = fopen(path, "r");
    if (!file) {
        perror("SRC_TO_STR::Could not open file");
        return NULL;
    }

    // Move to the end to find the file size
    if (fseek(file, 0, SEEK_END) != 0) {
        perror("SRC_TO_STR::fseek failed");
        fclose(file);
        return NULL;
    }

    long length = ftell(file);
    if (length < 0) {
        perror("SRC_TO_STR::ftell failed");
        fclose(file);
        return NULL;
    }

    rewind(file); // Reset to the beginning

    // Allocate buffer (+1 for null terminator)
    char* buffer = (char*)malloc(length + 1);
    if (!buffer) {
        perror("SRC_TO_STR::Memory allocation failed");
        fclose(file);
        return NULL;
    }

    size_t read_size = fread(buffer, 1, length, file);
    buffer[read_size] = '\0'; // Null-terminate the string

    fclose(file);
    return buffer;
}
