#include "Translation.h"
#include "Component.h"
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>

static uint16_t tokens_to_instr(Token* tokens);
static Label* get_label(char* name);
static Token arg_to_token(const char* arg);
static uint16_t swap_endian(uint16_t in);


static Label* labels;
static int labels_indx;
static int labels_size;

// ByteCode.instr must be freed after use
ByteCode Translation_translate(char* path) {
    FILE* source = fopen(path,"r");

    labels = malloc(16*sizeof(Label));
    labels_indx = 0;
    labels_size = 16;

    int num_instr = 0;
    int line_count = 0;
    

    // Dynamic array
    Token* tokens = malloc(sizeof(Token)*100);
    int tokens_index = 0;
    int tokens_size = 0;
    char* line = malloc(512*sizeof(char));

    while (1) {
        if (fscanf(source,"%511[^\n]%*[\n]",line) == -1) break;
        if (*line == EOF || *line == '\0') break;


        // Get tokens from line string and add them to the tokens list
        Token* line_tokens = Translation_tokenize(line);
        if (line_tokens == NULL) continue;
        line_count++;
        int i = 0;
        while(line_tokens[i].type != END) {
            // resize tokens array if needed
            if (tokens_index == tokens_size-1) {
                tokens_size = floor(tokens_size*1.5);
                realloc(tokens, tokens_size);
            }
            tokens[tokens_index+i] = line_tokens[i];
            i++;
        }
        tokens[tokens_index+i] = line_tokens[i];
        i++;
        free(line_tokens);

        // count instructions and set label definitions
        Token* first_token = &tokens[tokens_index];
        tokens_index += i;
        switch(first_token->type) {
            case INSTR:
                num_instr++;
                break;
            case LABEL:
                // resize labels array if needed
                if (labels_indx == labels_size-1) {
                    labels_size = floor(labels_size*1.5);
                    realloc(labels, labels_size);
                }
                if (get_label(first_token->name) != NULL) {
                    printf("Multiple definitions of label %s\n",first_token->name);
                    return (ByteCode){-1,NULL};
                }
                labels[labels_indx++] = (Label){.value=2*num_instr};
                strcpy(labels[labels_indx-1].name,first_token->name);
                break;
            default:
                printf("Unexpected first token of type %d on line %d\n",first_token->type,line_count);
                return (ByteCode){-1,NULL};
                break;
        }
    }

    fclose(source);
    free(line);
    ByteCode bc_out = (ByteCode){num_instr, malloc(num_instr * sizeof(uint16_t))};


    // Iterate through instructions and convert to bytecode
    Token* current_token = tokens;
    for (int i=0; i<num_instr; i++) {
        // skip labels
        if (current_token[0].type == LABEL) {
            current_token++;
            continue;
        }
        // convert to bytecode
        bc_out.instr[i].instr = swap_endian(tokens_to_instr(current_token));
        // move current_token to next line
        int tokens_in_instr = 0; //includes the END token
        while(current_token[tokens_in_instr++].type != END);
        current_token += tokens_in_instr;
    }

    free(labels);
    free(tokens);
    return bc_out;
}


Token* Translation_tokenize(char* line) {
    Token* tokens = malloc(5*sizeof(Token));
    for (int i=0; i<5; i++) {
        tokens[i] = (Token){.type=END};
    }
    char label[MAX_TOKEN_SIZE+1];
    char instr[MAX_TOKEN_SIZE+1];
    char arg1[17];
    char arg2[17];
    char arg3[17];
    int n;
    if (sscanf(line," %[#]",label)) {
        free(tokens);
        return NULL;
    }
    else if (sscanf(line," %1[.]%32[a-zA-Z0-9]%1[:]",arg1,label,arg2) == 3) {
        if (isdigit(label[0])) {
            printf("Tokenizer::Illegal first character in label %s\n",label);
            return NULL;
        }
        tokens[0] = (Token){.type=LABEL};
        strcpy(tokens[0].name,label);
    } else if ((n = sscanf(line," %32s %16[^,] , %16[^,] , %16[^,]",instr,arg1,arg2,arg3))) {
        tokens[0] = (Token){.type=INSTR};
        strcpy(tokens[0].name,instr);
        switch (n) {
            case 4:
                tokens[3] = arg_to_token(arg3);
            case 3:
                tokens[2] = arg_to_token(arg2);
            case 2:
                tokens[1] = arg_to_token(arg1);
                break;
        }
    } else {
        printf("Tokenizer::Something went wrong\n");
        free(tokens);
        return NULL;
    }

    return tokens;
}


Token arg_to_token(const char* arg) {
    if (arg[0]=='%' && arg[1]=='r') {
        char* endptr;
        int reg = strtol(&arg[2],&endptr,10);
        if (*endptr != '\0') {
            printf("Tokenizer::Failed parsing register argument %s\n",arg);
        }
        return (Token){.type=REG,.reg=reg};
    } else if (isalpha(arg[0])){
        Token out = (Token){.type=LABEL};
        strcpy(out.name,arg);
        return out;
    } else if (isdigit(arg[0])) { 
        char* endptr;
        int imm = strtol(arg,&endptr,0);
        if (*endptr != '\0') {
            printf("Tokenizer::Failed parsing immediate argument %s\n",arg);
        }
        return (Token){.type=IMM,.immediate=imm};
    }
    printf("Tokenizer::Failed parsing argument %s\n",arg);
    return (Token){.type=INVALID};
};


static Label* get_label(char* name) {
    for (int i=0; i<labels_size; i++) {
        if (strcmp(labels[i].name,name) == 0) {
            return &labels[i];
        }
    }
    return NULL;
}



static uint16_t tokens_to_instr(Token* tokens) {
    uint16_t op = 0;
    for (;op<16; op++) {
        if (strcmp(instr_table[op],tokens[0].name) == 0)
            break;
    }
    if (op == 15) {
        printf("Translation::Failed to identify instruction\n");
    }
    uint16_t out = 0;
    switch(instr_type[op]) {
        case M_TYPE:
            if (tokens[1].type != REG || tokens[2].type != REG || tokens[3].type != IMM) {
                printf("Translation::Expected '%s REG, REG, IMM'\n",tokens[0].name);
                return -1;
            }
            out |= (op & 0xf)<<12;
            out |= (tokens[1].reg & 0x7)<<9;
            out |= (tokens[2].reg & 0x7)<<6;
            out |= tokens[3].immediate & 0x3f;
            break;
        case R_TYPE:
            if (tokens[1].type != REG || tokens[2].type != REG || tokens[3].type != REG) {
                printf("Translation::Expected '%s REG, REG, REG'\n",tokens[0].name);
                return -1;
            }
            out |= (op & 0xf)<<12;
            out |= (tokens[1].reg & 0x7)<<9;
            out |= (tokens[2].reg & 0x7)<<6;
            out |= (tokens[3].reg & 0x7)<<3;
            break;
        case I_TYPE:
            if (!(tokens[1].type == IMM || tokens[1].type == LABEL)
                    && !(tokens[1].type == REG && (tokens[2].type == IMM || tokens[2].type == LABEL))) {
                printf("Translation::Expected '%s REG, IMM' or '%s REG, LABEL\n",tokens[0].name,tokens[0].name);
                return -1;
            }
            out |= (op & 0xf)<<12;
            if (tokens[1].type == REG) {
                out |= (tokens[1].reg & 0x7)<<9;
                if (tokens[2].type == IMM) {
                    out |= tokens[2].immediate & 0xff;
                } else {
                    Label* label = get_label(tokens[2].name);
                    if (label == NULL) {
                        printf("Translation::Label %s not defined\n",tokens[2].name);
                        return -1;
                    }
                    out |= label->value & 0xff;
                }
            } else {
                if (tokens[1].type == IMM) {
                    out |= tokens[1].immediate & 0xff;
                } else {
                    Label* label = get_label(tokens[1].name);
                    if (label == NULL) {
                        printf("Translation::Label %s not defined\n",tokens[1].name);
                        return -1;
                    }
                    out |= label->value & 0xff;
                }

            }
            break;
        case NOP:
            break;
    }
    return out;
}


void Translation_token_to_str(Token* token, char* out) {
    char* type;
    char val[17];
    switch(token->type) {
        case LABEL:
            type = "LABEL";
            strcpy(val,token->name);
            break;
        case INSTR:
            type = "INSTR";
            strcpy(val,token->name);
            break;
        case REG:
            type = "REG";
            sprintf(val,"%d",token->reg);
            break;
        case IMM:
            type = "IMM";
            sprintf(val,"%d",token->immediate);
            break;
        case END:
            type = "END";
            strcpy(val,"__");
            break;
        case INVALID:
            type = "INVALID";
            strcpy(val,"__");
            break;
        default:
            type = "UNKNOWN";
            strcpy(val,"__");
            break;
    }
    sprintf(out,"Token: %s, %s\n",type,val);
}


int Translation_token_cmpeq(Token* token, Token* other) {
    return (token->type==LABEL && other->type==LABEL && !strcmp(token->name,other->name))
        || (token->type==INSTR && other->type==INSTR  && !strcmp(token->name,other->name))
        || (token->type==REG && other->type==REG  && token->reg==other->reg)
        || (token->type==IMM && other->type==IMM  && token->immediate==other->immediate)
        || (token->type==other->type);
}

static uint16_t swap_endian(uint16_t in) {
    uint8_t low = in;
    uint8_t high = (in>>8);
    return (low<<8) | high;
}


void Translation_instr_to_str(uint16_t instr, char* out, int width) {
    uint16_t op = (instr>>12) & 0xf;
    const char* mnemonic = instr_table[op];
    const char* regDest = register_labels[(instr>>9)&0x7];
    const char* reg1 = register_labels[(instr>>6)&0x7];
    const char* reg2 = register_labels[(instr>>3)&0x7];
    uint16_t immM = instr & 0x3f;
    uint16_t immI = instr & 0xff;
    switch (instr_type[op]) {
    case NOP:
        sprintf_s(out, width, "NOP");
        break;
    case R_TYPE:
        sprintf_s(out, width, "%s %%%s, %%%s, %%%s",mnemonic,regDest,reg1,reg2);
        break;
    case M_TYPE:
        sprintf_s(out, width, "%s %%%s, %%%s, 0x%x",mnemonic,regDest,reg1,immM);
        break;
    case I_TYPE:
        sprintf_s(out, width, "%s %%%s, 0x%x",mnemonic,regDest,immI);
        break;
    default:
        printf("Translation::instr_to_str Unexpected instruction!\n");
    }
}


ByteCode Translation_translate_str(char* source) {
    FILE* tempfile = fopen("temp_assembly.txt","w+");
    fputs(source,tempfile);
    fclose(tempfile);
    ByteCode result = Translation_translate("temp_assembly.txt");
    remove("temp_assembly.txt");
    return result;
}
