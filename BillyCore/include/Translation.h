#ifndef TRANSLATION_H
#define TRANSLATION_H

#include <stdint.h>
#include <stdlib.h>

typedef struct ByteCode {
    size_t num_instr;
    uint8_t* instr;
} ByteCode;

ByteCode Translation_translate(char* ass);

ByteCode Translation_translate(char* ass) {
    int lines = 0;
    int indx = 0;
    while (ass[indx] != 0) {
        lines += ass[indx] == '\n' ? 1 : 0;
    }
    int curr_instr = 0;
    indx = 0;
    for (int i=0; i<lines; i++) {
        
    }

}

#endif
