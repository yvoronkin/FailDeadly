#include <stdint.h>

#include "tim.h"

#include "FreeRTOS.h"

void ConfigureRunTimeStatsTimer(void)
{
    __HAL_TIM_SET_COUNTER(&htim2, 0U);
    HAL_TIM_Base_Start(&htim2);
}

uint32_t GetRunTimeStatsCounter(void)
{
    return __HAL_TIM_GET_COUNTER(&htim2);
}

