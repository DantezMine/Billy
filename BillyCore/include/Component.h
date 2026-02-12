#ifndef COMPONENT_H
#define COMPONENT_H

#include <stdint.h>
#include <stdbool.h>


#define BYTE_TO_BIN_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BIN(byte)  \
  ((byte) & 0x80 ? '1' : '0'), \
  ((byte) & 0x40 ? '1' : '0'), \
  ((byte) & 0x20 ? '1' : '0'), \
  ((byte) & 0x10 ? '1' : '0'), \
  ((byte) & 0x08 ? '1' : '0'), \
  ((byte) & 0x04 ? '1' : '0'), \
  ((byte) & 0x02 ? '1' : '0'), \
  ((byte) & 0x01 ? '1' : '0') 


#define MEM_SIZE 128


typedef enum INSTR_TYPE {
    M_TYPE,
    R_TYPE,
    I_TYPE,
    NOP,
} INSTR_TYPE;


static const char* instr_table[] = {
    "NOP",
    "LDA",
    "STR",
    "LDI",
    "ADD",
    "SUB",
    "AND",
    "OR",
    "XOR",
    "LSL",
    "LSR",
    "ASR",
    "BEQ",
    "BLT",
    "JMP",
    "NOP",
};

static INSTR_TYPE instr_type[] = {
    NOP,
    M_TYPE,
    M_TYPE,
    I_TYPE,
    R_TYPE,
    R_TYPE,
    R_TYPE,
    R_TYPE,
    R_TYPE,
    R_TYPE,
    R_TYPE,
    R_TYPE,
    I_TYPE,
    I_TYPE,
    I_TYPE,
    NOP,
};

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
    Register reg[MEM_SIZE];
} Memory;


typedef struct DecodeUnit {
    Register instruction_High;
    Register instruction_Low;
    bool stall;
    bool branch;
} DecodeUnit;


typedef struct StageFetch {
    Memory* memoryInstr;
    Register* PC;
    bool* branchPC;
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
    Register* flagZero;
    Register* flagNegative;
    Register* flagOverflow;
    Register* PC;
    bool* branchPC;
} StageExecute;


typedef struct StageMemory {
    Memory* memoryData;
    RegisterFile* writeRegister;
    Register* regIn;
    Register* regOut;
    DecodeUnit* decodeMemory;
    DecodeUnit* decodeWriteback;
    DecodeUnit* decodeDecode;
    Register* regA;
    Register* regB;
} StageMemory;


typedef struct StageWriteback {
    RegisterFile* regFileA;
    RegisterFile* regFileB;
    RegisterFile* regFileWrite;
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

void StageFetch_clock(StageFetch* fetchStage);

void StageDecode_clock(StageDecode* decodeStage);

void StageExecute_clock(StageExecute* executeStage);

void StageMemory_clock(StageMemory* memoryStage);

void StageWriteback_clock(StageWriteback* writebackStage);

#endif
