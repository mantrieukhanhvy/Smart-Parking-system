/*
 * hcsr04.c
 *
 *  Created on: Oct 21, 2026
 *      Author: ASUS
 */
#include "hcsr04.h"

void HCSR04_Init(void) {
    RCC->AHB1ENR |= (1 << 4); // Turn on clock Port E
    RCC->APB1ENR |= (1 << 0); // Turn on clock TIM2

    // PE9 (Trig) -> Output, PE10 (Echo) -> Input
    GPIOE->MODER &= ~((3U << 18) | (3U << 20));
    GPIOE->MODER |= (1U << 18);

    // Config TIM2 (1 us/tick)
    TIM2->PSC = 16 - 1;
    TIM2->ARR = 0xFFFFFFFF;
    TIM2->CR1 |= (1 << 0);
}

float HCSR04_GetDistance(void) {
    uint32_t time_us = 0;

    // Pulse 10us
    GPIOE->BSRR = (1 << 9);
    TIM2->CNT = 0;
    while (TIM2->CNT < 10);
    GPIOE->BSRR = (1 << (9 + 16));

    // Measure time Echo
    while (!(GPIOE->IDR & (1 << 10)));
    TIM2->CNT = 0;
    while ((GPIOE->IDR & (1 << 10)));

    time_us = TIM2->CNT;
    return (time_us * 0.0343f) / 2.0f;
}

void HCSR04_Delay(void) {
    //Use TIM2 to delay 100ms debouncing
    TIM2->CNT = 0;
    while (TIM2->CNT < 100000);
}

