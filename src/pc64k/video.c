#include <pc64k/video.h>
#include <pc64k/font.h>

void pc64k_video_init(PC64KVideoCtx* context) {
    context->bg_color = 0b00000011;
    context->fg_color = 0b11111111;

    memset(context->framebuffer, context->bg_color, sizeof(context->framebuffer));
    memset(context->characters, 0, sizeof(context->characters));
    memset(context->custom_font, 0, sizeof(context->custom_font));

    context->character_x = 0;
    context->character_y = 0;
}

static void pc64k_draw_character(PC64KVideoCtx* context, PC64KCharacter character, uint8_t x, uint8_t y) {
    uint16_t sx = (uint16_t) x * CHAR_WIDTH;
    uint8_t sy = y * CHAR_HEIGHT;
    for(uint16_t cx = sx; cx < sx + CHAR_WIDTH; cx++)
        for(uint16_t cy = sy; cy < sy + CHAR_HEIGHT; cy++)
            context->framebuffer[cx][cy] =
                (((character.font == 0 ? system_font : context->custom_font)[character.character][cy - sy] >> (cx - sx)) & 1)
                ? context->fg_color
                : context->bg_color;
}

static void pc64k_redraw(PC64KVideoCtx* context) {
    for(uint8_t x = 0; x < SCREEN_WIDTH; x++)
        for(uint8_t y = 0; y < SCREEN_HEIGHT; y++)
            pc64k_draw_character(context, context->characters[x][y], x, y);
}

static void pc64k_newline(PC64KVideoCtx* context) {
    if(context->character_y == SCREEN_HEIGHT - 1) {
        for(uint8_t y = 0; y < SCREEN_HEIGHT - 1; y++)
            for(uint8_t x = 0; x < SCREEN_WIDTH; x++)
                context->characters[x][y] = context->characters[x][y + 1];
        for(uint8_t x = 0; x < SCREEN_WIDTH; x++)
            context->characters[x][SCREEN_HEIGHT - 1] = (PC64KCharacter){
                .character = 0,
                .font = 0
            };
        pc64k_redraw(context);
    } else context->character_y++;
}

void pc64k_video_print(PC64KVideoCtx* context, PC64KCharacter character) {
    if(character.character == '\r') {
        context->character_x = 0;
        return;
    } else if(character.character == '\n') {
        pc64k_newline(context);
    } else {
        context->characters[context->character_x][context->character_y] = character;
        pc64k_draw_character(context, character, context->character_x, context->character_y);
        context->character_x++;
        if(context->character_x == SCREEN_WIDTH) {
            pc64k_newline(context);
            context->character_x = 0;
        }
    }
}
