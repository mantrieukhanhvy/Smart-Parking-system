/**
 ******************************************************************************
 * @file    system_stm32f4.c
 * @brief   Minimal SystemInit() for the bare-metal STM32F407 project.
 *
 * The startup file calls SystemInit() before .data/.bss are initialised, so
 * this function must not rely on globals. It is also required by FreeRTOS:
 *   - the Cortex-M4F port executes VFP instructions, so CP10/CP11 must be
 *     enabled before the scheduler starts;
 *   - the FreeRTOS port validates the vector table through VTOR.
 ******************************************************************************
 */
#include <stdint.h>

#include "system_stm32f4.h"

#define SCB_CPACR   ( *( volatile uint32_t * ) 0xE000ED88UL )
#define SCB_VTOR    ( *( volatile uint32_t * ) 0xE000ED08UL )

/* Vector table defined in Startup/startup_stm32f407vgtx.s. Using its address
 * keeps this code correct for both the FLASH and the RAM linker scripts. */
extern uint32_t g_pfnVectors[];

void SystemInit( void )
{
    /* Full access to CP10/CP11 (the FPU). The project is compiled with
     * -mfloat-abi=hard, so any float instruction faults until this is set. */
    SCB_CPACR |= ( 0xFUL << 20 );

    /* Relocate the vector table to the start of this image. */
    SCB_VTOR = ( uint32_t ) g_pfnVectors;
}
