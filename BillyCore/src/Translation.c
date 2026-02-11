#include "Translation.h"

static Token* tokenize(char* line);
static uint16_t tokens_to_instr(Token* tokens, Label* labels);
static Label* get_label(Label* labels, char* name);


// ByteCode.instr must be freed after use
ByteCode Translation_translate(char* assm) {
    Label* labels = malloc(16*sizeof(Label));
    size_t labels_indx = 0;
    size_t labels_size = 16;

    int num_instr = 0;
    int line_count = 0;
    
    size_t offset = 0;

    // Dynamic array
    Token* tokens = malloc(sizeof(Token)*100);
    int tokens_index = 0;
    int tokens_size = 0;
    char* line = malloc(512*sizeof(char));

    while (1) {
        sscanf(assm+offset,"%511[\n]",line);
        if (*line == EOF) break;
        offset += strlen(line)+1;

        line_count++;

        // Get tokens from line string and add them to the tokens list
        Token* line_tokens = tokenize(line);
        int i = 0;
        while(line_tokens[i].type != END) {
            // resize tokens array if needed
            if (tokens_index == tokens_size-1) {
                tokens_size = floor(tokens_size*1.5);
                realloc(tokens, tokens_size);
            }
            tokens[tokens_index++] = line_tokens[i++];
        }
        tokens[tokens_index++] = line_tokens[i++];
        free(line_tokens);

        // count instructions and set label definitions
        Token* first_token = &tokens[tokens_index];
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
                if (get_label(labels,first_token->name) != NULL) {
                    printf("Multiple definitions of label %s\n",first_token->name);
                    return (ByteCode){-1,NULL};
                }
                labels[labels_indx++] = (Label){.value=num_instr+1, .name={*line_tokens[0].name}};
                break;
            default:
                printf("Unexpected first token of type %d on line %d\n",first_token->type,line_count);
                return (ByteCode){-1,NULL};
                break;
        }
    }

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
        bc_out.instr[i] = tokens_to_instr(current_token,labels);
        // move current_token to next line
        int tokens_in_instr = 0; //includes the END token
        while(current_token[i+tokens_in_instr++].type != END);
        current_token += tokens_in_instr;
    }

    free(labels);
    free(tokens);
    return bc_out;
}
