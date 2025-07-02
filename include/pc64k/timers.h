#pragma once

#include <pc64k/callbacks.h>

typedef struct {
    uint8_t value;
    uint8_t freq;
    uint64_t last;
} PC64KTimer;

void pc64k_timer_init(PC64KTimer* timer, PC64KMicrosGetter micros);
void pc64k_timer_tick(PC64KTimer* timer, PC64KMicrosGetter micros);