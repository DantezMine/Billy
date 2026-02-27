#ifndef CPU_H
#define CPU_H

#include "Component.h"

#define PC_START 0

typedef union Instruction {
    struct {
        uint8_t low;
        uint8_t high;
    };
    uint16_t instr;
} Instruction;


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

Register* CPU_getPC();

uint16_t CPU_getStageFetchInstr();

uint16_t CPU_getStageDecodeInstr();

uint16_t CPU_getStageExecuteInstr();

uint16_t CPU_getStageMemoryInstr();

uint16_t CPU_getStageWritebackInstr();
#endif
