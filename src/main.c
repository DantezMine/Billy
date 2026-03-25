#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "SFML/Graphics.h"
#include "utils/events.h"
#include "utils/clock.h"
#include "gui.h"
#include "CPU.h"
#include "Parser.h"

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

    ByteCode bc = Parser_translate_from_file("BillyCore/test/testFiles/mult.txt");

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
