#include <pc64k/video.h>

void pc64k_video_init(PC64KVideoCtx* context) {
    memset(context->framebuffer, 0, sizeof(context->framebuffer));
    memset(context->characters, 0, sizeof(context->characters));

    context->bg_color = 0b00000000;
    context->fg_color = 0b11111111;
    context->character_x = 0;
    context->character_y = 0;
}