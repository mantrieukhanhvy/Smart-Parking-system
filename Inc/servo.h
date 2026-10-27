/*
 * servo.h
 *
 *  Created on: Oct 21, 2026
 *      Author: ASUS
 */

#ifndef SERVO_H_
#define SERVO_H_
#include "my_stm32f4.h"

void Servo_Init(void);
void Servo_SetAngle(uint8_t open_gate);

#endif /* SERVO_H_ */
