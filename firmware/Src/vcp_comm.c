#include "usbd_cdc_if.h"
#include "usb_device.h"

#include "FreeRTOS.h"
#include "task.h"
#include "stream_buffer.h"
#include "semphr.h"

#include "macros.h"
#include "vcp_comm.h"

struct vcp_data
{
    StaticSemaphore_t    StaticMutex;
    StaticSemaphore_t    StaticSemaphore;
    StaticStreamBuffer_t StaticBuffer;

    SemaphoreHandle_t   vcpStreamBufferSendMutex;
    SemaphoreHandle_t   vcpStreamBufferIsEmptySemaphore;
    StreamBufferHandle_t vcpTransmitStreamBuffer;

    uint8_t vcpInitialized;
    uint8_t buffer[VCP_DRV_BUFF_LEN + 1];
};

static
struct vcp_data vcp_data
__attribute__((aligned(ROUND_UP_POW2(sizeof(struct vcp_data)))));

const
MemoryRegion_t vcp_data_region = {
    &vcp_data,
    sizeof(vcp_data),
    portMPU_REGION_READ_WRITE | 
    portMPU_REGION_PRIVILEGED_READ_WRITE | 
    portMPU_REGION_EXECUTE_NEVER,
};

static 
void vcpInit(void)
{
    vcp_data.vcpTransmitStreamBuffer = 
        xStreamBufferCreateStatic(VCP_DRV_BUFF_LEN, 1, 
                                  vcp_data.buffer, &vcp_data.StaticBuffer);
    if (vcp_data.vcpTransmitStreamBuffer == NULL)
    {
        Error_Handler();
    }

    vcp_data.vcpStreamBufferSendMutex = 
        xSemaphoreCreateMutexStatic(&vcp_data.StaticMutex);
    if (vcp_data.vcpStreamBufferSendMutex == (SemaphoreHandle_t)NULL)
    {
        Error_Handler();
    }

    vcp_data.vcpStreamBufferIsEmptySemaphore = 
        xSemaphoreCreateBinaryStatic(&vcp_data.StaticSemaphore);
    if (vcp_data.vcpStreamBufferIsEmptySemaphore == (SemaphoreHandle_t) NULL)
    {
        Error_Handler();
    }

    vcp_data.vcpInitialized = 1;
    return;
}

extern 
USBD_HandleTypeDef hUsbDeviceFS;

void VCPTransmitTask(void * arg)
{
    int32_t ret;
    uint16_t numBytes;
    static uint8_t tempBuffer[VCP_DRV_BUFF_LEN];
    USBD_CDC_HandleTypeDef *hcdc = NULL;

    (void)arg;
    vcp_data.vcpInitialized = 0;
    vcpInit();

    while (hcdc == NULL)
    {
        hcdc = (USBD_CDC_HandleTypeDef *)hUsbDeviceFS.pClassData;
        vTaskDelay(1);
    }

    while (1)
    {
        // Wait forever for data to become available in the stream-buffer
        numBytes = (uint16_t)xStreamBufferReceive(
                                vcp_data.vcpTransmitStreamBuffer, tempBuffer,
                                VCP_DRV_BUFF_LEN, portMAX_DELAY);

        ret = (int32_t)uxSemaphoreGetCount(vcp_data.vcpStreamBufferIsEmptySemaphore);
        if (ret == 0)
        {
            ret = xSemaphoreGive(vcp_data.vcpStreamBufferIsEmptySemaphore);
        }

        // Transmit the data
        CDC_Transmit_FS(tempBuffer, numBytes);

        // Wait for transmission completion
        while (hcdc->TxState == 0U)
        {
            vTaskDelay(1);
        }
    }
}

#define VCP_STACK_DEPTH 128 + (VCP_DRV_BUFF_LEN / 4)
#define VCP_STACK_ALIGN ROUND_UP_POW2((VCP_STACK_DEPTH * 4))

#if (VCP_STACK_ALIGN > 2048)
#warn "VCP Stack take a lot of MPU region"
#endif

static 
portSTACK_TYPE xTaskStack[ VCP_STACK_DEPTH ] 
__attribute__((aligned(VCP_STACK_ALIGN)));

void VCPTransmitTaskInit(void)
{
    BaseType_t res;

    const
    TaskParameters_t task_params = {
        VCPTransmitTask,
        "VCP comm",
        VCP_STACK_DEPTH,
        NULL,
        6 | portPRIVILEGE_BIT,
        xTaskStack,
        {
            { 0 },
        }
    };

    res = xTaskCreateRestricted(&task_params, NULL);
    assert_param(res == pdPASS);
}

int32_t vcpSend(const char * buf, uint16_t len)
{
    int32_t ret; 
    size_t bytes_available;

    TickType_t start_tick_count, cur_tick_count, 
               elapsed_tick_count, remaining_tick_count;
    
    while (vcp_data.vcpInitialized == 0)
    {
        vTaskDelay(1);
    }

    start_tick_count = xTaskGetTickCount();
    
    if (xSemaphoreTake(vcp_data.vcpStreamBufferSendMutex, VCP_DRV_SEND_MAX_WAIT) == pdPASS)
    {
        cur_tick_count = xTaskGetTickCount();

        // how long mutex taken
        if (cur_tick_count >= start_tick_count)
        {
            elapsed_tick_count = cur_tick_count - start_tick_count;
        }
        else
        {
            elapsed_tick_count = (portMAX_DELAY - start_tick_count) + cur_tick_count + 1;
        }
        remaining_tick_count = VCP_DRV_SEND_MAX_WAIT - elapsed_tick_count;

        // buff is empty
        if ((int32_t)uxSemaphoreGetCount(vcp_data.vcpStreamBufferIsEmptySemaphore) == 1) 
        {
            // reset state
            xSemaphoreTake(vcp_data.vcpStreamBufferIsEmptySemaphore, 0);
        }
        
        // get size available in buffer
        bytes_available = xStreamBufferSpacesAvailable(vcp_data.vcpTransmitStreamBuffer);

        // if there is some waiting-time remaining wait until buffer will be emptied
        if ((bytes_available < len) && (remaining_tick_count > 0))
        {
            if (xSemaphoreTake(vcp_data.vcpStreamBufferIsEmptySemaphore, 
                               remaining_tick_count) == pdPASS)
            {
                bytes_available = VCP_DRV_BUFF_LEN;
            }
        }

        if (bytes_available >= len)
        {
            ret = xStreamBufferSend(vcp_data.vcpTransmitStreamBuffer, buf, len, 0);
        }
        // not enough space, return error
        else ret = pdFAIL;

        xSemaphoreGive(vcp_data.vcpStreamBufferSendMutex);
    }
    else ret = -2;

    return ret;
}
