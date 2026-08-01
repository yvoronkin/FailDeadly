#include <string.h>

#include "main.h"
#include "stm32_assert.h"
#include "tim.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#include "macros.h"
#include "uptime.h"

struct uptime_data
{
    shared_uptime uptime;

    StaticSemaphore_t StaticMutex;
    SemaphoreHandle_t uptime_get_mutex;
};

#define UPTIME_DATA_SIZE 64
#define UPTIME_DATA_ALIGN ROUND_UP_POW2(UPTIME_DATA_SIZE)

#if (UPTIME_DATA_ALIGN > 128)
#warn "UPTIME data align take too much MPU region"
#endif

static 
struct uptime_data uptime_data
__attribute__ ((aligned(UPTIME_DATA_ALIGN)));

void uptime_data_reset(void)
{
    memset(&uptime_data, 0, sizeof(struct uptime_data));

    uptime_data.uptime_get_mutex = 
        xSemaphoreCreateMutexStatic(&uptime_data.StaticMutex);
    
    assert_param(uptime_data.uptime_get_mutex != NULL);
            
    __HAL_TIM_SET_COUNTER(&htim4, 0U);
    HAL_TIM_Base_Start_IT(&htim4);
}

void uptime_update_isr(void)
{
    uint32_t prev_value;

    prev_value = uptime_data.uptime.low;
    uptime_data.uptime.low += 1;
    if (uptime_data.uptime.low < prev_value)
    {
        // low part overflow
        uptime_data.uptime.high += 1;
    }
}

int32_t uptime_get(shared_uptime * uptime)
{
    int32_t res;
    
    res = xSemaphoreTake(uptime_data.uptime_get_mutex, 100);
    if (res == pdPASS)
    {
        __disable_irq();
        memcpy(uptime, &uptime_data.uptime, sizeof(uptime_data.uptime));
        __enable_irq();
        xSemaphoreGive(uptime_data.uptime_get_mutex);
    }
    
    return res;
}
