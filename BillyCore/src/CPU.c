#include "CPU.h"
#include "Component.h"
#include <malloc.h>
#include <stdio.h>

static StageFetch fetchStage;
static StageDecode decodeStage;
static StageExecute executeStage;
static StageMemory memoryStage;
static StageWriteback writebackStage;

void CPU_Init() {
    Memory* memoryInstr = malloc(sizeof(Memory));
    Memory* memoryData = malloc(sizeof(Memory));
    DecodeUnit* decodeFetch = malloc(sizeof(DecodeUnit));
    DecodeUnit* decodeDecode = malloc(sizeof(DecodeUnit));
    DecodeUnit* decodeExecute = malloc(sizeof(DecodeUnit));
    DecodeUnit* decodeMemory = malloc(sizeof(DecodeUnit));
    DecodeUnit* decodeWriteback = malloc(sizeof(DecodeUnit));
    RegisterFile* regFileA = malloc(sizeof(RegisterFile));
    RegisterFile* regFileB = malloc(sizeof(RegisterFile));
    RegisterFile* regFileWrite = malloc(sizeof(RegisterFile));
    Register* PC = malloc(sizeof(Register));
    Register* regA = malloc(sizeof(Register));
    Register* regB = malloc(sizeof(Register));
    Register* regExecMem = malloc(sizeof(Register));
    Register* flagZero = malloc(sizeof(Register));
    Register* flagNegative = malloc(sizeof(Register));
    Register* flagOverflow = malloc(sizeof(Register));
    Register* regMemWriteback = malloc(sizeof(Register));

    PC->data = PC_START;

    fetchStage = (StageFetch) {
        .memoryInstr = memoryInstr,
        .PC = PC,
        .branchPC = false,
        .decodeFetch = decodeFetch,
        .decodeDecode = decodeDecode,
        .decodeExecute = decodeExecute,
    };
    decodeStage = (StageDecode) {
        .regFileA = regFileA,
        .regFileB = regFileB,
        .regFileWrite = regFileWrite,
        .regA = regA,
        .regB = regB,
        .decodeDecode = decodeDecode,
        .decodeExecute = decodeExecute,
        .decodeMemory = decodeMemory,
    };
    executeStage = (StageExecute) {
        .regA = regA,
        .regB = regB,
        .regOut = regExecMem,
        .decodeExecute = decodeExecute,
        .decodeMemory = decodeMemory,
        .flagZero = flagZero,
        .flagNegative = flagNegative,
        .flagOverflow = flagOverflow,
        .PC = PC,
        .branchPC = false,
    };
    memoryStage = (StageMemory) {
        .memoryData = memoryData,
        .writeRegister = regFileWrite,
        .regIn = regExecMem,
        .regOut = regMemWriteback,
        .decodeMemory = decodeMemory,
        .decodeWriteback = decodeWriteback,
        .decodeDecode = decodeDecode,
        .regA = regA,
        .regB = regB,
    };
    writebackStage = (StageWriteback) {
        .regFileA = regFileA,
        .regFileB = regFileB,
        .regFileWrite = regFileWrite,
        .regIn = regMemWriteback,
        .decodeWriteback = decodeWriteback,
    };
}


void CPU_Clock() {
    StageWriteback_update(&writebackStage);
    StageMemory_update(&memoryStage);
    StageExecute_update(&executeStage);
    StageDecode_update(&decodeStage);
    StageFetch_update(&fetchStage);

    StageWriteback_clock(&writebackStage);
    StageMemory_clock(&memoryStage);
    StageExecute_clock(&executeStage);
    StageDecode_clock(&decodeStage);
    StageFetch_clock(&fetchStage);
}


void CPU_SetInstructionMemory(uint8_t instr[128]) {
    Memory* memory = fetchStage.memoryInstr;
    for (int i=0; i<MEM_SIZE; i++) {
        memory->reg[i].data = instr[i];
        // printf("Instr at %d is "BYTE_TO_BIN_PATTERN"\n",i, BYTE_TO_BIN(memory->reg[i].data));
    }
}


void CPU_SetDataMemory(uint8_t data[128]) {
    Memory* memory = memoryStage.memoryData;
    for (int i=0; i<MEM_SIZE; i++) {
        memory->reg[i].data = data[i];
    }
}


StageFetch* CPU_getStageFetch() {
    return &fetchStage;
}


StageDecode* CPU_getStageDecode() {
    return &decodeStage;
}


StageExecute* CPU_getStageExecute() {
    return &executeStage;
}


StageMemory* CPU_getStageMemory() {
    return &memoryStage;
}


StageWriteback* CPU_getStageWriteback() {
    return &writebackStage;
}
