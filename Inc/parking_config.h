#ifndef PARKING_CONFIG_H_
#define PARKING_CONFIG_H_

#include "my_stm32f4.h"

#define PARKING_DEFAULT_CAPACITY 1U

extern uint32_t parking_capacity;

uint8_t ParkingConfig_Init(uint32_t *capacity);

#endif /* PARKING_CONFIG_H_ */
