#include "Component.h"
#include <stdint.h>
#define VANGO_TEST_ROOT
#include <vangotest/casserts.h>

#include "CPU.h"


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

vango_test(basic_math) {
    int a = 10;
    vg_assert_eq(a, 10);
}

vango_test(instruction_forwarding) {
    CPU_Init();
    
    uint8_t* instr = malloc(sizeof(uint8_t)*MEM_SIZE);
    memset(instr,0, sizeof(uint8_t)*MEM_SIZE);
    instr[0] = 0b00110010; // LDI $r2, 26
    instr[1] = 0b00011010;
    instr[2] = 0b00110001; // LDI $r1, 14
    instr[3] = 0b00001110;
    instr[4] = 0b01000010; // ADD $r2, $r2, $r1
    instr[5] = 0b00100001;
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
    printf("Instruction is: "BYTE_TO_BIN_PATTERN"; Expected: "BYTE_TO_BIN_PATTERN"\n",BYTE_TO_BIN(instrHigh),BYTE_TO_BIN(0b00110010));
    vg_assert_eq(instrHigh, 0b00110010);
    printf("Instruction is: "BYTE_TO_BIN_PATTERN"; Expected: "BYTE_TO_BIN_PATTERN"\n",BYTE_TO_BIN(instrLow),BYTE_TO_BIN(0b00011010));
    vg_assert_eq(instrLow, 0b00011010);
    CPU_Clock();
    // After 5 clocks: writeback stage holds LDI $r1, 14
    instrHigh = CPU_getStageWriteback()->decodeWriteback->instruction_High.data;
    instrLow = CPU_getStageWriteback()->decodeWriteback->instruction_Low.data;
    printf("Instruction is: "BYTE_TO_BIN_PATTERN"; Expected: "BYTE_TO_BIN_PATTERN"\n",BYTE_TO_BIN(instrHigh),BYTE_TO_BIN(0b00110001));
    vg_assert_eq(instrHigh, 0b00110001);
    printf("Instruction is: "BYTE_TO_BIN_PATTERN"; Expected: "BYTE_TO_BIN_PATTERN"\n",BYTE_TO_BIN(instrLow),BYTE_TO_BIN(0b00001110));
    vg_assert_eq(instrLow, 0b00001110);

    CPU_Clock();
    // After 6 clocks: writeback stage holds ADD $r2, $r2, $r1
    instrHigh = CPU_getStageWriteback()->decodeWriteback->instruction_High.data;
    instrLow = CPU_getStageWriteback()->decodeWriteback->instruction_Low.data;
    printf("Instruction is: "BYTE_TO_BIN_PATTERN"; Expected: "BYTE_TO_BIN_PATTERN"\n",BYTE_TO_BIN(instrHigh),BYTE_TO_BIN(0b01000010));
    vg_assert_eq(instrHigh, 0b01000010);
    printf("Instruction is: "BYTE_TO_BIN_PATTERN"; Expected: "BYTE_TO_BIN_PATTERN"\n",BYTE_TO_BIN(instrLow),BYTE_TO_BIN(0b00100001));
    vg_assert_eq(instrLow, 0b00100001);

}

vango_test_main(
        vango_test_reg(basic_math);
        vango_test_reg(instruction_forwarding);
)
