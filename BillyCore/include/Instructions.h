#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef union Instruction {
    struct {
        uint8_t low;
        uint8_t high;
    };
    uint16_t instr;
} Instruction;

Instruction Instr_M(const char* instr, uint8_t regDest, uint8_t regSrc1, uint8_t immediate) {
    uint8_t opcode = 0;
    if (strcmp(instr,"LDA") == 0)
        opcode = 1;
    else if (strcmp(instr,"STR") == 0)
        opcode = 2;
    else
        return (Instruction) { .instr=-1 };
    uint16_t instruction = (opcode << 12) | ((0x7 & regDest) << 9) | ((0x7 & regSrc1) << 6) | (0x3f & immediate);
    return (Instruction){ .instr=instruction };
}


Instruction Instr_R(const char* instr, uint8_t regDest, uint8_t regSrc1, uint8_t regSrc2) {
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
        return (Instruction) { .instr=-1 };
    uint16_t instruction = (opcode << 12) | ((0x7 & regDest) << 9) | ((0x7 & regSrc1) << 6) | ((0x7 & regSrc2) << 3);
    return (Instruction) {.instr=instruction };
}

Instruction Instr_I(const char* instr, uint8_t regDest, uint8_t immediate) {
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
        return (Instruction) { .instr=-1 };
    uint16_t instruction = (opcode << 12) | ((0x7 & regDest) << 9) | immediate;
    return (Instruction) { .instr=instruction };
}

#endif
