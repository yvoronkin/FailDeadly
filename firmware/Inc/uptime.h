#ifndef UPTIME_H
#define UPTIME_H

#include <stdint.h>

typedef
struct
{
    uint32_t low;
    uint32_t high;
} shared_uptime;

void uptime_data_reset(void);
void uptime_update_isr(void);

int32_t uptime_get(shared_uptime * uptime);

#endif

