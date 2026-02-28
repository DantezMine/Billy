#include "Component.h"
#include <stdint.h>
#include <stdio.h>


// #define CPU_DEBUG

#define DATA_MEM_OFFSET 128

static uint8_t getInstruction(uint8_t instrHigh, uint8_t instrLow);
static uint8_t getDestRegister(uint8_t instrHigh, uint8_t instrLow);
static uint8_t getSource1Register(uint8_t instrHigh, uint8_t instrLow);
static uint8_t getSource2Register(uint8_t instrHigh, uint8_t instrLow);
static uint8_t getImmediateMType(uint8_t instrHigh, uint8_t instrLow);
static uint8_t getImmediateIType(uint8_t instrHigh, uint8_t instrLow);
static bool getControlBitDecode(uint8_t instr, uint8_t bitIndex);
static bool getControlBitExecute(uint8_t instr, uint8_t bitIndex);
static bool getControlBitMemory(uint8_t instr, uint8_t bitIndex);
static bool getControlBitWriteback(uint8_t instr, uint8_t bitIndex);
static void updateFlags(StageExecute* executeStage, uint16_t out);



/*
 * Control signals
*/

// branch, regA, regB, immI, immM
static bool controlDecode[] = {
    /* NOP */ 0,0,0,0,0,
    /* LDA */ 0,1,0,0,1,
    /* STR */ 0,1,0,0,1,
    /* LDI */ 0,0,0,1,0,
    /* ADD */ 0,1,1,0,0,
    /* SUB */ 0,1,1,0,0,
    /* AND */ 0,1,1,0,0,
    /* OR  */ 0,1,1,0,0,
    /* XOR */ 0,1,1,0,0,
    /* LSL */ 0,1,1,0,0,
    /* LSR */ 0,1,1,0,0,
    /* ASR */ 0,1,1,0,0,
    /* BEQ */ 1,0,0,1,0,
    /* BLT */ 1,0,0,1,0,
    /* JMP */ 1,0,0,1,0,
    /*     */ 0,0,0,0,0,
};
static int controlWidthDecode = 5;

// branch, destI, ALU1, ALU2, ALU3, Flags, branchCond1, branchCond2
static bool controlExecute[] = {
    /* NOP */ 0,0,0,0,0,0,0,0,
    /* LDA */ 0,0,0,0,0,0,0,0,
    /* STR */ 0,0,0,0,0,0,0,0,
    /* LDI */ 0,1,0,0,0,0,0,0,
    /* ADD */ 0,0,0,0,0,1,0,0,
    /* SUB */ 0,0,1,0,0,1,0,0,
    /* AND */ 0,0,0,1,0,1,0,0,
    /* OR  */ 0,0,1,1,0,1,0,0,
    /* XOR */ 0,0,0,0,1,1,0,0,
    /* LSL */ 0,0,1,0,1,1,0,0,
    /* LSR */ 0,0,0,1,1,1,0,0,
    /* ASR */ 0,0,1,1,1,1,0,0,
    /* BEQ */ 1,1,0,0,0,0,1,0,
    /* BLT */ 1,1,0,0,0,0,0,1,
    /* JMP */ 1,1,0,0,0,0,1,1,
    /*     */ 0,0,0,0,0,0,0,0,
};
static int controlWidthExecute = 8;

// branch, destI, MR, MW
static bool controlMemory[] = {
    /* NOP */ 0,0,0,0,
    /* LDA */ 0,0,1,0,
    /* STR */ 0,0,0,1,
    /* LDI */ 0,1,0,0,
    /* ADD */ 0,0,0,0,
    /* SUB */ 0,0,0,0,
    /* AND */ 0,0,0,0,
    /* OR  */ 0,0,0,0,
    /* XOR */ 0,0,0,0,
    /* LSL */ 0,0,0,0,
    /* LSR */ 0,0,0,0,
    /* ASR */ 0,0,0,0,
    /* BEQ */ 1,1,0,0,
    /* BLT */ 1,1,0,0,
    /* JMP */ 1,1,0,0,
    /*     */ 0,0,0,0,
};
static int controlWidthMemory = 4;

// branch, destI, write
static bool controlWriteback[] = {
    /* NOP */ 0,0,0,
    /* LDA */ 0,0,1,
    /* STR */ 0,0,0,
    /* LDI */ 0,1,1,
    /* ADD */ 0,0,1,
    /* SUB */ 0,0,1,
    /* AND */ 0,0,1,
    /* OR  */ 0,0,1,
    /* XOR */ 0,0,1,
    /* LSL */ 0,0,1,
    /* LSR */ 0,0,1,
    /* ASR */ 0,0,1,
    /* BEQ */ 1,1,0,
    /* BLT */ 1,1,0,
    /* JMP */ 1,1,0,
    /*     */ 0,0,0,
};
static int controlWidthWriteback = 3;


/*
 * Global Functions
*/

char* INSTR_STRINGS[] = {
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
};

void Register_clock(Register* reg) {
    if (reg->write) reg->data = reg->in;
}

void RegisterFile_clock(RegisterFile* regFile) {
    uint8_t address = regFile->addrWrite;
    regFile->reg[address].in = regFile->dataIn;
    // strictly enfore zero register
    regFile->reg[0].in = 0;
    Register_clock(&regFile->reg[address]);
}

void StageFetch_update(StageFetch* fetchStage) {
    uint8_t address = fetchStage->PC->data;
    uint8_t data_Low = fetchStage->memoryInstr->reg[address+1].data;
    uint8_t data_High = fetchStage->memoryInstr->reg[address].data;

    uint8_t instrDecodeHigh = fetchStage->decodeDecode->instruction_High.data;
    uint8_t instrDecodeLow = fetchStage->decodeDecode->instruction_Low.data;
    uint8_t instrExecuteHigh = fetchStage->decodeExecute->instruction_High.data;
    uint8_t instrExecuteLow = fetchStage->decodeExecute->instruction_Low.data;
    uint8_t instrDecode = getInstruction(instrDecodeHigh, instrDecodeLow);
    uint8_t instrExecute = getInstruction(instrExecuteHigh, instrExecuteLow);

    bool stall = fetchStage->decodeDecode->stall || getControlBitDecode(instrDecode,0) || getControlBitExecute(instrExecute,0);

    // if decode stage stalls or there is an unresolved branch instruction, stall the fetch
    fetchStage->decodeDecode->instruction_High.in = data_High * !stall;
    fetchStage->decodeDecode->instruction_Low.in = data_Low * !stall;
    fetchStage->decodeDecode->instruction_High.write = !fetchStage->decodeDecode->stall;
    fetchStage->decodeDecode->instruction_Low.write = !fetchStage->decodeDecode->stall;
    fetchStage->PC->write = !stall | *fetchStage->branchPC;
    fetchStage->PC->in = *fetchStage->branchPC ? fetchStage->PC->in : fetchStage->PC->data+2;

#ifdef CPU_DEBUG
    printf("Fetch -- Instruction: %s\n",INSTR_STRINGS[getInstruction(data_High,data_Low)]);
    printf("Fetch -- Fetching address: %d\n",address);
    printf("Fetch -- Stall: %d, branchPC: %d\n",stall,*fetchStage->branchPC);
#endif
}

void StageDecode_update(StageDecode* decodeStage) {
    uint8_t instrExecuteHigh = decodeStage->decodeExecute->instruction_High.data;
    uint8_t instrExecuteLow = decodeStage->decodeExecute->instruction_Low.data;
    uint8_t instrMemHigh = decodeStage->decodeMemory->instruction_High.data;
    uint8_t instrMemLow = decodeStage->decodeMemory->instruction_Low.data;
    uint8_t instrDecHigh = decodeStage->decodeDecode->instruction_High.data;
    uint8_t instrDecLow = decodeStage->decodeDecode->instruction_Low.data;

    uint8_t instrExecute = getInstruction(instrExecuteHigh,instrExecuteLow);
    uint8_t instrMem = getInstruction(instrMemHigh,instrMemLow);
    uint8_t instrDec = getInstruction(instrDecHigh,instrDecLow);

    uint8_t destRegExecute = getDestRegister(instrExecuteHigh,instrExecuteLow);
    uint8_t destRegMemory = getDestRegister(instrMemHigh,instrMemHigh);
    uint8_t sourceReg1 = getSource1Register(instrDecHigh,instrDecLow);
    uint8_t sourceReg2 = getSource2Register(instrDecHigh,instrDecLow);

    bool E1 = destRegExecute == sourceReg1;
    bool E2 = destRegExecute == sourceReg2;
    bool M1 = destRegMemory == sourceReg1;
    bool M2 = destRegMemory == sourceReg2;

    // flow dependence and no forwarding -> stall
    decodeStage->decodeDecode->stall = (getControlBitDecode(instrDec,1) && E1) || (getControlBitDecode(instrDec,2) && E2);

    // can always write to A and B registers, even in case of stall, since execution stage never stalls
    decodeStage->regA->write = true;
    decodeStage->regB->write = true;

    // write to register A if no dependence nor forwarding
    if (getControlBitDecode(instrDec,1) & !E1 & !M1) {
        decodeStage->regA->in = decodeStage->regFileA->reg[sourceReg1].data;
    }

    // Multiplexer:
    // write to register B if no dependence nor forwarding
    if (getControlBitDecode(instrDec,2) & !E2 & !M2) {
        decodeStage->regB->in = decodeStage->regFileB->reg[sourceReg2].data;
    }
    // write immediate to register B
    if (getControlBitDecode(instrDec,4)) {
        decodeStage->regB->in = getImmediateMType(instrDecHigh, instrDecLow);
    }
    if (getControlBitDecode(instrDec,3)) {
        decodeStage->regB->in = getImmediateIType(instrDecHigh, instrDecLow);
        decodeStage->regA->in = 0;
    }

    decodeStage->decodeExecute->instruction_High.in = instrDecHigh*!decodeStage->decodeDecode->stall;
    decodeStage->decodeExecute->instruction_High.write = true;
    decodeStage->decodeExecute->instruction_Low.in = instrDecLow*!decodeStage->decodeDecode->stall;
    decodeStage->decodeExecute->instruction_Low.write = true;

#ifdef CPU_DEBUG
    printf("Decode -- Instruction: %s\n",INSTR_STRINGS[instrDec]);
    printf("Decode -- Stall: %d\n",decodeStage->decodeDecode->stall);
    printf("Decode -- Source 1: %d, Source 2: %d\n",sourceReg1, sourceReg2);
#endif
}

void StageExecute_update(StageExecute* executeStage) { 
    uint8_t instrExecuteHigh = executeStage->decodeExecute->instruction_High.data;
    uint8_t instrExecuteLow = executeStage->decodeExecute->instruction_Low.data;
    uint8_t instrExecute = getInstruction(instrExecuteHigh, instrExecuteLow);
    uint8_t instrALU = getControlBitExecute(instrExecute,2) + 2*getControlBitExecute(instrExecute,3) + 4*getControlBitExecute(instrExecute,4);
    
    uint16_t out = 0;
    uint16_t MSB = executeStage->regA->data & 128;

    executeStage->flagZero->write = false;
    executeStage->flagNegative->write = false;
    executeStage->flagOverflow->write = false;
    switch (instrALU) {
        case 0: // ADD
            out = executeStage->regA->data + executeStage->regB->data;
            if (getControlBitExecute(instrExecute, 5)) updateFlags(executeStage, out);
            break;
        case 1: // SUB
            out = executeStage->regA->data - executeStage->regB->data;
            if (getControlBitExecute(instrExecute, 5)) updateFlags(executeStage, out);
            break;
        case 2: // AND
            out = executeStage->regA->data & executeStage->regB->data;
            if (getControlBitExecute(instrExecute, 5)) updateFlags(executeStage, out);
            break;
        case 3: // OR
            out = executeStage->regA->data | executeStage->regB->data;
            if (getControlBitExecute(instrExecute, 5)) updateFlags(executeStage, out);
            break;
        case 4: // XOR
            out = executeStage->regA->data ^ executeStage->regB->data;
            if (getControlBitExecute(instrExecute, 5)) updateFlags(executeStage, out);
            break;
        case 5: // LSL
            out = executeStage->regA->data << 1;
            if (getControlBitExecute(instrExecute, 5)) updateFlags(executeStage, out);
            break;
        case 6: // LSR
            out = executeStage->regA->data >> 1;
            if (getControlBitExecute(instrExecute, 5)) updateFlags(executeStage, out);
            break;
        case 7: // ASR
            out = executeStage->regA->data >> 1 | MSB;
            if (getControlBitExecute(instrExecute, 5)) updateFlags(executeStage, out);
            break;
    }

    executeStage->regOut->in = out;
    executeStage->regOut->write = true;

    bool C0 = getControlBitExecute(instrExecute, 6);
    bool C1 = getControlBitExecute(instrExecute, 7);
    bool condition = (C0 & !C1 & executeStage->flagZero->data) | (!C0  & C1 & executeStage->flagNegative->data) | (C0 & C1);
    executeStage->PC->in = condition ? out : executeStage->PC->in;
    *executeStage->branchPC = condition;

#ifdef CPU_DEBUG
    uint8_t destRegExecute = getDestRegister(instrExecuteHigh,instrExecuteLow);

    printf("Execute -- Instruction: %s\n",INSTR_STRINGS[instrExecute]);
    printf("Execute -- regA: %d; regB: %d; out: %d\n",executeStage->regA->data,executeStage->regB->data,out);
    printf("Execute -- Destination: %d\n",destRegExecute);
    printf("Execute -- Flag Zero: %d, Flag Negative: %d\n",executeStage->flagZero->data,executeStage->flagNegative->data);
    printf("Execute -- Branch: %d\n",*executeStage->branchPC);
#endif

    executeStage->decodeMemory->instruction_High.in = instrExecuteHigh;
    executeStage->decodeMemory->instruction_High.write = true;
    executeStage->decodeMemory->instruction_Low.in = instrExecuteLow;
    executeStage->decodeMemory->instruction_Low.write = true;
}

void StageMemory_update(StageMemory* memoryStage) {
    uint8_t instrMemHigh = memoryStage->decodeMemory->instruction_High.data;
    uint8_t instrMemLow = memoryStage->decodeMemory->instruction_Low.data;
    uint8_t instrMem = getInstruction(instrMemHigh, instrMemLow);

    uint8_t instrDecHigh = memoryStage->decodeDecode->instruction_High.data;
    uint8_t instrDecLow = memoryStage->decodeDecode->instruction_Low.data;
    uint8_t instrDec = getInstruction(instrDecHigh, instrDecLow);

    uint8_t regSource1 = getSource1Register(instrMemHigh, instrMemLow);
    uint8_t regDest = getDestRegister(instrMemHigh,instrMemLow);

    uint8_t decRegSource1 = getSource1Register(instrDecHigh,instrDecLow);
    uint8_t decRegSource2 = getSource2Register(instrDecHigh,instrDecLow);

    uint8_t in = memoryStage->regIn->data;
    uint8_t out = in;
    if (getControlBitMemory(instrMem, 2)) {
        out = memoryStage->memoryData->reg[in].data;
    }
    if (getControlBitMemory(instrMem, 3)) {
        memoryStage->memoryData->reg[in-DATA_MEM_OFFSET].in = memoryStage->writeRegister->reg[regSource1].data;
        memoryStage->memoryData->reg[in-DATA_MEM_OFFSET].write = true;
        Register_clock(&memoryStage->memoryData->reg[in]);
        memoryStage->memoryData->reg[in].write = false;
    }
    // enfore zero register
    if (decRegSource1 == regDest && getControlBitDecode(instrDec,1) && regDest != 0) {
        memoryStage->regA->in = out;
        memoryStage->regA->write = true;
    }
    if (decRegSource2 == regDest && getControlBitDecode(instrDec,2) && regDest != 0) {
        memoryStage->regB->in = out;
        memoryStage->regB->write = true;
    }

    memoryStage->regOut->in = out;
    memoryStage->regOut->write = true;

    memoryStage->decodeWriteback->instruction_High.in = instrMemHigh;
    memoryStage->decodeWriteback->instruction_High.write = true;
    memoryStage->decodeWriteback->instruction_Low.in = instrMemLow;
    memoryStage->decodeWriteback->instruction_Low.write = true;


#ifdef CPU_DEBUG
    printf("Memory -- Instruction: %s\n",INSTR_STRINGS[instrMem]);
#endif
}


void StageWriteback_update(StageWriteback* writebackStage) {
    uint8_t instrWriteHigh = writebackStage->decodeWriteback->instruction_High.data; 
    uint8_t instrWriteLow = writebackStage->decodeWriteback->instruction_Low.data; 
    uint8_t instrWrite = getInstruction(instrWriteHigh,instrWriteLow);

    uint8_t destReg = getDestRegister(instrWriteHigh, instrWriteLow);
    uint8_t in = writebackStage->regIn->data;

    bool write = getControlBitWriteback(instrWrite,2);

    writebackStage->regFileA->reg[destReg].in = in;
    writebackStage->regFileB->reg[destReg].in = in;
    writebackStage->regFileWrite->reg[destReg].in = in;
    writebackStage->regFileA->reg[destReg].write = write;
    writebackStage->regFileB->reg[destReg].write = write;
    writebackStage->regFileWrite->reg[destReg].write = write;

#ifdef CPU_DEBUG
    printf("\nNext cycle:\n");
    printf("Writeback -- Instruction: %s\n",INSTR_STRINGS[instrWrite]);
    printf("Writeback -- Destination: %d; Data: %d\n",destReg,in);
#endif
}


void StageFetch_clock(StageFetch* fetchStage) {
    Register_clock(&fetchStage->decodeDecode->instruction_Low);
    Register_clock(&fetchStage->decodeDecode->instruction_High);
    Register_clock(fetchStage->PC);
}

void StageDecode_clock(StageDecode* decodeStage) {
    Register_clock(decodeStage->regA);
    Register_clock(decodeStage->regB);
    Register_clock(&decodeStage->decodeExecute->instruction_High);
    Register_clock(&decodeStage->decodeExecute->instruction_Low);
}

void StageExecute_clock(StageExecute* executeStage) {
    Register_clock(executeStage->regOut);
    Register_clock(executeStage->flagZero);
    Register_clock(executeStage->flagNegative);
    Register_clock(executeStage->flagOverflow);
    Register_clock(&executeStage->decodeMemory->instruction_High);
    Register_clock(&executeStage->decodeMemory->instruction_Low);
}

void StageMemory_clock(StageMemory* memoryStage) {
   Register_clock(memoryStage->regOut);
   Register_clock(&memoryStage->decodeWriteback->instruction_High);
   Register_clock(&memoryStage->decodeWriteback->instruction_Low);
}

void StageWriteback_clock(StageWriteback* writebackStage) {
    uint8_t instrWriteHigh = writebackStage->decodeWriteback->instruction_High.data; 
    uint8_t instrWriteLow = writebackStage->decodeWriteback->instruction_Low.data; 
    uint8_t instrWrite = getInstruction(instrWriteHigh,instrWriteLow);

    uint8_t destReg = getDestRegister(instrWriteHigh, instrWriteLow);

    Register_clock(&writebackStage->regFileA->reg[destReg]);
    Register_clock(&writebackStage->regFileB->reg[destReg]);
    Register_clock(&writebackStage->regFileWrite->reg[destReg]);
}


/*
 * Static Functions
*/


static uint8_t getInstruction(uint8_t instrHigh, uint8_t instrLow) {
    return (instrHigh & 0b11110000) >> 4;
}

static uint8_t getSource1Register(uint8_t instrHigh, uint8_t instrLow) {
    return (instrHigh & 0b00000001) << 2 | (instrLow & 0b11000000) >> 6;
}

static uint8_t getDestRegister(uint8_t instrHigh, uint8_t instrLow) {
    return (instrHigh & 0b00001110) >> 1;
}

static uint8_t getSource2Register(uint8_t instrHigh, uint8_t instrLow) {
    return (instrLow & 0b00111000) >> 3;
}

static uint8_t getImmediateMType(uint8_t instrHigh, uint8_t instrLow) {
    return (instrLow & 0b00111111);
}

static uint8_t getImmediateIType(uint8_t instrHigh, uint8_t instrLow) {
    return (instrLow & 0b11111111);
}

static bool getControlBitDecode(uint8_t instr, uint8_t bitIndex) {
    return controlDecode[bitIndex + controlWidthDecode*instr];
}

static bool getControlBitExecute(uint8_t instr, uint8_t bitIndex) {
    return controlExecute[bitIndex + controlWidthExecute*instr];
}


static bool getControlBitMemory(uint8_t instr, uint8_t bitIndex) {
    return controlMemory[bitIndex + controlWidthMemory*instr];
}


static bool getControlBitWriteback(uint8_t instr, uint8_t bitIndex) {
    return controlWriteback[bitIndex + controlWidthWriteback*instr];
}

static void updateFlags(StageExecute* executeStage, uint16_t out) {
    executeStage->flagZero->in = out == 0;
    executeStage->flagZero->write = true;
    executeStage->flagNegative->in = out > 127;
    executeStage->flagNegative->write = true;
    executeStage->flagOverflow->in = out > 255;
    executeStage->flagOverflow->write = true;
}
