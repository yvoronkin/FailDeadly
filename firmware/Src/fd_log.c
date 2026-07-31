#include "dma.h"
#include "usart.h"

#include "FreeRTOS.h"
#include "task.h"
#include "stream_buffer.h"
#include "semphr.h"

#include "macros.h"
#include "fd_log.h"

typedef
enum log_state_flags
{
    RTOS_INITIALIZED    = 0x01,
    KEEP_BLOCKING       = 0x02,
} log_state_flags;

#ifndef DISABLE_LOG_FUNC

struct log_data
{
    StaticSemaphore_t StaticMutex;
    StaticSemaphore_t StaticSemaphore;
    StaticSemaphore_t StaticSemaphore2;
    StaticStreamBuffer_t StaticBuffer;

    SemaphoreHandle_t logStreamBufferSendMutex;
    SemaphoreHandle_t logStreamBufferIsEmptySemaphore;
    SemaphoreHandle_t logTransmitionComplete;
    StreamBufferHandle_t logStreamBuffer;

    log_state_flags state : 8;
    uint8_t buffer[LOG_BUFFER_LEN + 1];
};

static 
__attribute__ ((aligned(2048)))
struct log_data log_data = {0}; // Init with zero

#else

struct log_data
{
    log_state_flags state : 8;
}

static
__attribute__ ((aligned(2)))
struct log_data log_data = {0};

#endif

const
MemoryRegion_t log_data_region = {
    &log_data,
    sizeof(log_data),
    portMPU_REGION_READ_WRITE |
    portMPU_REGION_PRIVILEGED_READ_WRITE |
    portMPU_REGION_EXECUTE_NEVER,
};

#ifndef DISABLE_LOG_FUNC

static
void FDLogInit(void)
{
    log_data.logStreamBuffer =
        xStreamBufferCreateStatic(LOG_BUFFER_LEN, 1,
                                  log_data.buffer, &log_data.StaticBuffer);
    if (log_data.logStreamBuffer == NULL)
    {
        Error_Handler();
    }

    log_data.logStreamBufferSendMutex =
        xSemaphoreCreateMutexStatic(&log_data.StaticMutex);
    if (log_data.logStreamBufferSendMutex == (SemaphoreHandle_t)NULL)
    {
        Error_Handler();
    }

    log_data.logStreamBufferIsEmptySemaphore = 
        xSemaphoreCreateBinaryStatic(&log_data.StaticSemaphore);
    if (log_data.logStreamBufferIsEmptySemaphore == (SemaphoreHandle_t)NULL)
    {
        Error_Handler();
    }

    log_data.logTransmitionComplete = 
        xSemaphoreCreateBinaryStatic(&log_data.StaticSemaphore2);
    if (log_data.logStreamBufferIsEmptySemaphore == (SemaphoreHandle_t)NULL)
    {
        Error_Handler();
    }

    // Allow FreeRTOS API from ISR
    NVIC_SetPriority(DMA2_Stream7_IRQn, 6);
    NVIC_SetPriority(USART1_IRQn, 6);

    FLAG_SET(log_data.state, RTOS_INITIALIZED);
}

static
void FDLogTask(void * arg)
{
    int32_t ret;
    uint16_t numBytes;
    static uint8_t tempBuffer[LOG_BUFFER_LEN];

    (void)arg;
    FLAG_CLR(log_data.state, RTOS_INITIALIZED);
    FDLogInit();

    while (1)
    {
        numBytes = (uint16_t)xStreamBufferReceive(
                            log_data.logStreamBuffer, tempBuffer,
                            LOG_BUFFER_LEN, portMAX_DELAY);

        // notify that buffer is empty
        ret = (int32_t)uxSemaphoreGetCount(log_data.logStreamBufferIsEmptySemaphore);
        if (ret == 0)
        {
            ret = xSemaphoreGive(log_data.logStreamBufferIsEmptySemaphore);
        }

        // Transmit the data
        HAL_UART_Transmit_DMA(&huart1, tempBuffer, numBytes); 

        // Wait for transmition copletion
        xSemaphoreTake(log_data.logTransmitionComplete, portMAX_DELAY);
    }
}

#define FDLOG_TASK_STACK_WORDS 128 + (LOG_BUFFER_LEN / 4)
#define FDLOG_TASK_STACK_ALIGN ROUND_UP_POW2(FDLOG_TASK_STACK_WORDS * 4)
#if (FDLOG_TASK_STACK_ALIGN > 2048)
#warn "FDLOG Stack take a lot of MPU region"
#endif

static
portSTACK_TYPE xTaskStack[FDLOG_TASK_STACK_WORDS]
__attribute__((aligned(FDLOG_TASK_STACK_ALIGN)));

void FDLogTaskInit(void)
{
    BaseType_t res;

    const
    TaskParameters_t task_params = {
        FDLogTask,
        "UART Logs",
        FDLOG_TASK_STACK_WORDS,
        NULL,
        7 | portPRIVILEGE_BIT,
        xTaskStack,
        {
            { 0 },
        }
    };

    res = xTaskCreateRestricted(&task_params, NULL);
    if (res != pdPASS) while(1) {};
}

extern UART_HandleTypeDef huart1;
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    if (huart->Instance == USART1)
    {
        FLAG_CLR(log_data.state, KEEP_BLOCKING); // clear flag anyway

        if (FLAG_HAS(log_data.state, RTOS_INITIALIZED))
        {
            xSemaphoreGiveFromISR(log_data.logTransmitionComplete, &xHigherPriorityTaskWoken); 
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
}

/*
 * Send log to USART1 TX using DMA
 * If RTOS not yet initialized, block untill dma completion
 */
int32_t FDSendLog(const unsigned char * buf, size_t len, int omit_rtos, int noblock)
{
    int32_t ret;
    size_t bytes_available;
    HAL_StatusTypeDef status;

    TickType_t start_tick, cur_tick, elapsed_tick, remaining_tick;

    if (FLAG_HAS(log_data.state, RTOS_INITIALIZED) && !omit_rtos)
    {
        start_tick = xTaskGetTickCount();
        if (xSemaphoreTake(log_data.logStreamBufferSendMutex, LOG_SEND_MAX_WAIT))
        {
            cur_tick = xTaskGetTickCount();

            //how long mutex taken
            if (cur_tick >= start_tick)
            {
                elapsed_tick = cur_tick - start_tick;
            }
            else
            {
                elapsed_tick = (portMAX_DELAY - start_tick) + cur_tick + 1;
            }
            remaining_tick = LOG_SEND_MAX_WAIT - elapsed_tick;

            if ((int32_t)uxSemaphoreGetCount(log_data.logStreamBufferIsEmptySemaphore) == 1)
            {
                // reset
                xSemaphoreTake(log_data.logStreamBufferIsEmptySemaphore, 0);
            }

            bytes_available = xStreamBufferSpacesAvailable(log_data.logStreamBuffer);

            // if there is some waiting-time remaining wait until buffer will be emptied
            if ((bytes_available < len) && (remaining_tick > 0))
            {
                if (xSemaphoreTake(log_data.logStreamBufferIsEmptySemaphore, 
                                    remaining_tick) == pdPASS)
                {
                    bytes_available = LOG_BUFFER_LEN;
                }
            }

            if (bytes_available >= len)
            {
                ret = xStreamBufferSend(log_data.logStreamBuffer, buf, len, 0); 
            }
            else ret = pdFAIL;

            xSemaphoreGive(log_data.logStreamBufferSendMutex);
        }
        else ret = -2;
    }
    else // process with blocking 
    {
        status = HAL_UART_Transmit_DMA(&huart1, buf, len); 
        if (status != HAL_OK)
        {
            // TODO: handle error somehow
            return pdFAIL;
        }
        if (noblock) return pdPASS;
        
        FLAG_SET(log_data.state, KEEP_BLOCKING);
        while (FLAG_HAS(log_data.state, KEEP_BLOCKING))
        {
            HAL_Delay(1); // Wait for ISR from USART1 DMA
        }

        ret = pdPASS;
    }

    return ret;
}

void FDLogFreeRTOSOnceTask(void)
{
    
}

#else

void FDSendLog(const char * buf, size_t len, int omit_rtos, int noblock) {}
void FDLogTaskInit(void) {}
void FDLogFreeRTOSOnceTask(void) {}

#endif

