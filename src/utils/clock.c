#include "clock.h"
#include <SFML/Graphics.h>


static sfClock* s_clock;
static sfTime s_lastFrame;
static sfTime s_thisFrame;

void Clock_init(void) {
    s_clock = sfClock_create();
    s_lastFrame = sfSeconds(0);
    s_thisFrame = sfSeconds(0);
}

void Clock_setFrame(void) {
    s_lastFrame = s_thisFrame;
    s_thisFrame = sfClock_getElapsedTime(s_clock);
}

float Clock_deltaTime(void) {
    return sfTime_asSeconds(s_thisFrame) - sfTime_asSeconds(s_lastFrame);
}

float Clock_totalTime(void) {
    return sfTime_asSeconds(s_thisFrame);
}

