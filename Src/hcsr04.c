/*
 * hcsr04.c
 *
 *  Created on: Oct 21, 2026
 *      Author: ASUS
 */
#include "hcsr04.h"

#define HCSR04_ECHO_TIMEOUT_US 30000U

void HCSR04_Init(void) {
    RCC->AHB1ENR |= (1 << 4); // Turn on clock Port E
    RCC->APB1ENR |= (1 << 0); // Turn on clock TIM2

    // PE9 (Trig) -> Output, PE10 (Echo) -> Input
    GPIOE->MODER &= ~((3U << 18) | (3U << 20));
    GPIOE->MODER |= (1U << 18);

    // Assumes TIM2 input clock is 16 MHz.
    TIM2->CR1 = 0U;           // Stop timer; upcounting, updates enabled
    TIM2->PSC = 16U - 1U;     // Divide by 16 -> 1 MHz
    TIM2->ARR = 0xFFFFFFFFU;

    TIM2->EGR = 1U;           // UG: apply the prescaler immediately
    TIM2->SR = 0U;            // Clear flags raised by the update
    TIM2->CNT = 0U;
    TIM2->CR1 = 1U;           // Start timer
}

uint8_t HCSR04_TryGetDistance(float *distance_cm) {
    uint32_t time_us = 0;

    if (distance_cm == 0) {
        return 0;
    }

    GPIOE->BSRR = (1 << 9);
    TIM2->CNT = 0;
    while (TIM2->CNT < 10);
    GPIOE->BSRR = (1 << (9 + 16));

    TIM2->CNT = 0;
    while (!(GPIOE->IDR & (1 << 10))) {
        if (TIM2->CNT >= HCSR04_ECHO_TIMEOUT_US) {
            return 0;
        }
    }

    TIM2->CNT = 0;
    while (GPIOE->IDR & (1 << 10)) {
        if (TIM2->CNT >= HCSR04_ECHO_TIMEOUT_US) {
            return 0;
        }
    }

    time_us = TIM2->CNT;
    *distance_cm = (time_us * 0.0343f) / 2.0f;
    return 1;
}

void HCSR04_Delay(void) {
    //Use TIM2 to delay 100ms debouncing
    TIM2->CNT = 0;
    while (TIM2->CNT < 100000);
}
