#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Returns disk size
typedef size_t(*PC64KDiskSizeGetter)();
// Allocates memory for read data and reads data. Doesn't free memory.
typedef uint8_t*(*PC64KDiskReader)(size_t pos, uint8_t len);
// Writes data to disk.
typedef void(*PC64KDiskWriter)(size_t pos, uint8_t* data, uint8_t len);

// Gets microseconds since an arbitrary point in time
typedef uint64_t(*PC64KMicrosGetter)();

typedef struct {
    uint16_t pc;
    uint8_t reg[16];
    uint8_t ram[65536];
    uint8_t display[320][192];
    uint8_t stack_pos;
    uint16_t stack[128];
    uint8_t custom_font[256][16];

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