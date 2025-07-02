#include <pc64k/timers.h>

void pc64k_timer_init(PC64KTimer* timer, PC64KMicrosGetter micros) {
    timer->freq = 10;
    timer->last = micros();
    timer->value = 0;
}
void pc64k_timer_tick(PC64KTimer* timer, PC64KMicrosGetter micros) {
    if(timer->freq == 0) return;
    uint64_t now = micros();
    uint64_t diff = timer->last - now;

    uint64_t interval = (double) 1000000 / timer->freq;
    while(now - timer->last >= interval) {
        if(timer->value > 0) timer->value--;
        timer->last += interval;
    }
}