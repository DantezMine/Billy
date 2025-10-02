#include "Component.h"
#include "CPU.h"
#include <stdint.h>
#include <stdlib.h>
#define VANGO_TEST_ROOT
#include <vangotest/casserts.h>


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

vango_test(instruction_forwarding) {
    CPU_Init();
    
    uint8_t* instr = malloc(sizeof(uint8_t)*MEM_SIZE);
    memset(instr,0, sizeof(uint8_t)*MEM_SIZE);
    instr[0] = 0b00110010; // LDI $r1, 26
    instr[1] = 0b00011010;
    instr[2] = 0b00110100; // LDI $r2, 14
    instr[3] = 0b00001110;
    instr[4] = 0b01000100; // ADD $r2, $r2, $r1
    instr[5] = 0b10001000;
    CPU_SetInstructionMemory(instr);

    free(instr);

    uint8_t instrHigh;
    uint8_t instrLow;

    CPU_Clock();
    CPU_Clock();
    CPU_Clock();
    CPU_Clock();

    // After 4 clocks: writeback stage holds LDI $r2, 26
    instrHigh = CPU_getStageWriteback()->decodeWriteback->instruction_High.data;
    instrLow = CPU_getStageWriteback()->decodeWriteback->instruction_Low.data;
    printf("Assert -- Value: %d, Expected: %d\n",instrHigh,0b00110010);
    vg_assert_eq(instrHigh,0b00110010);
    printf("Assert -- Value: %d, Expected: %d\n",instrLow, 0b00011010);
    vg_assert_eq(instrLow, 0b00011010);

    CPU_Clock();
    // After 5 clocks: writeback stage holds LDI $r1, 14
    instrHigh = CPU_getStageWriteback()->decodeWriteback->instruction_High.data;
    instrLow = CPU_getStageWriteback()->decodeWriteback->instruction_Low.data;
    printf("Assert -- Value: %d, Expected: %d\n",instrHigh, 0b00110100);
    vg_assert_eq(instrHigh, 0b00110100);
    printf("Assert -- Value: %d, Expected: %d\n",instrLow, 0b00001110);
    vg_assert_eq(instrLow, 0b00001110);

    CPU_Clock();
    CPU_Clock();
    // After 6 clocks: writeback stage holds ADD $r2, $r2, $r1
    instrHigh = CPU_getStageWriteback()->decodeWriteback->instruction_High.data;
    instrLow = CPU_getStageWriteback()->decodeWriteback->instruction_Low.data;
    printf("Assert -- Value: %d, Expected: %d\n",instrHigh, 0b01000100);
    vg_assert_eq(instrHigh, 0b01000100);
    printf("Assert -- Value: %d, Expected: %d\n",instrLow, 0b10001000);
    vg_assert_eq(instrLow, 0b10001000);

}


vango_test(load_imm) {
    CPU_Init();
    
    uint8_t* instr = malloc(sizeof(uint8_t)*MEM_SIZE);
    memset(instr,0, sizeof(uint8_t)*MEM_SIZE);
    instr[0] = 0b00110010; // LDI $r1, 26
    instr[1] = 0b00011010;
    instr[2] = 0b00110100; // LDI $r2, 14
    instr[3] = 0b00001110;
    instr[4] = 0b01000100; // ADD $r2, $r2, $r1
    instr[5] = 0b10001000;
    CPU_SetInstructionMemory(instr);

    free(instr);

    CPU_Clock();
    CPU_Clock();
    CPU_Clock();
    CPU_Clock(); // After 4 clocks: writeback stage holds LDI $r1, 14 
    CPU_Clock(); // After 5 clocks: writeback stage holds LDI $r2, 26

    uint8_t reg1 = CPU_getRegisterFile()->reg[1].data;
    // printf("Register 1 is: %d; Expected: %d\n",reg1, 26);
    printf("Assert -- Value: %d, Expected: %d\n",reg1,26);
    vg_assert_eq(reg1, 26);

    CPU_Clock();

    uint8_t reg2 = CPU_getRegisterFile()->reg[2].data;
    // printf("Register 2 is: %d; Expected: %d\n",reg2, 14);
    printf("Assert -- Value: %d, Expected: %d\n",reg2,14);
    vg_assert_eq(reg2, 14);

    CPU_Clock();
    CPU_Clock();
    reg2 = CPU_getRegisterFile()->reg[2].data;
    printf("Assert -- Value: %d, Expected: %d\n",reg2,40);
    vg_assert_eq(reg2, 40);
}

vango_test(uncond_jump) {
    CPU_Init();
    
    uint8_t* instr = malloc(sizeof(uint8_t)*MEM_SIZE);
    memset(instr,0, sizeof(uint8_t)*MEM_SIZE);
    instr[0] = 0b00110010; // LDI $r1, 26
    instr[1] = 0b00011010;
    instr[2] = 0b00110100; // LDI $r2, 14
    instr[3] = 0b00001110;
    instr[4] = 0b11100000; // JMP 8($r0)
    instr[5] = 0b00001000;
    instr[6] = 0b01000100; // ADD $r2, $r2, $r1
    instr[7] = 0b10001000;
    CPU_SetInstructionMemory(instr);

    free(instr);
    // printf("PC: %x\n",CPU_getPC()->data);
    for (int i=0; i<8; i++) {
        CPU_Clock();
        // printf("PC: %x\n",CPU_getPC()->data);
    }
    uint8_t reg2 = CPU_getRegisterFile()->reg[2].data;
    printf("Assert -- Value: %d, Expected: %d\n",reg2,14);
    vg_assert_eq(reg2, 14);
    uint8_t pc = CPU_getPC()->data;
    printf("Assert -- Value: %d, Expected: %d\n",pc,0xe);
    vg_assert_eq(pc, 0xe);
}


vango_test(beq_jump) {
    CPU_Init();
    
    uint8_t* instr = malloc(sizeof(uint8_t)*MEM_SIZE);
    memset(instr,0, sizeof(uint8_t)*MEM_SIZE);
    instr[0] = 0b00110010; // LDI $r1, 14
    instr[1] = 0b00001110;
    instr[2] = 0b00110100; // LDI $r2, 14
    instr[3] = 0b00001110;
    instr[4] = 0b01010100; // SUB $r2, $r2, $r1
    instr[5] = 0b10001000;
    instr[6] = 0b11000000; // BEQ 12($r0)
    instr[7] = 0b00001100;
    instr[8] = 0b00110100; // LDI $r2, 26
    instr[9] = 0b00011010;
    instr[10] = 0b01000100; // ADD $r2, $r2, $r1
    instr[11] = 0b10001000;
    CPU_SetInstructionMemory(instr);

    free(instr);
    // printf("PC: %x\n",CPU_getPC()->data);
    for (int i=0; i<10; i++) {
        CPU_Clock();
        // printf("PC: %x\n",CPU_getPC()->data);
    }
    uint8_t reg2 = CPU_getRegisterFile()->reg[2].data;
    printf("Assert -- Value: %d, Expected: %d\n",reg2,0);
    vg_assert_eq(reg2, 0);
    uint8_t pc = CPU_getPC()->data;
    printf("Assert -- Value: %d, Expected: %d\n",pc,18);
    vg_assert_eq(pc, 18);
}


vango_test(beq_notjump) {
    CPU_Init();
    
    uint8_t* instr = malloc(sizeof(uint8_t)*MEM_SIZE);
    memset(instr,0, sizeof(uint8_t)*MEM_SIZE);
    instr[0] = 0b00110010; // LDI $r1, 10
    instr[1] = 0b00001010;
    instr[2] = 0b00110100; // LDI $r2, 14
    instr[3] = 0b00001110;
    instr[4] = 0b01010100; // SUB $r2, $r2, $r1
    instr[5] = 0b10001000;
    instr[6] = 0b11000000; // BEQ 12($r0)
    instr[7] = 0b00001100;
    instr[8] = 0b00110100; // LDI $r2, 26
    instr[9] = 0b00011010;
    instr[10] = 0b01000100; // ADD $r2, $r2, $r1
    instr[11] = 0b10001000;
    CPU_SetInstructionMemory(instr);

    free(instr);
    // printf("PC: %x\n",CPU_getPC()->data);
    for (int i=0; i<14; i++) {
        CPU_Clock();
        // printf("PC: %x\n",CPU_getPC()->data);
    }
    uint8_t reg2 = CPU_getRegisterFile()->reg[2].data;
    printf("Assert -- Value: %d, Expected: %d\n",reg2,36);
    vg_assert_eq(reg2, 36);
    uint8_t pc = CPU_getPC()->data;
    printf("Assert -- Value: %d, Expected: %d\n",pc,14);
    vg_assert_eq(pc, 20);
}


vango_test(blt_jump) {
    CPU_Init();
    
    uint8_t* instr = malloc(sizeof(uint8_t)*MEM_SIZE);
    memset(instr,0, sizeof(uint8_t)*MEM_SIZE);
    instr[0] = 0b00110010; // LDI $r1, 15
    instr[1] = 0b00001111;
    instr[2] = 0b00110100; // LDI $r2, 14
    instr[3] = 0b00001110;
    instr[4] = 0b01010100; // SUB $r2, $r2, $r1
    instr[5] = 0b10001000;
    instr[6] = 0b11010000; // BLT 12($r0)
    instr[7] = 0b00001100;
    instr[8] = 0b00110100; // LDI $r2, 26
    instr[9] = 0b00011010;
    instr[10] = 0b01000100; // ADD $r2, $r2, $r1
    instr[11] = 0b10001000;
    CPU_SetInstructionMemory(instr);

    free(instr);
    // printf("PC: %x\n",CPU_getPC()->data);
    for (int i=0; i<10; i++) {
        CPU_Clock();
        // printf("PC: %x\n",CPU_getPC()->data);
    }
    uint8_t reg2 = CPU_getRegisterFile()->reg[2].data;
    printf("Assert -- Value: %d, Expected: %d\n",reg2,255);
    vg_assert_eq(reg2, 255);
    uint8_t pc = CPU_getPC()->data;
    printf("Assert -- Value: %d, Expected: %d\n",pc,18);
    vg_assert_eq(pc, 18);
}


vango_test(blt_notjump) {
    CPU_Init();
    
    uint8_t* instr = malloc(sizeof(uint8_t)*MEM_SIZE);
    memset(instr,0, sizeof(uint8_t)*MEM_SIZE);
    instr[0] = 0b00110010; // LDI $r1, 14
    instr[1] = 0b00001110;
    instr[2] = 0b00110100; // LDI $r2, 14
    instr[3] = 0b00001110;
    instr[4] = 0b01010100; // SUB $r2, $r2, $r1
    instr[5] = 0b10001000;
    instr[6] = 0b11010000; // BLT 12($r0)
    instr[7] = 0b00001100;
    instr[8] = 0b00110100; // LDI $r2, 26
    instr[9] = 0b00011010;
    instr[10] = 0b01000100; // ADD $r2, $r2, $r1
    instr[11] = 0b10001000;
    CPU_SetInstructionMemory(instr);

    free(instr);
    // printf("PC: %x\n",CPU_getPC()->data);
    for (int i=0; i<14; i++) {
        CPU_Clock();
        // printf("PC: %x\n",CPU_getPC()->data);
    }
    uint8_t reg2 = CPU_getRegisterFile()->reg[2].data;
    printf("Assert -- Value: %d, Expected: %d\n",reg2,40);
    vg_assert_eq(reg2, 40);
    uint8_t pc = CPU_getPC()->data;
    printf("Assert -- Value: %d, Expected: %d\n",pc,14);
    vg_assert_eq(pc, 20);
}

vango_test_main(
        vango_test_reg(instruction_forwarding);
        vango_test_reg(load_imm);
        vango_test_reg(uncond_jump);
        vango_test_reg(beq_jump);
        vango_test_reg(beq_notjump);
        vango_test_reg(blt_jump);
        vango_test_reg(blt_notjump);
)
