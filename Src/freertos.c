/**
 ******************************************************************************
 * @file    freertos.c
 * @brief   FreeRTOS for smart parking project.
 *
 *
 ******************************************************************************
 */
#include "freertos_init.h"

#include "FreeRTOS.h"
#include "task.h"

/* Stack size is in words (1 word = 4 bytes on Cortex-M4). */
#define DEFAULT_TASK_STACK_WORDS    128U
#define DEFAULT_TASK_PRIORITY       1U
#define DEFAULT_TASK_PERIOD_MS      1000U

static void StartDefaultTask(void *argument)
{
    (void)argument;

    for (;;)
    {
        vTaskDelay(pdMS_TO_TICKS(DEFAULT_TASK_PERIOD_MS));
    }
}

void FreeRTOS_Init(void)
{
    BaseType_t created;

    created = xTaskCreate(StartDefaultTask,
                          "Default",
                          DEFAULT_TASK_STACK_WORDS,
                          NULL,
                          DEFAULT_TASK_PRIORITY,
                          NULL);
    configASSERT(created == pdPASS);

    vTaskStartScheduler();

    /* Only reached if the scheduler could not start, normally because the
     * idle task could not be allocated from the FreeRTOS heap. */
}
