#ifndef LED_H_
#define LED_H_

#include "my_stm32f4.h"

#define LED_STATUS_UNKNOWN  0U
#define LED_STATUS_VACANT   1U
#define LED_STATUS_OCCUPIED 2U

void LED_Init(void);
void LED_SetParkingStatus(uint8_t status);

#endif /* LED_H_ */
