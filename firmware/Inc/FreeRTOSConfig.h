#ifndef FREERTOSCONFIG_H
#define FREERTOSCONFIG_H

#include <stdint.h>

void ConfigureRunTimeStatsTimer(void);
uint32_t GetRunTimeStatsCounter(void);

extern uint32_t SystemCoreClock;

#define configUSE_PREEMPTION                             1
#define configUSE_TIME_SLICING                          1
#define configUSE_PORT_OPTIMISEX_TASK_SELECTION         1

#define configCPU_CLOCK_HZ                              SystemCoreClock
#define configTICK_RATE_HZ                              ((TickType_t)1000)
#define configMAX_PRIORITIES                            8
#define configMINIMAL_STACK_SIZE                        ((uint16_t)128)
#define configMAX_TASK_NAME_LEN                         16
#define configTOTAL_HEAP_SIZE                           ((size_t)(6u * 1024))

#define configUSE_16_BIT_TICKS                          0
#define configIDLE_SHOULD_YIELD                         1

#define configUSE_IDLE_HOOK                             0
#define configUSE_TICK_HOOK                             0

#define configUSE_MUTEXES                               1
#define configUSE_RECURSIVE_MUTEXES                     1
#define configUSE_COUNTING_SEMAPHORES                   1
#define configQUEUE_REGISTRY_SIZE                       8

#define configUSE_TIMERS                                1
#define configTIMER_TASK_PRIORITY                       3
#define configTIMER_QUEUE_LENGTH                        8
#define configTIMER_TASK_STACK_DEPTH                    256

#define configUSE_TASK_NOTIFICATIONS                    1
#define configTASK_NOTIFICATION_ARRAY_ENTRIES           1

#define configCHECK_FOR_STACK_OVERFLOW                  0

#define configSUPPORT_STATIC_ALLOCATION                 1
#define configSUPPORT_DYNAMIC_ALLOCATION                1

#define configUSE_TRACE_FACILITY                        1
#define configGENERATE_RUN_TIME_STATS                   1
#define configUSE_STATS_FORMATTING_FUNCTIONS            0

#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS() \
    ConfigureRunTimeStatsTimer()
#define portGET_RUN_TIME_COUNTER_VALUE() \
    GetRunTimeStatsCounter()

#define configENABLE_FPU                                1

#define configENABLE_MPU                                1
#define configENFORCE_SYSTEM_CALLS_FROM_KERNEL_ONLY     0
#define configALLOW_UNPRIVILEGED_CRITICAL_SECTIONS      0
#define configUSE_MPU_WRAPPERS_V1                       0
#define configPROTECTED_KERNEL_OBJECT_POOL_SIZE         32
#define configSYSTEM_CALL_STACK_SIZE                    configMINIMAL_STACK_SIZE
#define configENABEL_ACCESS_CONTROL_LIST                1


/* STM32F4 implements four priority bits. */
#define configPRIO_BITS                                 4

#define configLIBRARY_LOWEST_INTERRUP_PRIORITY          15
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY    5

#define configKERNEL_INTERRUPT_PRIORITY \
    (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))

#define configMAX_SYSCALL_INTERRUPT_PRIORITY \
    (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << \
     (8 - configPRIO_BITS))

#define INCLUDE_vTaskPrioritySet                        1
#define INCLUDE_uxTaskPriorityGet                       1
#define INCLUDE_vTaskDelete                             1
#define INCLUDE_vTaskSuspend                            1
#define INCLUDE_vTaskDelay                              1
#define INCLUDE_vTaskDelayUntil                         1
#define INCLUDE_xTaskGetSchedulerState                  1
#define INCLUDE_xTaskGetCurrentTaskHandle               1

#define configASSERT(expr)              \
    do {                                \
        if ((expr) == 0) {              \
            taskDISABLE_INTERRUPTS();   \
            for (;;) {}                 \
        }                               \
    } while (0);

/* Cortex-M exception handlers used by the FreeRTOS port. */
#define vPortSVCHandler       SVC_Handler
#define xPortPendSVHandler    PendSV_Handler
#define xPortSysTickHandler   SysTick_Handler

#endif
