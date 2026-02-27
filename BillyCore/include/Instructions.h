#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "CPU.h"


Instruction Instr_M(const char* instr, uint8_t regDest, uint8_t regSrc1, uint8_t immediate);

Instruction Instr_R(const char* instr, uint8_t regDest, uint8_t regSrc1, uint8_t regSrc2);

Instruction Instr_I(const char* instr, uint8_t regDest, uint8_t immediate);

#endif
