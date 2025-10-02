#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

uint16_t Instr_M(const char* instr, uint8_t regSrc1, uint8_t regDest, uint8_t immediate) {
    uint8_t opcode = 0;
    if (strcmp(instr,"LDA") == 0)
        opcode = 1;
    else if (strcmp(instr,"STR") == 0)
        opcode = 2;
    else
        return -1;
    return (opcode << 12) | ((0x7 & regSrc1) << 9) | ((0x7 & regDest) << 6) | (0x3f & immediate);
}


uint16_t Instr_R(const char* instr, uint8_t regSrc1, uint8_t regDest, uint8_t regSrc2) {
    uint8_t opcode;
    if (strcmp(instr,"ADD") == 0)
        opcode = 4;
    else if (strcmp(instr,"SUB") == 0)
        opcode = 5;
    else if (strcmp(instr,"AND") == 0)
        opcode = 6;
    else if (strcmp(instr,"OR") == 0)
        opcode = 7;
    else if (strcmp(instr,"XOR") == 0)
        opcode = 8;
    else if (strcmp(instr,"LSL") == 0)
        opcode = 9;
    else if (strcmp(instr,"LSR") == 0)
        opcode = 10;
    else if (strcmp(instr,"ASR") == 0)
        opcode = 11;
    else
        return -1;
    return (opcode << 12) | ((0x7 & regSrc1) << 9) | ((0x7 & regDest) << 6) | ((0x7 & regSrc2) << 3);
}

uint16_t Instr_I(const char* instr, uint8_t regDest, uint8_t immediate) {
    uint8_t opcode;
    if (strcmp(instr,"LDI") == 0)
        opcode = 3;
    else if (strcmp(instr,"BEQ") == 0)
        opcode = 12;
    else if (strcmp(instr,"BLT") == 0)
        opcode = 13;
    else if (strcmp(instr,"JMP") == 0)
        opcode = 14;
    else
        return -1;
    return (opcode << 12) | ((0x7 & regDest) << 9) | immediate;
}

#endif
