#ifndef COMPONENT_H
#define COMPONENT_H

#include <stdint.h>


typedef struct Register {
    uint8_t data;
    uint8_t in;
} Register;


typedef struct RegisterFile {
    Register reg[8];
    uint8_t addrWrite;
    uint8_t dataIn;
} RegisterFile;


typedef struct Memory {
    Register reg[128];
} Memory;


typedef struct DecodeUnit {
    uint8_t instruction;
} DecodeUnit;


typedef struct StageFetch {
    Memory* memoryInstr;
} StageFetch;


typedef struct StageDecode {
    RegisterFile* regFileA;
    RegisterFile* regFileB;
    RegisterFile* regFileWrite;
    Register* regA;
    Register* regB;
} StageDecode;


typedef struct StageExecute {
    Register* regA;
    Register* regB;
    Register* regOut;
} StageExecute;


typedef struct StageMemory {
    Memory* memoryData;
    RegisterFile* writeRegister;
    Register* regIn;
    Register* regOut;
} StageMemory;


typedef struct StageWriteback {
    Register* regIn;
} StageWriteback;



void Register_clock(Register* reg);

void RegisterFile_clock(RegisterFile* regFile);

void StageFetch_update(StageFetch* fetchStage);

void StageDecode_update(StageDecode* decodeStage);

void StageExecute_update(StageExecute* executeStage);

void StageMemory_update(StageMemory* memoryStage);

#endif
