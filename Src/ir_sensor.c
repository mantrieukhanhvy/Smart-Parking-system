/*
 * ir_sensor.c
 *
 *  Created on: Oct 21, 2026
 *      Author: ASUS
 */

#include "ir_sensor.h"

void IR_Sensor_Init(void) {
    RCC->AHB1ENR |= (1 << 4);    // Turn on clock Port E
    GPIOE->MODER &= ~(3U << 16); //  PE8 Input
}

uint8_t IR_Sensor_Read(void) {
    // 1 vehicle is detectect
    if ((GPIOE->IDR & (1 << 8)) == 0) {
        return 1;
    }
    return 0;
}
