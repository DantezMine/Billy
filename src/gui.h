#ifndef H_GUI
#define H_GUI

#include <SFML/Graphics.h>
#include "CPU.h"

#define COLOR_BACKGROUND  sfColor_fromInteger(0x202020FF)
#define COLOR_TEXT        sfColor_fromInteger(0xFFFFFFFF)
#define COLOR_BORDER      sfColor_fromInteger(0x000000FF)
#define COLOR_FILL        sfColor_fromInteger(0x4E4EF5FF)
#define BORDER_THICKNESS  0.001
#define TEXT_SIZE         22

typedef enum GUI_content_type {
    GUI_REGISTERS,
    GUI_DATA,
    GUI_INSTRUCIONS,
    GUI_INSTR,
} GUI_content_type ;

typedef enum {
    REL_FREE,
    REL_ABOVE_LEFT,
    REL_ABOVE_RIGHT,
    REL_ABOVE_MID,
    REL_BELOW_LEFT,
    REL_BELOW_RIGHT,
    REL_BELOW_MID,
    REL_LEFT_TOP,
    REL_LEFT_BOT,
    REL_LEFT_MID,
    REL_RIGHT_TOP,
    REL_RIGHT_BOT,
    REL_RIGHT_MID,
} GUI_POS_RELATION;

// Position and size are in [0,1] x [0,1]
typedef struct GUI_Container {
    GUI_content_type content_type;
    int parent;
    int children[4]; // 0 considered invalid, used for end of array
    GUI_POS_RELATION relation;
    sfVector2f computed_pos; // Computed automatically from parent relation
    sfVector2f rel_pos; // Specifies relative position from default placement given by parent relation
    sfVector2f size;
    char* text;
    int stage;
    int level;
} GUI_Container;

sfView* gui_init(sfRenderWindow* window);

void gui_update(sfRenderWindow* window);

void gui_clock();

void gui_close();

#endif
