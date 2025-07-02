#pragma once
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#define SCREEN_WIDTH 40
#define SCREEN_HEIGHT 12
#define CHAR_WIDTH 8
#define CHAR_HEIGHT 16
#define DISPLAY_WIDTH (SCREEN_WIDTH * CHAR_WIDTH)
#define DISPLAY_HEIGHT (SCREEN_HEIGHT * CHAR_HEIGHT)

typedef struct {
    uint8_t font;
    uint8_t character;
} PC64KCharacter;

typedef struct {
    uint8_t framebuffer[DISPLAY_WIDTH][DISPLAY_HEIGHT];
    PC64KCharacter characters[SCREEN_WIDTH][SCREEN_HEIGHT];

    uint8_t character_x;
    uint8_t character_y;

    uint8_t bg_color;
    uint8_t fg_color;

    uint8_t custom_font[256][16];
} PC64KVideoCtx;

void pc64k_video_init(PC64KVideoCtx* context);

void pc64k_redraw(PC64KVideoCtx* context);

void pc64k_video_print(PC64KVideoCtx* context, PC64KCharacter character);
void pc64k_video_clear(PC64KVideoCtx* context);