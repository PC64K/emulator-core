#pragma once
#include <pc64k/video.h>
#include <pc64k/timers.h>
#include <pc64k/callbacks.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    uint16_t pc;
    uint8_t reg[16];
    uint8_t ram[65536];
    PC64KVideoCtx video;
    uint8_t stack_pos;
    uint16_t stack[128];
    uint8_t keyboard[32];

    PC64KTimer delay;
    PC64KTimer sound;

    PC64KDiskSizeGetter cb_disk_size;
    PC64KDiskReader cb_disk_read;
    PC64KDiskWriter cb_disk_write;
    PC64KMicrosGetter cb_micros;
} PC64K;

PC64K* pc64k_alloc_init(uint8_t* rom, size_t rom_size,
    PC64KDiskSizeGetter cb_disk_size,
    PC64KDiskReader cb_disk_read,
    PC64KDiskWriter cb_disk_write,
    PC64KMicrosGetter cb_micros);
void pc64k_deinit_free(PC64K* ctx);

void pc64k_setkey(PC64K* ctx, uint8_t key, bool down);
void pc64k_tick(PC64K* ctx);