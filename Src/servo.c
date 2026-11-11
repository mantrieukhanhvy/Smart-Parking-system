/*
 * servo.c
 *
 *  Created on: Oct 21, 2026
 *      Author: ASUS
 */

#include "FreeRTOS.h"
#include "task.h"
#include "servo.h"

#define SERVO_CLOSED_US 1450U
#define SERVO_OPEN_US   2400U
#define SERVO_STEP_US    10U

static uint16_t pulse_us = SERVO_CLOSED_US;

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
    TIM3->CCR1 = SERVO_CLOSED_US;
    TIM3->CR1 |= (1 << 0);
}

void Servo_SetAngle(uint8_t open_gate)
{
    const uint16_t target_us = open_gate ? SERVO_OPEN_US : SERVO_CLOSED_US;

    if (pulse_us < target_us) {
        const uint16_t remaining = target_us - pulse_us;
        while (pulse_us < target_us){
        pulse_us += SERVO_STEP_US;
        TIM3->CCR1 = pulse_us;
        vTaskDelay(pdMS_TO_TICKS(20));
        }
    } else if (pulse_us > target_us) {
        const uint16_t remaining = pulse_us - target_us;
        pulse_us -= (remaining < SERVO_STEP_US) ? remaining : SERVO_STEP_US;
    }

    TIM3->CCR1 = pulse_us;
}
