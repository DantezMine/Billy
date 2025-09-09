#include "Component.h"
#include <stdint.h>

static uint8_t getInstruction(uint8_t instrHigh, uint8_t instrLow);
static uint8_t getDestRegister(uint8_t instrHigh, uint8_t instrLow);
static uint8_t getSource1Register(uint8_t instrHigh, uint8_t instrLow);
static uint8_t getSource2Register(uint8_t instrHigh, uint8_t instrLow);
static uint8_t getImmediateMType(uint8_t instrHigh, uint8_t instrLow);
static uint8_t getImmediateIType(uint8_t instrHigh, uint8_t instrLow);
static bool getControlBit(uint8_t instr, uint8_t bitIndex);
static void updateFlags(StageExecute* executeStage, uint16_t out);

// Control signals
// M, R, I, J, ALU1, ALU2, ALU3, F, C0, C1
static bool control[] = {
    /* NOP */ 0,0,0,0,0,0,0,0,0,0,
    /* LDA */ 1,0,0,0,0,0,0,0,0,0,
    /* STR */ 1,0,0,0,0,0,0,0,0,0,
    /* LDI */ 0,0,1,0,0,0,0,0,0,0,
    /* ADD */ 0,1,0,0,0,0,0,1,0,0,
    /* SUB */ 0,1,0,0,1,0,0,1,0,0,
    /* AND */ 0,1,0,0,0,1,0,1,0,0,
    /* OR  */ 0,1,0,0,1,1,0,1,0,0,
    /* XOR */ 0,1,0,0,0,0,1,1,0,0,
    /* LSL */ 0,1,0,0,1,0,1,1,0,0,
    /* LSR */ 0,1,0,0,0,1,1,1,0,0,
    /* ASR */ 0,1,0,0,1,1,1,1,0,0,
    /* BEQ */ 0,0,0,1,0,0,0,0,1,0,
    /* BLT */ 0,0,0,1,0,0,0,0,0,1,
    /* JMP */ 0,0,0,1,0,0,0,0,1,1,
    /*     */ 0,0,0,0,0,0,0,0,0,0,
};

static int controlWidth = 8;


void Register_clock(Register* reg) {
    reg->data = reg->in;
}

void RegisterFile_clock(RegisterFile* regFile) {
    uint8_t address = regFile->addrWrite;
    regFile->reg[address].in = regFile->dataIn;
    Register_clock(&regFile->reg[address]);
}

void StageFetch_update(StageFetch* fetchStage) {
    uint8_t address = fetchStage->PC->data;
    uint8_t data_Low = fetchStage->memoryInstr->reg[address+1].data;
    uint8_t data_High = fetchStage->memoryInstr->reg[address].data;
    fetchStage->decodeDecode->instruction_Low->in = data_Low;
    fetchStage->decodeDecode->instruction_High->in = data_High;

    uint8_t instrDecodeHigh = fetchStage->decodeDecode->instruction_High->data;
    uint8_t instrDecodeLow = fetchStage->decodeDecode->instruction_Low->data;
    uint8_t instrExecuteHigh = fetchStage->decodeExecute->instruction_High->data;
    uint8_t instrExecuteLow = fetchStage->decodeExecute->instruction_Low->data;
    uint8_t instrDecode = getInstruction(instrDecodeHigh, instrDecodeLow);
    uint8_t instrExecute = getInstruction(instrExecuteHigh, instrExecuteLow);

    bool stall = !fetchStage->decodeDecode->stall && !getControlBit(instrDecode,3) && !getControlBit(instrExecute, 3);

    // if decode stage stalls or there is an unresolved branch instruction, stall the fetch
    fetchStage->decodeDecode->instruction_High->write = stall;
    fetchStage->decodeDecode->instruction_High->write = stall;
    // PC stall may be overwritten if branch is about to be resolved
    fetchStage->PC->write = stall | getControlBit(instrExecute, 3);
    fetchStage->PC->in = fetchStage->branchPC ? fetchStage->PC->in : fetchStage->PC->data+2;

    // TODO: only clock after updating all stages
    Register_clock(fetchStage->decodeDecode->instruction_Low);
    Register_clock(fetchStage->decodeDecode->instruction_High);
    Register_clock(fetchStage->PC);
}

void StageDecode_update(StageDecode* decodeStage) {
    uint8_t instrExecuteHigh = decodeStage->decodeExecute->instruction_High->data;
    uint8_t instrExecuteLow = decodeStage->decodeExecute->instruction_Low->data;
    uint8_t instrMemHigh = decodeStage->decodeMemory->instruction_High->data;
    uint8_t instrMemLow = decodeStage->decodeMemory->instruction_Low->data;
    uint8_t instrDecHigh = decodeStage->decodeDecode->instruction_High->data;
    uint8_t instrDecLow = decodeStage->decodeDecode->instruction_Low->data;

    uint8_t instrExecute = getInstruction(instrExecuteHigh,instrExecuteLow);
    uint8_t instrMem = getInstruction(instrMemHigh,instrMemLow);
    uint8_t instrDec = getInstruction(instrDecHigh,instrDecLow);

    // I-type has dest register at different position
    uint8_t destRegExecute = getControlBit(instrExecute, 2) ?
        getSource1Register(instrExecuteHigh,instrExecuteLow) : getDestRegister(instrExecuteHigh,instrExecuteLow);
    uint8_t destRegMemory = getControlBit(instrMem, 2) ?
        getSource1Register(instrMemHigh, instrMemLow) : getDestRegister(instrMemHigh,instrMemHigh);
    uint8_t sourceReg1 = getSource1Register(instrDecHigh,instrDecLow);
    uint8_t sourceReg2 = getSource2Register(instrDecHigh,instrDecLow);

    bool E1 = destRegExecute == sourceReg1;
    bool E2 = destRegExecute == sourceReg2;
    bool M1 = destRegMemory == sourceReg1;
    bool M2 = destRegMemory == sourceReg2;
    bool dependence = (E1 & !M1) | (E2 & !M2);

    decodeStage->decodeDecode->stall = dependence;

    // write to register A if not I-type and no dependence nor forwarding
    if (!getControlBit(instrDec, 2) & !E1 & !M1) decodeStage->regA->in = decodeStage->regFileA->reg[sourceReg1].data;
    // write to register B if R-type and no dependence nor forwarding
    if (getControlBit(instrDec, 1) & !E2 & !M2) decodeStage->regB->in = decodeStage->regFileB->reg[sourceReg2].data;
    // write immediate to register B
    if (getControlBit(instrDec, 0)) decodeStage->regB->in = getImmediateMType(instrDecHigh, instrDecLow);
    if (getControlBit(instrDec, 2) || getControlBit(instrDec, 3)) decodeStage->regB->in = getImmediateIType(instrDecHigh, instrDecLow);
}

void StageExecute_update(StageExecute* executeStage) { 
    uint8_t instrExecuteHigh = executeStage->decodeExecute->instruction_High->data;
    uint8_t instrExecuteLow = executeStage->decodeExecute->instruction_Low->data;
    uint8_t instrExecute = getInstruction(instrExecuteHigh, instrExecuteLow);
    uint8_t instrALU = getControlBit(instrExecute,4) + 2*getControlBit(instrExecute,5) + 4*getControlBit(instrExecute,6);
    
    uint16_t out = 0;
    uint16_t MSB = executeStage->regA->data & 128;
    switch (instrALU) {
        case 0: // ADD
            out = executeStage->regA->data + executeStage->regB->data;
            if (getControlBit(instrALU, 7)) updateFlags(executeStage, out);
            break;
        case 1: // SUB
            out = executeStage->regA->data - executeStage->regB->data;
            if (getControlBit(instrALU, 7)) updateFlags(executeStage, out);
            break;
        case 2: // AND
            out = executeStage->regA->data & executeStage->regB->data;
            if (getControlBit(instrALU, 7)) updateFlags(executeStage, out);
            break;
        case 3: // OR
            out = executeStage->regA->data | executeStage->regB->data;
            if (getControlBit(instrALU, 7)) updateFlags(executeStage, out);
            break;
        case 4: // XOR
            out = executeStage->regA->data ^ executeStage->regB->data;
            if (getControlBit(instrALU, 7)) updateFlags(executeStage, out);
            break;
        case 5: // LSL
            out = executeStage->regA->data << 1;
            if (getControlBit(instrALU, 7)) updateFlags(executeStage, out);
            break;
        case 6: // LSR
            out = executeStage->regA->data >> 1;
            if (getControlBit(instrALU, 7)) updateFlags(executeStage, out);
            break;
        case 7: // ASR
            out = executeStage->regA->data >> 1 | MSB;
            if (getControlBit(instrALU, 7)) updateFlags(executeStage, out);
            break;
    }

    executeStage->regOut->in = out;

    bool C0 = getControlBit(instrALU, 8);
    bool C1 = getControlBit(instrALU, 9);
    bool condition = (C0 & !C1 & executeStage->flagZero->data) | (!C0  & C1 & executeStage->flagNegative->data) | (C0 & C1);
    executeStage->PC->in = condition ? out : executeStage->PC->in;
    executeStage->branchPC = condition;
    
}

void StageMemory_update(StageMemory* memoryStage);

void StageWriteback_update(StageWriteback* writebackStage);


/*
 * Static Functions
*/


static uint8_t getInstruction(uint8_t instrHigh, uint8_t instrLow) {
    return (instrHigh & 0b11110000) >> 4;
}

static uint8_t getDestRegister(uint8_t instrHigh, uint8_t instrLow) {
    return (instrHigh & 0b00000001) << 2 | (instrLow & 0b11000000) >> 6;
}

static uint8_t getSource1Register(uint8_t instrHigh, uint8_t instrLow) {
    return (instrHigh & 0b00001110) >> 1;
}

static uint8_t getSource2Register(uint8_t instrHigh, uint8_t instrLow) {
    return (instrLow & 0b00111000) >> 3;
}

static uint8_t getImmediateMType(uint8_t instrHigh, uint8_t instrLow) {
    return (instrLow & 0b00011111);
}

static uint8_t getImmediateIType(uint8_t instrHigh, uint8_t instrLow) {
    return (instrLow & 0b11111111);
}

static bool getControlBit(uint8_t instr, uint8_t bitIndex) {
    return control[instr*controlWidth+bitIndex];
}

static void updateFlags(StageExecute* executeStage, uint16_t out) {
    executeStage->flagZero->in = out == 0;
    executeStage->flagNegative->in = out < 0;
    executeStage->flagOverflow->in = out > 255;
}
