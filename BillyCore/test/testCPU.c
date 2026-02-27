#include "Component.h"
#include "CPU.h"
#include <stdint.h>
#include <stdlib.h>
#include "Instructions.h"
#include "Translation.h"

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

#define PRINT_ASSERTS

#ifdef PRINT_ASSERTS
#define PRINT(...) (printf(__VA_ARGS__))
#else
#define PRINT(...)
#endif

vango_test(Instruction_LDA) {
    uint16_t instr = 0b0001010011011010;
    uint16_t instr_auto = Instr_M("LDA", 2, 3, 26).instr;
    PRINT("Expected: %x, Actual: %x\n",instr, instr_auto);
    vg_assert_eq(instr, instr_auto);
}

vango_test(Instruction_STR) {
    uint16_t instr = 0b0010010011011010;
    uint16_t instr_auto = Instr_M("STR", 2, 3, 26).instr;
    PRINT("Expected: %x, Actual: %x\n",instr, instr_auto);
    vg_assert_eq(instr, instr_auto);
}

vango_test(Instruction_LDI) {
    uint16_t instr = 0b0011011000001111;
    uint16_t instr_auto = Instr_I("LDI", 3, 15).instr;
    PRINT("Expected: %x, Actual: %x\n",instr, instr_auto);
    vg_assert_eq(instr, instr_auto);
}

vango_test(Instruction_ADD) {
    uint16_t instr = 0b0100010101011000;
    uint16_t instr_auto = Instr_R("ADD", 2, 5, 3).instr;
    PRINT("Expected: %x, Actual: %x\n",instr, instr_auto);
    vg_assert_eq(instr, instr_auto);
}

vango_test(Instruction_SUB) {
    uint16_t instr = 0b0101010101011000;
    uint16_t instr_auto = Instr_R("SUB", 2, 5, 3).instr;
    PRINT("Expected: %x, Actual: %x\n",instr, instr_auto);
    vg_assert_eq(instr, instr_auto);
}

vango_test(Instruction_AND) {
    uint16_t instr = 0b0110010101011000;
    uint16_t instr_auto = Instr_R("AND", 2, 5, 3).instr;
    PRINT("Expected: %x, Actual: %x\n",instr, instr_auto);
    vg_assert_eq(instr, instr_auto);
}

vango_test(Instruction_OR) {
    uint16_t instr = 0b0111010101011000;
    uint16_t instr_auto = Instr_R("OR", 2, 5, 3).instr;
    PRINT("Expected: %x, Actual: %x\n",instr, instr_auto);
    vg_assert_eq(instr, instr_auto);
}

vango_test(Instruction_XOR) {
    uint16_t instr = 0b1000010101011000;
    uint16_t instr_auto = Instr_R("XOR", 2, 5, 3).instr;
    PRINT("Expected: %x, Actual: %x\n",instr, instr_auto);
    vg_assert_eq(instr, instr_auto);
}

vango_test(Instruction_LSL) {
    uint16_t instr = 0b1001010101011000;
    uint16_t instr_auto = Instr_R("LSL", 2, 5, 3).instr;
    PRINT("Expected: %x, Actual: %x\n",instr, instr_auto);
    vg_assert_eq(instr, instr_auto);
}

vango_test(Instruction_LSR) {
    uint16_t instr = 0b1010010101011000;
    uint16_t instr_auto = Instr_R("LSR", 2, 5, 3).instr;
    PRINT("Expected: %x, Actual: %x\n",instr, instr_auto);
    vg_assert_eq(instr, instr_auto);
}

vango_test(Instruction_ASR) {
    uint16_t instr = 0b1011010101011000;
    uint16_t instr_auto = Instr_R("ASR", 2, 5, 3).instr;
    PRINT("Expected: %x, Actual: %x\n",instr, instr_auto);
    vg_assert_eq(instr, instr_auto);
}

vango_test(Instruction_BEQ) {
    uint16_t instr = 0b1100100000010001;
    uint16_t instr_auto = Instr_I("BEQ", 4, 17).instr;
    PRINT("Expected: %x, Actual: %x\n",instr, instr_auto);
    vg_assert_eq(instr, instr_auto);
}

vango_test(Instruction_BLT) {
    uint16_t instr = 0b1101100000010001;
    uint16_t instr_auto = Instr_I("BLT", 4, 17).instr;
    PRINT("Expected: %x, Actual: %x\n",instr, instr_auto);
    vg_assert_eq(instr, instr_auto);
}

vango_test(Instruction_JMP) {
    uint16_t instr = 0b1110100000010001;
    uint16_t instr_auto = Instr_I("JMP", 4, 17).instr;
    PRINT("Expected: %x, Actual: %x\n",instr, instr_auto);
    vg_assert_eq(instr, instr_auto);
}


vango_test(instruction_forwarding) {
    CPU_Init();
    
    uint8_t* instr = calloc(sizeof(uint8_t),MEM_SIZE);
    instr[0] = Instr_I("LDI", 1, 26).high;
    instr[1] = Instr_I("LDI", 1, 26).low;
    instr[2] = Instr_I("LDI", 2, 14).high;
    instr[3] = Instr_I("LDI", 2, 14).low;
    instr[4] = Instr_R("ADD", 2, 2, 1).high;
    instr[5] = Instr_R("ADD", 2, 2, 1).low;
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
    PRINT("Assert -- Value: %d, Expected: %d\n",instrHigh,0b00110010);
    vg_assert_eq(instrHigh,0b00110010);
    PRINT("Assert -- Value: %d, Expected: %d\n",instrLow, 0b00011010);
    vg_assert_eq(instrLow, 0b00011010);

    CPU_Clock();
    // After 5 clocks: writeback stage holds LDI $r1, 14
    instrHigh = CPU_getStageWriteback()->decodeWriteback->instruction_High.data;
    instrLow = CPU_getStageWriteback()->decodeWriteback->instruction_Low.data;
    PRINT("Assert -- Value: %d, Expected: %d\n",instrHigh, 0b00110100);
    vg_assert_eq(instrHigh, 0b00110100);
    PRINT("Assert -- Value: %d, Expected: %d\n",instrLow, 0b00001110);
    vg_assert_eq(instrLow, 0b00001110);

    CPU_Clock();
    CPU_Clock();
    // After 6 clocks: writeback stage holds ADD $r2, $r2, $r1
    instrHigh = CPU_getStageWriteback()->decodeWriteback->instruction_High.data;
    instrLow = CPU_getStageWriteback()->decodeWriteback->instruction_Low.data;
    PRINT("Assert -- Value: %d, Expected: %d\n",instrHigh, 0b01000100);
    vg_assert_eq(instrHigh, 0b01000100);
    PRINT("Assert -- Value: %d, Expected: %d\n",instrLow, 0b10001000);
    vg_assert_eq(instrLow, 0b10001000);

}


vango_test(load_imm) {
    CPU_Init();
    
    uint8_t* instr = calloc(sizeof(uint8_t),MEM_SIZE);
    instr[0] = Instr_I("LDI", 1, 26).high;
    instr[1] = Instr_I("LDI", 1, 26).low;
    instr[2] = Instr_I("LDI", 2, 14).high;
    instr[3] = Instr_I("LDI", 2, 14).low;
    instr[4] = Instr_R("ADD", 2, 2, 1).high;
    instr[5] = Instr_R("ADD", 2, 2, 1).low;
    CPU_SetInstructionMemory(instr);

    free(instr);

    CPU_Clock();
    CPU_Clock();
    CPU_Clock();
    CPU_Clock(); // After 4 clocks: writeback stage holds LDI $r1, 14 
    CPU_Clock(); // After 5 clocks: writeback stage holds LDI $r2, 26

    uint8_t reg1 = CPU_getRegisterFile()->reg[1].data;
    // PRINT("Register 1 is: %d; Expected: %d\n",reg1, 26);
    PRINT("Assert -- Value: %d, Expected: %d\n",reg1,26);
    vg_assert_eq(reg1, 26);

    CPU_Clock();

    uint8_t reg2 = CPU_getRegisterFile()->reg[2].data;
    // PRINT("Register 2 is: %d; Expected: %d\n",reg2, 14);
    PRINT("Assert -- Value: %d, Expected: %d\n",reg2,14);
    vg_assert_eq(reg2, 14);

    CPU_Clock();
    CPU_Clock();
    reg2 = CPU_getRegisterFile()->reg[2].data;
    PRINT("Assert -- Value: %d, Expected: %d\n",reg2,40);
    vg_assert_eq(reg2, 40);
}

vango_test(basic_add) {
    CPU_Init();
    
    uint8_t* instr = calloc(sizeof(uint8_t),MEM_SIZE);
    instr[0] = Instr_I("LDI", 1, 26).high;
    instr[1] = Instr_I("LDI", 1, 26).low;
    instr[2] = Instr_I("LDI", 2, 14).high;
    instr[3] = Instr_I("LDI", 2, 14).low;
    instr[4] = Instr_R("ADD", 2, 2, 1).high;
    instr[5] = Instr_R("ADD", 2, 2, 1).low;
    CPU_SetInstructionMemory(instr);
    free(instr);

    for (int i=0; i<8; i++) {
        CPU_Clock();
    }

    uint8_t reg2 = CPU_getRegisterFile()->reg[2].data;
    PRINT("Assert -- Value: %d, Expected: %d\n",reg2,40);
    vg_assert_eq(reg2, 40);
}

vango_test(uncond_jump) {
    CPU_Init();
    
    uint8_t* instr = calloc(sizeof(uint8_t),MEM_SIZE);
    instr[0] = Instr_I("LDI", 1, 26).high;
    instr[1] = Instr_I("LDI", 1, 26).low;
    instr[2] = Instr_I("LDI", 2, 14).high;
    instr[3] = Instr_I("LDI", 2, 14).low;
    instr[4] = Instr_I("JMP", 0, 8).high;
    instr[5] = Instr_I("JMP", 0, 8).low;
    instr[6] = Instr_R("ADD", 2, 2, 1).high;
    instr[6] = Instr_R("ADD", 2, 2, 1).low;
    CPU_SetInstructionMemory(instr);

    free(instr);
    // PRINT("PC: %x\n",CPU_getPC()->data);
    for (int i=0; i<8; i++) {
        CPU_Clock();
        // PRINT("PC: %x\n",CPU_getPC()->data);
    }
    uint8_t reg2 = CPU_getRegisterFile()->reg[2].data;
    PRINT("Assert -- Value: %d, Expected: %d\n",reg2,14);
    vg_assert_eq(reg2, 14);
    uint8_t pc = CPU_getPC()->data;
    PRINT("Assert -- Value: %d, Expected: %d\n",pc,0xe);
    vg_assert_eq(pc, 0xe);
}


vango_test(beq_jump) {
    CPU_Init();
    
    uint8_t* instr = calloc(sizeof(uint8_t),MEM_SIZE);
    instr[0] = Instr_I("LDI", 1, 14).high;
    instr[1] = Instr_I("LDI", 1, 14).low;
    instr[2] = Instr_I("LDI", 2, 14).high;
    instr[3] = Instr_I("LDI", 2, 14).low;
    instr[4] = Instr_R("SUB", 2, 2, 1).high;
    instr[5] = Instr_R("SUB", 2, 2, 1).low;
    instr[6] = Instr_I("BEQ", 0, 12).high;
    instr[7] = Instr_I("BEQ", 0, 12).low;
    instr[8] = Instr_I("LDI", 2, 26).high;
    instr[9] = Instr_I("LDI", 2, 26).low;
    instr[10] = Instr_R("ADD", 2, 2, 1).high;
    instr[11] = Instr_R("ADD", 2, 2, 1).low;
    CPU_SetInstructionMemory(instr);

    free(instr);
    // PRINT("PC: %x\n",CPU_getPC()->data);
    for (int i=0; i<10; i++) {
        CPU_Clock();
        // PRINT("PC: %x\n",CPU_getPC()->data);
    }
    uint8_t reg2 = CPU_getRegisterFile()->reg[2].data;
    PRINT("Assert -- Value: %d, Expected: %d\n",reg2,0);
    vg_assert_eq(reg2, 0);
    uint8_t pc = CPU_getPC()->data;
    PRINT("Assert -- Value: %d, Expected: %d\n",pc,18);
    vg_assert_eq(pc, 18);
}


vango_test(beq_notjump) {
    CPU_Init();
    
    uint8_t* instr = calloc(sizeof(uint8_t),MEM_SIZE);
    instr[0] = Instr_I("LDI", 1, 10).high;
    instr[1] = Instr_I("LDI", 1, 10).low;
    instr[2] = Instr_I("LDI", 2, 14).high;
    instr[3] = Instr_I("LDI", 2, 14).low;
    instr[4] = Instr_R("SUB", 2, 2, 1).high;
    instr[5] = Instr_R("SUB", 2, 2, 1).low;
    instr[6] = Instr_I("BEQ", 0, 12).high;
    instr[7] = Instr_I("BEQ", 0, 12).low;
    instr[8] = Instr_I("LDI", 2, 26).high;
    instr[9] = Instr_I("LDI", 2, 26).low;
    instr[10] = Instr_R("ADD", 2, 2, 1).high;
    instr[11] = Instr_R("ADD", 2, 2, 1).low;
    CPU_SetInstructionMemory(instr);

    free(instr);
    // PRINT("PC: %x\n",CPU_getPC()->data);
    for (int i=0; i<14; i++) {
        CPU_Clock();
        // PRINT("PC: %x\n",CPU_getPC()->data);
    }
    uint8_t reg2 = CPU_getRegisterFile()->reg[2].data;
    PRINT("Assert -- Value: %d, Expected: %d\n",reg2,36);
    vg_assert_eq(reg2, 36);
    uint8_t pc = CPU_getPC()->data;
    PRINT("Assert -- Value: %d, Expected: %d\n",pc,14);
    vg_assert_eq(pc, 20);
}


vango_test(blt_jump) {
    CPU_Init();
    
    uint8_t* instr = calloc(sizeof(uint8_t),MEM_SIZE);
    instr[0] = Instr_I("LDI", 1, 15).high;
    instr[1] = Instr_I("LDI", 1, 15).low;
    instr[2] = Instr_I("LDI", 2, 14).high;
    instr[3] = Instr_I("LDI", 2, 14).low;
    instr[4] = Instr_R("SUB", 2, 2, 1).high;
    instr[5] = Instr_R("SUB", 2, 2, 1).low;
    instr[6] = Instr_I("BLT", 0, 12).high;
    instr[7] = Instr_I("BLT", 0, 12).low;
    instr[8] = Instr_I("LDI", 2, 26).high;
    instr[9] = Instr_I("LDI", 2, 26).low;
    instr[10] = Instr_R("ADD", 2, 2, 1).high;
    instr[11] = Instr_R("ADD", 2, 2, 1).low;
    CPU_SetInstructionMemory(instr);

    free(instr);
    // PRINT("PC: %x\n",CPU_getPC()->data);
    for (int i=0; i<10; i++) {
        CPU_Clock();
        // PRINT("PC: %x\n",CPU_getPC()->data);
    }
    uint8_t reg2 = CPU_getRegisterFile()->reg[2].data;
    PRINT("Assert -- Value: %d, Expected: %d\n",reg2,255);
    vg_assert_eq(reg2, 255);
    uint8_t pc = CPU_getPC()->data;
    PRINT("Assert -- Value: %d, Expected: %d\n",pc,18);
    vg_assert_eq(pc, 18);
}


vango_test(blt_notjump) {
    CPU_Init();
    
    uint8_t* instr = calloc(sizeof(uint8_t),MEM_SIZE);
    instr[0] = Instr_I("LDI", 1, 14).high;
    instr[1] = Instr_I("LDI", 1, 14).low;
    instr[2] = Instr_I("LDI", 2, 14).high;
    instr[3] = Instr_I("LDI", 2, 14).low;
    instr[4] = Instr_R("SUB", 2, 2, 1).high;
    instr[5] = Instr_R("SUB", 2, 2, 1).low;
    instr[6] = Instr_I("BLT", 0, 12).high;
    instr[7] = Instr_I("BLT", 0, 12).low;
    instr[8] = Instr_I("LDI", 2, 26).high;
    instr[9] = Instr_I("LDI", 2, 26).low;
    instr[10] = Instr_R("ADD", 2, 2, 1).high;
    instr[11] = Instr_R("ADD", 2, 2, 1).low;
    CPU_SetInstructionMemory(instr);

    free(instr);
    // PRINT("PC: %x\n",CPU_getPC()->data);
    for (int i=0; i<14; i++) {
        CPU_Clock();
        // PRINT("PC: %x\n",CPU_getPC()->data);
    }
    uint8_t reg2 = CPU_getRegisterFile()->reg[2].data;
    PRINT("Assert -- Value: %d, Expected: %d\n",reg2,40);
    vg_assert_eq(reg2, 40);
    uint8_t pc = CPU_getPC()->data;
    PRINT("Assert -- Value: %d, Expected: %d\n",pc,14);
    vg_assert_eq(pc, 20);
}

vango_test(translate_tokenize_LDA) {
    char* line = "  LDA     %r5, %r1, 0x2f";
    Token* tokens = Translation_tokenize(line);
    Token tokens_exp[] = {
        (Token){.type=INSTR,.name="LDA"},
        (Token){.type=REG,.reg=5},
        (Token){.type=REG,.reg=1},
        (Token){.type=IMM,.immediate=0x2f},
        (Token){.type=END},
    };
    for (int i=0; i<5; i++) {
        char token_str[50];
        Translation_token_to_str(&tokens[i],token_str);
        PRINT("%s\n",token_str);
        vg_assert(Translation_token_cmpeq(&tokens[i],&tokens_exp[i]));
    }
};

vango_test(translate_tokenize_LDI) {
    char* line = "  LDI     %r5, 0x2f";
    Token* tokens = Translation_tokenize(line);
    Token tokens_exp[] = {
        (Token){.type=INSTR,.name="LDI"},
        (Token){.type=REG,.reg=5},
        (Token){.type=IMM,.immediate=0x2f},
        (Token){.type=END},
    };
    for (int i=0; i<4; i++) {
        char token_str[50];
        Translation_token_to_str(&tokens[i],token_str);
        PRINT("%s\n",token_str);
        vg_assert(Translation_token_cmpeq(&tokens[i],&tokens_exp[i]));
    }
};

vango_test(translate_tokenize_ADD) {
    char* line = "  ADD     %r5, %r1, %r2";
    Token* tokens = Translation_tokenize(line);
    Token tokens_exp[] = {
        (Token){.type=INSTR,.name="ADD"},
        (Token){.type=REG,.reg=5},
        (Token){.type=REG,.reg=1},
        (Token){.type=REG,.reg=2},
        (Token){.type=END},
    };
    for (int i=0; i<5; i++) {
        char token_str[50];
        Translation_token_to_str(&tokens[i],token_str);
        PRINT("%s\n",token_str);
        vg_assert(Translation_token_cmpeq(&tokens[i],&tokens_exp[i]));
    }
};

vango_test(translate_tokenize_BEQ) {
    char* line = "  BEQ     ABC";
    Token* tokens = Translation_tokenize(line);
    Token tokens_exp[] = {
        (Token){.type=INSTR,.name="BEQ"},
        (Token){.type=LABEL,.name="ABC"},
        (Token){.type=END},
    };
    for (int i=0; i<3; i++) {
        char token_str[50];
        Translation_token_to_str(&tokens[i],token_str);
        PRINT("%s\n",token_str);
        vg_assert(Translation_token_cmpeq(&tokens[i],&tokens_exp[i]));
    }
};

vango_test(translate_tokenize_BLT) {
    char* line = "  BLT     0xe5";
    Token* tokens = Translation_tokenize(line);
    Token tokens_exp[] = {
        (Token){.type=INSTR,.name="BLT"},
        (Token){.type=IMM,.immediate=0xe5},
        (Token){.type=END},
    };
    for (int i=0; i<3; i++) {
        char token_str[50];
        Translation_token_to_str(&tokens[i],token_str);
        PRINT("%s\n",token_str);
        vg_assert(Translation_token_cmpeq(&tokens[i],&tokens_exp[i]));
    }
};

vango_test(translate_medium_test) {
    uint8_t* instr = calloc(sizeof(uint8_t),MEM_SIZE);
    instr[0] = Instr_I("LDI", 1, 14).high;
    instr[1] = Instr_I("LDI", 1, 14).low;
    instr[2] = Instr_I("LDI", 2, 14).high;
    instr[3] = Instr_I("LDI", 2, 14).low;
    instr[4] = Instr_R("SUB", 2, 2, 1).high;
    instr[5] = Instr_R("SUB", 2, 2, 1).low;
    instr[6] = Instr_I("BEQ", 0, 12).high;
    instr[7] = Instr_I("BEQ", 0, 12).low;
    instr[8] = Instr_I("LDI", 2, 26).high;
    instr[9] = Instr_I("LDI", 2, 26).low;
    instr[10] = Instr_R("ADD", 2, 2, 1).high;
    instr[11] = Instr_R("ADD", 2, 2, 1).low;

    char* assembly = 
        "   LDI %r1, 14\n"
        "   LDI %r2, 14\n"
        "   SUB %r2, %r2, %r1\n"
        "   BEQ 12\n"
        "   LDI %r2, 26\n"
        "   ADD %r2, %r2, %r1\n"
        "";
    ByteCode bc = Translation_translate_str(assembly);

    vg_assert_eq(bc.num_instr,6);
    for (int i=0; i < bc.num_instr; i++) {
        PRINT("(%d) -- Instruction: 0x%x\n(%d) -- Expected   : 0x%x\n",i,bc.instr[i].instr,i, ((uint16_t*)instr)[i]);
        vg_assert_eq(bc.instr[i].instr, ((uint16_t*)instr)[i]);
    }

    free(bc.instr);
    free(instr);
};

vango_test(translate_medium_label) {
    uint8_t* instr = calloc(sizeof(uint8_t),MEM_SIZE);
    instr[0] = Instr_I("LDI", 1, 14).high;
    instr[1] = Instr_I("LDI", 1, 14).low;
    instr[2] = Instr_I("LDI", 2, 14).high;
    instr[3] = Instr_I("LDI", 2, 14).low;
    instr[4] = Instr_R("SUB", 2, 2, 1).high;
    instr[5] = Instr_R("SUB", 2, 2, 1).low;
    instr[6] = Instr_I("BEQ", 0, 12).high;
    instr[7] = Instr_I("BEQ", 0, 12).low;
    instr[8] = Instr_I("LDI", 2, 26).high;
    instr[9] = Instr_I("LDI", 2, 26).low;
    instr[10] = Instr_R("ADD", 2, 2, 1).high;
    instr[11] = Instr_R("ADD", 2, 2, 1).low;

    char* assembly = 
        "   LDI %r1, 14\n"
        "   LDI %r2, 14\n"
        "   SUB %r2, %r2, %r1\n"
        "   BEQ END\n"
        "   LDI %r2, 26\n"
        "   ADD %r2, %r2, %r1\n"
        ".END:\n"
        "";
    ByteCode bc = Translation_translate_str(assembly);

    vg_assert_eq(bc.num_instr,6);
    for (int i=0; i < bc.num_instr; i++) {
        PRINT("(%d) -- Instruction: 0x%x\n(%d) -- Expected   : 0x%x\n",i,bc.instr[i].instr,i, ((uint16_t*)instr)[i]);
        vg_assert_eq(bc.instr[i].instr, ((uint16_t*)instr)[i]);
    }

    free(bc.instr);
    free(instr);
};

vango_test_main(
        vango_test_reg(Instruction_LDA);
        vango_test_reg(Instruction_STR);
        vango_test_reg(Instruction_LDI);
        vango_test_reg(Instruction_ADD);
        vango_test_reg(Instruction_SUB);
        vango_test_reg(Instruction_AND);
        vango_test_reg(Instruction_OR);
        vango_test_reg(Instruction_XOR);
        vango_test_reg(Instruction_LSL);
        vango_test_reg(Instruction_LSR);
        vango_test_reg(Instruction_ASR);
        vango_test_reg(Instruction_BEQ);
        vango_test_reg(Instruction_BLT);
        vango_test_reg(Instruction_JMP);
        vango_test_reg(instruction_forwarding);
        vango_test_reg(load_imm);
        vango_test_reg(basic_add);
        vango_test_reg(uncond_jump);
        vango_test_reg(beq_jump);
        vango_test_reg(beq_notjump);
        vango_test_reg(blt_jump);
        vango_test_reg(blt_notjump);
        vango_test_reg(translate_tokenize_LDA);
        vango_test_reg(translate_tokenize_LDI);
        vango_test_reg(translate_tokenize_ADD);
        vango_test_reg(translate_tokenize_BEQ);
        vango_test_reg(translate_tokenize_BLT);
        vango_test_reg(translate_medium_test);
        vango_test_reg(translate_medium_label);
)
