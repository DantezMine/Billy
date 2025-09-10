#include <stdio.h>
#include "CPU.h"
#include "SFML/Window.h"

int main() {
    CPU_Init();
    while(1) {
        if (sfKeyboard_isKeyPressed(sfKeySpace)) {
            CPU_Clock();
        }
    }
}
