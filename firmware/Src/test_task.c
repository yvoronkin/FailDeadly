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

static 
portSTACK_TYPE xTaskStack[ 128 ] 
__attribute__((aligned(128*4)));


extern const
MemoryRegion_t vcp_data_region;

void BlinkTestTaskInit(void)
{
    const
    TaskParameters_t blink_param = {
        BlinkTestTask,
        "blink",
        128,
        NULL,
        tskIDLE_PRIORITY + 1,
        xTaskStack,
        {
            vcp_data_region,
        }
    };

    BaseType_t res = xTaskCreateRestricted(&blink_param, NULL);

    if (res != pdPASS) Error_Handler();
}
