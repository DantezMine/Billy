#include "CPU.h"
#include "Component.h"
#include "Instructions.h"
#include <malloc.h>
#include <stdio.h>

// #define CPU_DEBUG

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

    decodeFetch->instruction_High.data = 0;
    decodeFetch->instruction_Low.data = 0;
    decodeDecode->instruction_High.data = 0;
    decodeDecode->instruction_Low.data = 0;
    decodeExecute->instruction_High.data = 0;
    decodeExecute->instruction_Low.data = 0;
    decodeMemory->instruction_High.data = 0;
    decodeMemory->instruction_Low.data = 0;
    decodeWriteback->instruction_High.data = 0;
    decodeWriteback->instruction_Low.data = 0;
    regFileA->reg[0].data = 0;

    PC->data = PC_START;
    bool* branchPC = malloc(sizeof(bool));

    fetchStage = (StageFetch) {
        .memoryInstr = memoryInstr,
        .PC = PC,
        .branchPC = branchPC,
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
        .branchPC = branchPC,
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
#ifdef CPU_DEBUG
    printf("\n=======================================\n");
    printf("Instruction in Decode is: "BYTE_TO_BIN_PATTERN" "BYTE_TO_BIN_PATTERN"\n",BYTE_TO_BIN(decodeStage.decodeDecode->instruction_High.data),BYTE_TO_BIN(decodeStage.decodeDecode->instruction_High.data));
    printf("Instruction in Execute is: "BYTE_TO_BIN_PATTERN" "BYTE_TO_BIN_PATTERN"\n",BYTE_TO_BIN(executeStage.decodeExecute->instruction_High.data),BYTE_TO_BIN(executeStage.decodeExecute->instruction_High.data));
    printf("Instruction in Memory is: "BYTE_TO_BIN_PATTERN" "BYTE_TO_BIN_PATTERN"\n",BYTE_TO_BIN(memoryStage.decodeMemory->instruction_High.data),BYTE_TO_BIN(memoryStage.decodeMemory->instruction_High.data));
    printf("Instruction in Writeback is: "BYTE_TO_BIN_PATTERN" "BYTE_TO_BIN_PATTERN"\n",BYTE_TO_BIN(writebackStage.decodeWriteback->instruction_High.data),BYTE_TO_BIN(writebackStage.decodeWriteback->instruction_High.data));
#endif
    StageWriteback_update(&writebackStage);
    StageWriteback_clock(&writebackStage);

    StageMemory_update(&memoryStage);
    StageExecute_update(&executeStage);
    StageDecode_update(&decodeStage);
    StageFetch_update(&fetchStage);

    StageMemory_clock(&memoryStage);
    StageExecute_clock(&executeStage);
    StageDecode_clock(&decodeStage);
    StageFetch_clock(&fetchStage);
}


void CPU_SetInstructionMemory(uint8_t instr[128]) {
    Memory* memory = fetchStage.memoryInstr;
    for (int i=0; i<MEM_SIZE; i++) {
        memory->reg[i].data = instr[i];
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

RegisterFile* CPU_getRegisterFile() {
    return writebackStage.regFileA;
}


Register* CPU_getPC() {
    return fetchStage.PC;
}


uint16_t CPU_getStageFetchInstr() {
    return ((Instruction){
            .low =fetchStage.memoryInstr->reg[fetchStage.PC->data+1].data,
            .high=fetchStage.memoryInstr->reg[fetchStage.PC->data  ].data}).instr;
}

uint16_t CPU_getStageDecodeInstr() {
    return ((Instruction){
            .low =decodeStage.decodeDecode->instruction_Low.data,
            .high=decodeStage.decodeDecode->instruction_High.data}).instr;
}

uint16_t CPU_getStageExecuteInstr() {
    return ((Instruction){
            .low =executeStage.decodeExecute->instruction_Low.data,
            .high=executeStage.decodeExecute->instruction_High.data}).instr;
}

uint16_t CPU_getStageMemoryInstr() {
    return ((Instruction){
            .low =memoryStage.decodeMemory->instruction_Low.data,
            .high=memoryStage.decodeMemory->instruction_High.data}).instr;
}

uint16_t CPU_getStageWritebackInstr() {
    return ((Instruction){
            .low =writebackStage.decodeWriteback->instruction_Low.data,
            .high=writebackStage.decodeWriteback->instruction_High.data}).instr;
}
