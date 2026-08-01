#include "main.h"

#include "FreeRTOS.h"
#include "task.h"

#include "fd_tasks.h"
#include "fd_log.h"
#include "test_task.h"
#include "fd_crypto.h"
#include "vcp_comm.h"


void FDTasksInit(void)
{
    FDLogTaskInit();
    VCPTransmitTaskInit();
    BlinkTestTaskInit();
    FDCryptoTaskInit();

    FDLogFreeRTOSStateInit();

    return;
}
