#include "events.h"
#include "CPU.h"
#include "../gui.h"
#include "SFML/Graphics/RenderWindow.h"
#include "SFML/Graphics/Types.h"
#include "SFML/Window/Event.h"

void handle_keyreleased(sfRenderWindow* window, sfEvent* event) {
    sfKeyEvent key_event = event->key;
    switch (key_event.code) {
        case sfKeyEscape:
            sfRenderWindow_close(window);
            break;
        case sfKeyC:
            // printf("Stepping CPU\n");
            CPU_Clock();
            gui_clock();
        default:
            break;
    }
}


void handle_mouse_released(sfRenderWindow* window, sfEvent* event) {
    sfMouseButtonEvent mouse_event = event->mouseButton;
    sfVector2u mouse_pos = (sfVector2u) {mouse_event.x,mouse_event.y};
    switch (mouse_event.button) {
        case sfMouseLeft:
            // printf("Mouse left released at (%u, %u)\n",mouse_pos.x,mouse_pos.y);
            break;
        case sfMouseRight:
            // printf("Mouse right released at (%u, %u)\n",mouse_pos.x,mouse_pos.y);
            break;
        default:
            break;
    };
}

void handle_resize(sfRenderWindow* window, sfEvent* event) {
    sfVector2u size = sfRenderWindow_getSize(window);
    float ratio = 16.f / 9.f;
    size.y = size.x / ratio;
    sfRenderWindow_setSize(window,size);
}

void handle_events(sfRenderWindow* window) {
    sfEvent event;
    while (sfRenderWindow_pollEvent(window, &event)) {
        switch (event.type) {
            case sfEvtClosed:
                sfRenderWindow_close(window);
                break;
            case sfEvtKeyReleased:
                handle_keyreleased(window, &event);
            case sfEvtMouseButtonReleased:
                handle_mouse_released(window, &event);
            case sfEvtResized:
                handle_resize(window, &event);
            default:
                break;
        }
    }
}
