/*
 * Colour.h - one colour as three brightness levels.
 */

#pragma once

#include <stdint.h>

/* Each channel runs from 0 (off) to 255 (full brightness). */
struct Colour {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};
