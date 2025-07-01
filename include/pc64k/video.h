#pragma once
#include <stdint.h>

typedef struct {
    uint8_t framebuffer[320][192];
    uint8_t characters[40][12];

    uint8_t character_x;
    uint8_t character_y;

    uint8_t bg_color;
    uint8_t fg_color;
} PC64KVideoCtx;

void pc64k_video_init(PC64KVideoCtx* context);