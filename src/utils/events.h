#ifndef H_IO
#define H_IO

#include <SFML/Graphics.h>
#include <stdio.h>

void handle_keyreleased(sfRenderWindow* window, sfEvent* event);

void handle_mouse_released(sfRenderWindow* window, sfEvent* event);

void handle_resize(sfRenderWindow* window, sfEvent* event);

void handle_events(sfRenderWindow* window);

#endif
