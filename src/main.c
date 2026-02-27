#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "SFML/Graphics.h"
#include "utils/events.h"
#include "utils/clock.h"
#include "gui.h"
#include "CPU.h"
#include "Translation.h"

#define SCREEN_MODE sfDefaultStyle
#define W_WIDTH 1600
#define W_HEIGHT 900


int main() {

    sfVideoMode videomode = sfVideoMode_getDesktopMode();
    sfRenderWindow* window = sfRenderWindow_create(videomode, "BillyApp", SCREEN_MODE, NULL);
    sfRenderWindow_setSize(window,(sfVector2u){W_WIDTH,W_HEIGHT});
    srand((uint32_t)time(NULL));
    Clock_init();


    gui_init(window);
    printf("Videomode size: %d, %d\n",videomode.width,videomode.height);



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

    CPU_Init();
    CPU_SetInstructionMemory((uint8_t*)bc.instr);
    gui_clock();

    while (sfRenderWindow_isOpen(window)) {
        Clock_setFrame();

        handle_events(window);

        sfRenderWindow_clear(window, COLOR_BACKGROUND);

        gui_update(window);

        sfRenderWindow_display(window);
    }

    gui_close();
    sfRenderWindow_destroy(window);
}
