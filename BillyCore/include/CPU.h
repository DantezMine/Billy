#ifndef CPU_H
#define CPU_H

#include "Component.h"

#define PC_START 0

void CPU_Init();

void CPU_Clock();

void CPU_SetInstructionMemory(uint8_t instr[128]);

void CPU_SetDataMemory(uint8_t data[128]);

StageFetch* CPU_getStageFetch();

StageDecode* CPU_getStageDecode();

StageExecute* CPU_getStageExecute();

StageMemory* CPU_getStageMemory();

StageWriteback* CPU_getStageWriteback();

RegisterFile* CPU_getRegisterFile();
#endif
