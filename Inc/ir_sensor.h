/*
 * ir_sensor.h
 *
 *  Created on: Oct 21, 2026
 *      Author: ASUS
 */

#ifndef IR_SENSOR_H_
#define IR_SENSOR_H_
#include "my_stm32f4.h"

void IR_Sensor_Init(void);
uint8_t IR_Sensor_Read(void);


#endif /* IR_SENSOR_H_ */
