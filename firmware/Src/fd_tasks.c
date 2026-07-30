#include "main.h"

#include "FreeRTOS.h"
#include "task.h"

#include "fd_tasks.h"
#include "test_task.h"
#include "vcp_comm.h"


void FDTasksInit(void)
{
    VCPTransmitTaskInit();
    BlinkTestTaskInit();
    FDCryptoTaskInit();

    return;
}
