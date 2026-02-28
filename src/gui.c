#include "gui.h"
#include "CPU.h"
#include "Translation.h"
#include "utils/vec2.h"
#include <malloc.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static sfView* guiview;

static sfFont* main_font;
static sfRectangleShape* rect;
static sfText* text;

static GUI_Container elements[18];

static void setup_elements();
static void update_elements();
static void draw_elements(sfRenderWindow* window);
static sfVector2f compute_position(GUI_Container* elem);
static void position_DFS(int curr, char* visited);
static int memory_offset_data(int stack_pointer);
static int memory_offset_instr(int pc);

typedef enum {
    F,D,E,M,W,
} stage_enum;

static uint16_t instr_history[5][3] = {
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
};


sfView* gui_init(sfRenderWindow* window) {
    sfVector2u window_size = sfRenderWindow_getSize(window);
    guiview = sfView_createFromRect((sfFloatRect){ 0, 0, window_size.x, window_size.y });
    sfRenderWindow_setView(window,guiview);

    main_font = sfFont_createFromFile("res/LiterationMonoNerdFontMono-Bold.ttf");

    rect = sfRectangleShape_create();
    sfRectangleShape_setFillColor(rect, COLOR_FILL);
    sfRectangleShape_setOutlineColor(rect, COLOR_BORDER);
    sfRectangleShape_setOutlineThickness(rect, -BORDER_THICKNESS);


    text = sfText_create();
    sfText_setColor(text, COLOR_TEXT);
    sfText_setFont(text, main_font);
    sfText_setCharacterSize(text, TEXT_SIZE);
    sfText_setStyle(text, sfTextRegular);

    setup_elements();

    char visited[18] = {};
    for (int i=0; i<18; i++) {
        if (!visited[i] && elements[i].parent == -1) {
            position_DFS(i, visited);
        }
    }

    return guiview;
}

void gui_clock() {
    for (int stage = 0; stage<5; stage++) {
        for (int level = 2; level>0; level--) {
            instr_history[stage][level] = instr_history[stage][level-1];
        }
    }
    instr_history[F][0] = CPU_getStageFetchInstr();
    instr_history[D][0] = CPU_getStageDecodeInstr();
    instr_history[E][0] = CPU_getStageExecuteInstr();
    instr_history[M][0] = CPU_getStageMemoryInstr();
    instr_history[W][0] = CPU_getStageWritebackInstr();
}

void gui_update(sfRenderWindow* window) {
    sfRenderWindow_setView(window, guiview);
    update_elements();
    draw_elements(window); 
}


void gui_close() {
    sfView_destroy(guiview);
    sfRectangleShape_destroy(rect);
    sfText_destroy(text);
    sfFont_destroy(main_font);
    for (int i=0; i<18; i++) {
        free(elements[i].text);
    }
}


static void update_elements() {
    RegisterFile* regs = CPU_getRegisterFile();
    const char* reg_string = 
                    "rax : 0x%2x  / %3u  / %4d\n"
                    "rbx : 0x%2x  / %3u  / %4d\n"
                    "rcx : 0x%2x  / %3u  / %4d\n"
                    "rdx : 0x%2x  / %3u  / %4d\n"
                    "rex : 0x%2x  / %3u  / %4d\n"
                    "rsp : 0x%2x  / %3u\n"
                    "rbp : 0x%2x  / %3u\n"
                    " pc : 0x%2x  / %3u\n";
    int mem_offset = 0;
    char instr_disas[50];
    char instr_line[50];
    for (int i=0; i<18; i++) {
        switch (elements[i].content_type) {
            case GUI_INSTR:
                Translation_instr_to_str(
                        instr_history[elements[i].stage][elements[i].level],
                        elements[i].text, 100);
                break;
            case GUI_REGISTERS:
                sprintf_s(elements[i].text,500, reg_string,
                        regs->reg[1].data,regs->reg[1].data,regs->reg[1].data,
                        regs->reg[2].data,regs->reg[2].data,regs->reg[2].data,
                        regs->reg[3].data,regs->reg[3].data,regs->reg[3].data,
                        regs->reg[4].data,regs->reg[4].data,regs->reg[4].data,
                        regs->reg[5].data,regs->reg[5].data,regs->reg[5].data,
                        regs->reg[6].data,regs->reg[6].data,
                        regs->reg[7].data,regs->reg[7].data,
                        CPU_getPC()->data,CPU_getPC()->data
                        );
                break;
            case GUI_INSTRUCIONS:
                elements[i].text[0] = 0;
                int pc = CPU_getPC()->data;
                mem_offset = memory_offset_instr(pc);
                for (int k=0; k<18; k++) {
                    Translation_instr_to_str(CPU_PeekInstructionMemory16(mem_offset+k*2), instr_disas, 50);
                    if (pc == mem_offset+k*2) {
                        sprintf_s(instr_line,50,"pc -> 0x%2x : %2x %2x  # %s\n",
                                mem_offset+k*2,
                                CPU_PeekInstructionMemory(mem_offset+k*2),
                                CPU_PeekInstructionMemory(mem_offset+k*2+1),
                                instr_disas);
                        strcat_s(elements[i].text,1000,instr_line);
                    } else {
                        sprintf_s(instr_line,50,"      0x%2x : %2x %2x  # %s\n",
                                mem_offset+k*2,
                                CPU_PeekInstructionMemory(mem_offset+k*2),
                                CPU_PeekInstructionMemory(mem_offset+k*2+1),
                                instr_disas);
                        strcat_s(elements[i].text,1000,instr_line);
                    }
                }
                break;
            case GUI_DATA:
                elements[i].text[0] = 0;
                int stack_pointer = CPU_getRegisterFile()->reg[6].data;
                mem_offset = memory_offset_data(stack_pointer);
                mem_offset -= fmod(mem_offset,4); // four byte aligned
                for (int k=0; k<18; k++) {
                    if (stack_pointer >= mem_offset-k*4 && stack_pointer <= mem_offset-k*4+3) {
                        sprintf_s(instr_line,50,"rsp -> 0x%2x : %2x %2x %2x %2x\n",
                                mem_offset-k*2,
                                CPU_PeekDataMemory(mem_offset-k*4),
                                CPU_PeekDataMemory(mem_offset-k*4+1),
                                CPU_PeekDataMemory(mem_offset-k*4+2),
                                CPU_PeekDataMemory(mem_offset-k*4+3));
                    } else {
                        sprintf_s(instr_line,50,"       0x%2x : %2x %2x %2x %2x\n",
                                mem_offset-k*2,
                                CPU_PeekDataMemory(mem_offset-k*4),
                                CPU_PeekDataMemory(mem_offset-k*4+1),
                                CPU_PeekDataMemory(mem_offset-k*4+2),
                                CPU_PeekDataMemory(mem_offset-k*4+3));
                    }
                    strcat_s(elements[i].text,1000,instr_line);
                }
                break;
            default:
                break;
        }
    }
}


static int memory_offset_data(int stack_pointer) {
    return fmin(252,fmax(stack_pointer+8,164));
}
static int memory_offset_instr(int pc) {
    return fmax(0,fmin(pc-4,190));
}

static void draw_elements(sfRenderWindow* window) {
    sfVector2f gui_scale = sfView_getSize(guiview);
    sfRectangleShape_setScale(rect, gui_scale);
    sfText_setString(text, "_");
    sfFloatRect char_bounds =  sfText_getLocalBounds(text);
    float char_width = char_bounds.width;

    for (int i=0; i<18; i++) {
        sfRectangleShape_setOrigin(rect, vec2f_mul(elements[i].computed_pos,-1));
        sfRectangleShape_setSize(rect, elements[i].size);


        sfText_setString(text, elements[i].text);
        sfFloatRect txt_bounds = sfText_getGlobalBounds(text);
        sfFloatRect rect_bounds = sfRectangleShape_getGlobalBounds(rect);
        sfVector2f text_offset;
        switch(elements[i].content_type) {
        case GUI_INSTR:
            text_offset = (sfVector2f) {char_width, rect_bounds.height/2-txt_bounds.height/1.5};
            sfText_setPosition(text, vec2f_add(vec2f_scale(elements[i].computed_pos,gui_scale),text_offset));
            break;
        case GUI_REGISTERS:
            text_offset = (sfVector2f) {char_width, char_bounds.height*2};
            sfText_setPosition(text, vec2f_add(vec2f_scale(elements[i].computed_pos,gui_scale),text_offset));
            break;
        case GUI_INSTRUCIONS:
            text_offset = (sfVector2f) {char_width, char_bounds.height*2};
            sfText_setPosition(text, vec2f_add(vec2f_scale(elements[i].computed_pos,gui_scale),text_offset));
            break;
        case GUI_DATA:
            text_offset = (sfVector2f) {char_width, char_bounds.height*2};
            sfText_setPosition(text, vec2f_add(vec2f_scale(elements[i].computed_pos,gui_scale),text_offset));
            break;
        default:
            break;
        }

        sfRenderWindow_drawRectangleShape(window, rect, NULL);
        sfRenderWindow_drawText(window, text, NULL);
    }
};


/* 
 * Positioning and element set-up
*/

static void position_DFS(int curr, char* visited) {
    if (visited[curr]) return;
    visited[curr] = 1;
    elements[curr].computed_pos = compute_position(&elements[curr]);
    int indx = 0;
    int* children = elements[curr].children;
    while (children[indx]) {
        position_DFS(elements[curr].children[indx], visited);
        indx++;
    }
}

static sfVector2f compute_position(GUI_Container* elem) {
    sfVector2f pos = (sfVector2f) {0,0};
    sfVector2f par_pos;
    sfVector2f par_size;
    if (elem->parent != -1) {
        par_pos = elements[elem->parent].computed_pos;
        par_size = elements[elem->parent].size;
    }
    switch (elem->relation) {
    case REL_FREE:
        break;
    case REL_ABOVE_LEFT:
        pos = (sfVector2f) {par_pos.x, par_pos.y-elem->size.y};
        break;
    case REL_ABOVE_RIGHT:
        pos = (sfVector2f) {par_pos.x+par_size.x-elem->size.x, par_pos.y-elem->size.y};
        break;
    case REL_ABOVE_MID:
        pos = (sfVector2f) {par_pos.x+(par_size.x-elem->size.x)/2, par_pos.y-elem->size.y};
        break;
    case REL_BELOW_LEFT:
        pos = (sfVector2f) {par_pos.x, par_pos.y+par_size.y};
        break;
    case REL_BELOW_RIGHT:
        pos = (sfVector2f) {par_pos.x+par_size.x-elem->size.x, par_pos.y+par_size.y};
        break;
    case REL_BELOW_MID:
        pos = (sfVector2f) {par_pos.x+(par_size.x-elem->size.x)/2, par_pos.y+par_size.y};
        break;
    case REL_LEFT_TOP:
        pos = (sfVector2f) {par_pos.x-elem->size.x, par_pos.y};
        break;
    case REL_LEFT_BOT:
        pos = (sfVector2f) {par_pos.x-elem->size.x, par_pos.y+par_size.y-elem->size.y};
        break;
    case REL_LEFT_MID:
        pos = (sfVector2f) {par_pos.x-elem->size.x, par_pos.y+(par_size.y-elem->size.y)/2};
        break;
    case REL_RIGHT_TOP:
        pos = (sfVector2f) {par_pos.x+par_size.x, par_pos.y};
        break;
    case REL_RIGHT_BOT:
        pos = (sfVector2f) {par_pos.x+par_size.x, par_pos.y+par_size.y-elem->size.y};
        break;
    case REL_RIGHT_MID:
        pos = (sfVector2f) {par_pos.x+par_size.x, par_pos.y+(par_size.y-elem->size.y)/2};
        break;
    default:
        break;
    }
    pos = vec2f_add(pos,elem->rel_pos);
    return pos;
}


static void setup_elements() {
    // Elements initially set up with coords and sizes for
    // 1600 x 900, then normalized to 1 x 1
    elements[0] = (GUI_Container) {
        .content_type = GUI_INSTR,
        .parent = -1,
        .children = {1, 3},
        .relation = REL_FREE,
        .rel_pos = (sfVector2f) {35, 76},
        .size = (sfVector2f) {290, 83},
        .text = calloc(100,1),
        .stage = F,
        .level = 0,
    };
    elements[1] = (GUI_Container) {
        .content_type = GUI_INSTR,
        .parent = 0,
        .children = {2},
        .relation = REL_BELOW_MID,
        .rel_pos = (sfVector2f) {0,0},
        .size = (sfVector2f) {290, 83},
        .text = calloc(100,1),
        .stage = F,
        .level = 1,
    };
    elements[2] = (GUI_Container) {
        .content_type = GUI_INSTR,
        .parent = 1,
        .children = {},
        .relation = REL_BELOW_MID,
        .rel_pos = (sfVector2f) {0,0},
        .size = (sfVector2f) {290, 83},
        .text = calloc(100,1),
        .stage = F,
        .level = 2,
    };
    elements[3] = (GUI_Container) {
        .content_type = GUI_INSTR,
        .parent = 0,
        .children = {4, 6},
        .relation = REL_RIGHT_MID,
        .rel_pos = (sfVector2f) {20, 0},
        .size = (sfVector2f) {290, 83},
        .text = calloc(100,1),
        .stage = D,
        .level = 0,
    };
    elements[4] = (GUI_Container) {
        .content_type = GUI_INSTR,
        .parent = 3,
        .children = {5},
        .relation = REL_BELOW_MID,
        .rel_pos = (sfVector2f) {0,0},
        .size = (sfVector2f) {290, 83},
        .text = calloc(100,1),
        .stage = D,
        .level = 1,
    };
    elements[5] = (GUI_Container) {
        .content_type = GUI_INSTR,
        .parent = 4,
        .children = {},
        .relation = REL_BELOW_MID,
        .rel_pos = (sfVector2f) {0,0},
        .size = (sfVector2f) {290, 83},
        .text = calloc(100,1),
        .stage = D,
        .level = 2,
    };
    elements[6] = (GUI_Container) {
        .content_type = GUI_INSTR,
        .parent = 3,
        .children = {7, 9},
        .relation = REL_RIGHT_MID,
        .rel_pos = (sfVector2f) {20, 0},
        .size = (sfVector2f) {290, 83},
        .text = calloc(100,1),
        .stage = E,
        .level = 0,
    };
    elements[7] = (GUI_Container) {
        .content_type = GUI_INSTR,
        .parent = 6,
        .children = {8},
        .relation = REL_BELOW_MID,
        .rel_pos = (sfVector2f) {0,0},
        .size = (sfVector2f) {290, 83},
        .text = calloc(100,1),
        .stage = E,
        .level = 1,
    };
    elements[8] = (GUI_Container) {
        .content_type = GUI_INSTR,
        .parent = 7,
        .children = {},
        .relation = REL_BELOW_MID,
        .rel_pos = (sfVector2f) {0,0},
        .size = (sfVector2f) {290, 83},
        .text = calloc(100,1),
        .stage = E,
        .level = 2,
    };
    elements[9] = (GUI_Container) {
        .content_type = GUI_INSTR,
        .parent = 6,
        .children = {10, 12},
        .relation = REL_RIGHT_MID,
        .rel_pos = (sfVector2f) {20, 0},
        .size = (sfVector2f) {290, 83},
        .text = calloc(100,1),
        .stage = M,
        .level = 0,
    };
    elements[10] = (GUI_Container) {
        .content_type = GUI_INSTR,
        .parent = 9,
        .children = {11},
        .relation = REL_BELOW_MID,
        .rel_pos = (sfVector2f) {0,0},
        .size = (sfVector2f) {290, 83},
        .text = calloc(100,1),
        .stage = M,
        .level = 1,
    };
    elements[11] = (GUI_Container) {
        .content_type = GUI_INSTR,
        .parent = 10,
        .children = {},
        .relation = REL_BELOW_MID,
        .rel_pos = (sfVector2f) {0,0},
        .size = (sfVector2f) {290, 83},
        .text = calloc(100,1),
        .stage = M,
        .level = 2,
    };
    elements[12] = (GUI_Container) {
        .content_type = GUI_INSTR,
        .parent = 9,
        .children = {13},
        .relation = REL_RIGHT_MID,
        .rel_pos = (sfVector2f) {20, 0},
        .size = (sfVector2f) {290, 83},
        .text = calloc(100,1),
        .stage = W,
        .level = 0,
    };
    elements[13] = (GUI_Container) {
        .content_type = GUI_INSTR,
        .parent = 12,
        .children = {14},
        .relation = REL_BELOW_MID,
        .rel_pos = (sfVector2f) {0,0},
        .size = (sfVector2f) {290, 83},
        .text = calloc(100,1),
        .stage = W,
        .level = 1,
    };
    elements[14] = (GUI_Container) {
        .content_type = GUI_INSTR,
        .parent = 13,
        .children = {},
        .relation = REL_BELOW_MID,
        .rel_pos = (sfVector2f) {0,0},
        .size = (sfVector2f) {290, 83},
        .text = calloc(100,1),
        .stage = W,
        .level = 2,
    };
    elements[15] = (GUI_Container) {
        .content_type = GUI_REGISTERS,
        .parent = -1,
        .children = {16},
        .relation = REL_FREE,
        .rel_pos = (sfVector2f) {46, 394},
        .size = (sfVector2f) {370, 229},
        .text = calloc(500,1),
    };
    elements[16] = (GUI_Container) {
        .content_type = GUI_INSTRUCIONS,
        .parent = 15,
        .children = {17},
        .relation = REL_RIGHT_TOP,
        .rel_pos = (sfVector2f) {45,0},
        .size = (sfVector2f) {570, 474},
        .text = calloc(1000,1),
    };
    elements[17] = (GUI_Container) {
        .content_type = GUI_DATA,
        .parent = 16,
        .children = {},
        .relation = REL_RIGHT_TOP,
        .rel_pos = (sfVector2f) {45, 0},
        .size = (sfVector2f) {360, 474},
        .text = calloc(1000,1),
    };

    sfVector2f scale = (sfVector2f) {1./1600., 1./900.};
    for (int i=0; i<18; i++) {
        elements[i].rel_pos = vec2f_scale(elements[i].rel_pos, scale);
        elements[i].size = vec2f_scale(elements[i].size, scale);
    }

}

/* Element positions in 1600x900
Fetch
62,76,290,83
67,161,248,78
67,239,248,78
Decode
371,76,290,83
376,161,248,78
376,239,248,78
Execute
680,76,290,83
685,161,248,78
685,239,248,78
Memory
989,76,290,83
994,161,248,78
994,239,248,78
Writeback
1298,76,290,83
1303,159,248,78
1303,237,248,78
Registers
46,394,291,229
Instruction Memory
422,394,490,474
Data Memory
1019,394,283,474


float vals[] = {
    62,78,290,83,
    67,163,248,78,
    67,243,248,78,
    371,78,290,83,
    376,163,248,78,
    376,243,248,78,
    680,78,290,83,
    685,163,248,78,
    685,243,248,78,
    989,78,290,83,
    994,163,248,78,
    994,243,248,78,
    1298,78,290,83,
    1303,163,248,78,
    1303,243,248,78,
    46,394,291,229,
    422,394,490,474,
    1019,394,283,474,
};

Spacing: 50

for (int i=0; i<18; i++) {
    printf("elements[%d] = (GUI_Container) {\n\t.content_type = GUI_INSTR,\n.pos = ",i);
    printf("(sfVector2f) {");
    printf("%lf,",vals[4*i+0]);
    printf("%lf,",vals[4*i+1]);
    printf("},\n");
    printf(".size = (sfVector2f) {");
    printf("%lf,",vals[4*i+2]);
    printf("%lf,",vals[4*i+3]);
    printf("}\n};\n");
}
*/
