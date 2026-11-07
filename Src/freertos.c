/**
 ******************************************************************************
 * @file    freertos.c
 * @brief   FreeRTOS for smart parking project.
 *
 *
 ******************************************************************************
 */
#include "freertos_init.h"
#include "tasks.h"

#include "FreeRTOS.h"
#include "task.h"

void FreeRTOS_Init(void)
{
    Create_Parking_Tasks();

    vTaskStartScheduler();

    /* Only reached if the scheduler could not start, normally because the
     * idle task could not be allocated from the FreeRTOS heap. */
    for (;;) {
    }
}
