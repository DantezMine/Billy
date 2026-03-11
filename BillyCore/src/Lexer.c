#include "Lexer.h"
#include "pcre2posix.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

static int find_reg(const char* reg_name);

static int parse_imm(const char* imm_name);

static Token Lexer_Match(Lexer_Iterator* it);

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

static char* pattern_next_word = "^([ \\t]*(.*))";
static regex_t regexp_next_word;

static Lexer_Pattern patterns[] = {
    {.token_type = WORD, .pattern =     "^([a-zA-Z][a-zA-Z0-9]*)"},
    {.token_type = REG, .pattern =      "^(%(r([a-zA-Z0-9]{1,2})))"},
    {.token_type = IMM, .pattern =      "^((0x[0-9a-fA-F]*)|(0b[0-1]*)|([0-9]+))"},
    {.token_type = COMMA, .pattern =    "^,"},
    {.token_type = COMMENT, .pattern =  "^(#[^\\n]*)"},
    {.token_type = NEWLINE, .pattern =  "^\\n"},
    {.token_type = DOT, .pattern =      "^\\."},
    {.token_type = COLON, .pattern =    "^:"},
    {.token_type = -1, .pattern = ""},
};


void Lexer_init(Lexer_Iterator *it) {
    int num_patterns = 0;
    while (patterns[num_patterns].token_type != (Token_Type)-1) num_patterns++;
    it->num_patterns = num_patterns;

    for (int i=0; i<it->num_patterns; i++) {
        regcomp(&patterns[i].regexp,patterns[i].pattern,REG_EXTENDED);
    }
    regcomp(&regexp_next_word,pattern_next_word,REG_EXTENDED|REG_DOTALL);
}

Token Lexer_next(Lexer_Iterator* it) {
    Token token = Lexer_Match(it);
    it->pos += token.bytes_consumed;
    return token;
}

Token Lexer_peek(Lexer_Iterator* it) {
    return Lexer_Match(it);
}

static Token Lexer_Match(Lexer_Iterator* it) {
    char next_word[MAX_TOKEN_SIZE] = "";

    Token res = (Token) {.type=INVALID};

    regmatch_t regmatch_next_word[4];
    if (regexec(&regexp_next_word, it->pos, 4, regmatch_next_word, 0) != 0) {
        printf("LEXER::MATCH::Failed finding next word\n");
        return res;
    }

    int nw_indx = 0;
    while (regmatch_next_word[nw_indx].rm_eo != -1) nw_indx++;
    nw_indx -= 1;

    int nw_len = fmin(regmatch_next_word[nw_indx].rm_eo-regmatch_next_word[nw_indx].rm_so, MAX_TOKEN_SIZE-1);
    char* nw_start = it->pos+regmatch_next_word[nw_indx].rm_so;

    memcpy(next_word,nw_start,nw_len);
    next_word[nw_len] = '\0';

    // printf("%s\n",it->pos);
    // printf("%s\n",next_word);
    int bytes_wspace = regmatch_next_word[2].rm_so;


    regmatch_t regmatch[5];
    Token_Type match = -1;
    for (int i=0; i<it->num_patterns; i++) {
        int d = regexec(&patterns[i].regexp, next_word, 5, regmatch, 0);
        if (d==0) {
            match = patterns[i].token_type;
            // printf("Found regex match of type %d\n",match);
            break;
        }
    }
    if (match == (Token_Type)-1) {
        printf("LEXER::MATCH::No match found in string starting with \"%s\"\n",next_word);
        return res;
    }

    res.type = match;


    res.bytes_consumed = bytes_wspace+regmatch[0].rm_eo-regmatch[0].rm_so;

    int match_len;
    char* match_start;

    switch (res.type) {
    case WORD:
        match_len = regmatch[1].rm_eo-regmatch[1].rm_so;
        match_start = next_word+regmatch[1].rm_so;
        memcpy(res.name, match_start, match_len);
        break;
    case REG:
        match_len = regmatch[2].rm_eo-regmatch[2].rm_so;
        match_start = next_word+regmatch[2].rm_so;
        memcpy(res.name, match_start, match_len);
        break;
    case IMM:
        match_len = regmatch[1].rm_eo-regmatch[1].rm_so;
        match_start = next_word+regmatch[1].rm_so;
        memcpy(res.name, match_start, match_len);
        break;
    case COMMA:
        break;
    case COMMENT:
        break;
    case END:
        break;
    case NEWLINE:
        break;
    default:
        break;
    }
    return res;
}

static int find_reg(const char* reg_name) {
    int reg = -1;
    for (int i=0; i<8; i++) {
        if (!strcmp(reg_name,register_labels[i])) {
            reg = i;
            break;
        }
    }
    if (reg == -1) {
        printf("Lexer::Match::Register label \"%s\" doesn't match possible registers.\n",reg_name);
    }
    return reg;
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
