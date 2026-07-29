#include "main.h"

#include "FreeRTOS.h"
#include "task.h"

#include "fd_tasks.h"
#include "test_task.h"
#include "vcp_comm.h"


void FDTasksInit(void)
{
    BaseType_t result;

    result = xTaskCreate(
        VCPTransmitTask,
        "vcp_comm",
        256,
        NULL,
        6,
        NULL
    );
    if (result != pdPASS) Error_Handler();

    result = xTaskCreate(
        BlinkTestTask,
        "BlinkTask",
        256,
        NULL,
        tskIDLE_PRIORITY + 1,
        NULL
    );

    if (result != pdPASS) Error_Handler();

    return;
}
