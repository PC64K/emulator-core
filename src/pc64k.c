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

    memset(ctx->display, 0, sizeof(ctx->display));
    memset(ctx->reg, 0, sizeof(ctx->reg));
    memset(ctx->custom_font, 0, sizeof(ctx->custom_font));

    return ctx;
}
void pc64k_deinit_free(PC64K* ctx) {
    free(ctx);
}