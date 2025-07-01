#include <pc64k.h>

PC64K* pc64k_alloc_init(uint8_t* rom, size_t rom_size,
    PC64KDiskSizeGetter cb_disk_size,
    PC64KDiskReader cb_disk_read,
    PC64KDiskWriter cb_disk_write,
    PC64KMicrosGetter cb_micros) {
    if(rom_size > 65535)
        return NULL;
    PC64K* ctx = (PC64K*) malloc(sizeof(PC64K));
    memcpy(ctx->ram, rom, rom_size);

    ctx->cb_disk_size = cb_disk_size;
    ctx->cb_disk_read = cb_disk_read;
    ctx->cb_disk_write = cb_disk_write;
    ctx->cb_micros = cb_micros;

    ctx->stack_pos = 0;
    ctx->pc = 0;

    pc64k_video_init(&ctx->video);
    memset(ctx->reg, 0, sizeof(ctx->reg));

    return ctx;
}
void pc64k_deinit_free(PC64K* ctx) {
    free(ctx);
}

uint8_t read_char(PC64K* ctx) {
    return ctx->ram[ctx->pc++];
}
uint16_t read_word(PC64K* ctx) {
    uint16_t w = (((uint16_t) ctx->ram[ctx->pc]) << 8) | ctx->ram[ctx->pc + 1];
    ctx->pc += 2;
    return w;
}

void pc64k_tick(PC64K* ctx) {
    uint8_t opcode = read_char(ctx);
    if(opcode == 0x00)
        ctx->pc = read_word(ctx);
    else if(opcode == 0x1c)
        pc64k_video_print(&ctx->video, (PC64KCharacter) {
            .font = 0,
            .character = read_char(ctx)
        });
}