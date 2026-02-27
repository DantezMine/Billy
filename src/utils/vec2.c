#include "vec2.h"


sfVector2f vec2f_mul(sfVector2f a, float b) {
    return (sfVector2f) {a.x*b, a.y*b};
}

sfVector2f vec2f_scale(sfVector2f a, sfVector2f b) {
    return (sfVector2f) {a.x*b.x, a.y*b.y};
}

sfVector2f vec2f_add(sfVector2f a, sfVector2f b) {
    return (sfVector2f) {a.x+b.x, a.y+b.y};
}

sfVector2f vec2f_sub(sfVector2f a, sfVector2f b) {
    return (sfVector2f) {a.x-b.x, a.y-b.y};
}
