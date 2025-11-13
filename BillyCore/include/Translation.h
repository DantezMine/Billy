#ifndef TRANSLATION_H
#define TRANSLATION_H

#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct ByteCode {
    size_t num_instr;
    uint8_t* instr;
} ByteCode;

#define BAD_STATE 4

ByteCode Translation_translate(char* ass);

ByteCode Translation_translate(char* ass) {
    typedef enum State {
        SOL,
        WORD,
        END,
        COMMENT,
        INSTR,
        LABEL,
        REG_R,
        REG_NUM,
        IMM,
        INSTR_LABEL,
        ERROR,
        ERROR_ILLEGAL_CHAR,
    } State;


    int num_lines = 0;
    int num_instr = 0;
    int num_labels = 0;
    int indx = 0;

    State state = SOL;

    while (state != END || state >= ERROR) {
        char c = ass[indx];
        switch (state) {
            case SOL:
                if (c == '#') { state = COMMENT; }
                else if (isalpha(c)) { state = WORD; }
                else if (isdigit(c)) { state = ERROR_ILLEGAL_CHAR; }
                else if (c == '\n') { state = SOL; num_lines++; }
                else { state = ERROR; }
                break;
            case WORD:
                if (isalnum(c)) { }
                else if (iswspace(c)) { state = INSTR; num_instr++; }
                else if (c == ':') { state = LABEL; num_labels++; }
                else { state = ERROR; }
                break;
            case COMMENT:
                if (c == '\n') { state = SOL; num_lines++; }
                break;
            case INSTR:
                if (c == '%') { state = REG_R; }
                else if (isdigit(c)) { state = IMM; }
                else if (isalpha(c)) { state = INSTR_LABEL; }
                else if (c == '\n') { state = SOL; num_lines++; }
                else { state = ERROR; }
                break;
            case INSTR_LABEL:
                if (isalpha(c)) { }
                else if (iswspace(c) || c == ',') { state = INSTR; }
                else if (c == '\n') { state = SOL; num_lines++; }
                else { state = ERROR; }
                break;
            case LABEL:
                if (c == '\n') { state = SOL; }
                else if (iswspace(c)) { }
                else { state = ERROR; }
                break;
            case REG_R:
                if (c == 'r') { state = REG_NUM; }
                else { state = ERROR; }
                break;
            case REG_NUM:
                if (isdigit(c)) { }
                else if (iswspace(c) || c == ',') { state = INSTR; }
                else if (c == '\n') { state = SOL; num_lines++; }
                else { state = ERROR; }
                break;
            case IMM:
                if (isdigit(c)) { }
                else if (iswspace(c) || c == ',') { state = INSTR; }
                else if (c == '\n') { state = SOL; num_lines++; }
                else { state = ERROR; }
                break;
            default:
                break;
        }
        indx++;
    }
}

#endif
