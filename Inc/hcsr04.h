/*
 * hcsr04.h
 *
 *  Created on: Oct 21, 2026
 *      Author: ASUS
 */

#ifndef HCSR04_H_
#define HCSR04_H_
#include "my_stm32f4.h"

void HCSR04_Init(void);
uint8_t HCSR04_TryGetDistance(float *distance_cm);
void HCSR04_Delay(void);


#endif /* HCSR04_H_ */
