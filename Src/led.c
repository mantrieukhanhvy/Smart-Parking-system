#include "led.h"

#define LED_GREEN_PIN 12U
#define LED_RED_PIN 14U
#define LED_ALL_PINS ((1U << LED_GREEN_PIN) | (1U << LED_RED_PIN))

void LED_Init(void) {
    RCC->AHB1ENR |= (1U << 3);

    GPIOD->MODER &= ~((3U << (LED_GREEN_PIN * 2U)) |
                      (3U << (LED_RED_PIN * 2U)));
    GPIOD->MODER |= (1U << (LED_GREEN_PIN * 2U)) |
                    (1U << (LED_RED_PIN * 2U));
    GPIOD->OTYPER &= ~LED_ALL_PINS;
    GPIOD->PUPDR &= ~((3U << (LED_GREEN_PIN * 2U)) |
                      (3U << (LED_RED_PIN * 2U)));
    GPIOD->BSRR = LED_ALL_PINS << 16;
}

void LED_SetParkingStatus(uint8_t status) {
    GPIOD->BSRR = LED_ALL_PINS << 16;

    if (status == LED_STATUS_VACANT) {
        GPIOD->BSRR = (1U << LED_GREEN_PIN);
    } else if (status == LED_STATUS_OCCUPIED) {
        GPIOD->BSRR = (1U << LED_RED_PIN);
    }
}
