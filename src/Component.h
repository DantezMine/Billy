#ifndef COMPONENT_H
#define COMPONENT_H

#include <stdint.h>
#include <stdbool.h>


typedef struct Register {
    uint8_t data;
    uint8_t in;
    bool write;
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
    Register* instruction_High;
    Register* instruction_Low;
    bool stall;
    bool branch;
} DecodeUnit;


typedef struct StageFetch {
    Memory* memoryInstr;
    Register* PC;
    DecodeUnit* decodeFetch;
    DecodeUnit* decodeDecode;
    DecodeUnit* decodeExecute;
} StageFetch;


typedef struct StageDecode {
    RegisterFile* regFileA;
    RegisterFile* regFileB;
    RegisterFile* regFileWrite;
    Register* regA;
    Register* regB;
    DecodeUnit* decodeDecode;
    DecodeUnit* decodeExecute;
    DecodeUnit* decodeMemory;
} StageDecode;


typedef struct StageExecute {
    Register* regA;
    Register* regB;
    Register* regOut;
    DecodeUnit* decodeExecute;
    DecodeUnit* decodeMemory;
} StageExecute;


typedef struct StageMemory {
    Memory* memoryData;
    RegisterFile* writeRegister;
    Register* regIn;
    Register* regOut;
    DecodeUnit* decodeMemory;
    DecodeUnit* decodeWriteback;
} StageMemory;


typedef struct StageWriteback {
    Register* regIn;
    DecodeUnit* decodeWriteback;
} StageWriteback;



void Register_clock(Register* reg);

void RegisterFile_clock(RegisterFile* regFile);

void StageFetch_update(StageFetch* fetchStage);

void StageDecode_update(StageDecode* decodeStage);

void StageExecute_update(StageExecute* executeStage);

void StageMemory_update(StageMemory* memoryStage);

void StageWriteback_update(StageWriteback* writebackStage);

#endif
