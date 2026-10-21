/*
 * servo.c
 *
 *  Created on: Oct 21, 2026
 *      Author: ASUS
 */

#include "servo.h"

void Servo_Init(void) {
    RCC->AHB1ENR |= (1 << 0); // Port A
    RCC->APB1ENR |= (1 << 1); // TIM3

    // PA6 for Alternate Function (AF2)
    GPIOA->MODER &= ~(3U << 12);
    GPIOA->MODER |= (2U << 12);
    GPIOA->AFRL &= ~(0xFU << 24);
    GPIOA->AFRL |= (2U << 24);

    // TIM3 craete PWM 50Hz
    TIM3->PSC = 16 - 1;
    TIM3->ARR = 20000 - 1;

    TIM3->CCMR1 &= ~(7U << 4);
    TIM3->CCMR1 |= (6U << 4);
    TIM3->CCMR1 |= (1U << 3);

    TIM3->CCER |= (1 << 0);
    TIM3->CR1 |= (1 << 0);
}

void Servo_SetAngle(uint8_t open_gate) {
    if (open_gate == 1) {
        TIM3->CCR1 = 1500; // open 90 degree
    } else {
        TIM3->CCR1 = 1000; // close 0 degree
    }
}
