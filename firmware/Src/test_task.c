#include "gpio.h"
#include "FreeRTOS.h"
#include "task.h"

#include "test_task.h"
#include "vcp_comm.h"

void BlinkTestTask(void *arg)
{
    const 
    char msg[] = "Smoke test\r\n";

    (void)arg;
    for (;;) {
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        vcpSend(msg, 12); 
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
