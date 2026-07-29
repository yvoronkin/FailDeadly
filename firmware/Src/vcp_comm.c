#include "usbd_cdc_if.h"
#include "usb_device.h"

#include "FreeRTOS.h"
#include "task.h"
#include "stream_buffer.h"
#include "semphr.h"

#include "vcp_comm.h"

uint8_t vcpInitialized = 0;

SemaphoreHandle_t   //vcpTransmitCompleteSemaphore,
                    vcpStreamBufferSendMutex,
                    vcpStreamBufferIsEmptySemaphore;

StreamBufferHandle_t vcpTransmitStreamBuffer;

extern 
USBD_HandleTypeDef hUsbDeviceFS;

USBD_CDC_HandleTypeDef *hcdc = NULL;

static 
void vcpInit(void)
{
/*
    vcpTransmitCompleteSemaphore = xSemaphoreCreateBinary();
    if (vcpTransmitCompleteSemaphore == (SemaphoreHandle_t) NULL)
    {
        Error_Handler();
    }
*/

    vcpTransmitStreamBuffer = xStreamBufferCreate(VCP_DRV_BUFF_LEN, 1);
    if (vcpTransmitStreamBuffer == NULL)
    {
        Error_Handler();
    }

    vcpStreamBufferSendMutex = xSemaphoreCreateMutex();
    if (vcpStreamBufferSendMutex == (SemaphoreHandle_t) NULL)
    {
        Error_Handler();
    }

    vcpStreamBufferIsEmptySemaphore = xSemaphoreCreateBinary();
    if (vcpStreamBufferIsEmptySemaphore == (SemaphoreHandle_t) NULL)
    {
        Error_Handler();
    }
    
    while (hcdc == NULL)
    {
        hcdc = (USBD_CDC_HandleTypeDef *)hUsbDeviceFS.pClassData;
        vTaskDelay(1);
    }

    // Allow FreeRTOS API calls within the ISR
    NVIC_SetPriority(OTG_FS_IRQn, 6);

    vcpInitialized = 1;
}

void VCPTransmitTask(void * arg)
{
    int32_t ret;
    uint16_t numBytes;
    static uint8_t tempBuffer[VCP_DRV_BUFF_LEN];

    (void)arg;
    vcpInitialized = 0;
    vcpInit();

    while (1)
    {
        // Wait forever for data to become available in the stream-buffer
        numBytes = (uint16_t)xStreamBufferReceive(
                                vcpTransmitStreamBuffer, tempBuffer,
                                VCP_DRV_BUFF_LEN, portMAX_DELAY);

        ret = (int32_t)uxSemaphoreGetCount(vcpStreamBufferIsEmptySemaphore);
        if (ret == 0)
        {
            ret = xSemaphoreGive(vcpStreamBufferIsEmptySemaphore);
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

int32_t vcpSend(const char * buf, uint16_t len)
{
    int32_t ret; 
    size_t bytes_available;

    TickType_t start_tick_count, cur_tick_count, 
               elapsed_tick_count, remaining_tick_count;
    
    while (vcpInitialized == 0)
    {
        vTaskDelay(1);
    }

    start_tick_count = xTaskGetTickCount();
    
    if (xSemaphoreTake(vcpStreamBufferSendMutex, portMAX_DELAY) == pdPASS)
    {
        cur_tick_count = xTaskGetTickCount();

        // how long mutex take
        if (cur_tick_count >= start_tick_count)
        {
            elapsed_tick_count = cur_tick_count - start_tick_count;
            remaining_tick_count = VCP_DRV_SEND_MAX_WAIT - elapsed_tick_count;
        }
        else
        {
            remaining_tick_count = 0;
        }

        // buff is empty
        if ((int32_t)uxSemaphoreGetCount(vcpStreamBufferIsEmptySemaphore) == 1) 
        {
            // reset state
            xSemaphoreTake(vcpStreamBufferIsEmptySemaphore, 0);
        }
        
        // get size available in buffer
        bytes_available = xStreamBufferSpacesAvailable(vcpTransmitStreamBuffer);

        // if there is some waiting-time remaining wait until buffer will be emptied
        if ((bytes_available < len) && (remaining_tick_count > 0))
        {
            if (xSemaphoreTake(vcpStreamBufferIsEmptySemaphore, 
                               remaining_tick_count) == pdPASS)
            {
                bytes_available = VCP_DRV_BUFF_LEN;
            }
        }

        if (bytes_available >= len)
        {
            ret = xStreamBufferSend(vcpTransmitStreamBuffer, buf, len, 0);
        }
        else // not enough space, return error
        {
            ret = -1;
        }
        xSemaphoreGive(vcpStreamBufferSendMutex);
    }
    else
    {
        ret = -2;
    }

    return ret;
}
