#include "led.h"

#define LED_GREEN_PIN 0U
#define LED_RED_PIN 1U
#define LED_ALL_PINS ((1U << LED_GREEN_PIN) | (1U << LED_RED_PIN))

void LED_Init(void) {
    RCC->AHB1ENR |= (1U << 1);

    GPIOB->MODER &= ~((3U << (LED_GREEN_PIN * 2U)) |
                      (3U << (LED_RED_PIN * 2U)));
    GPIOB->MODER |= (1U << (LED_GREEN_PIN * 2U)) |
                    (1U << (LED_RED_PIN * 2U));
    GPIOB->OTYPER &= ~LED_ALL_PINS;
    GPIOB->PUPDR &= ~((3U << (LED_GREEN_PIN * 2U)) |
                      (3U << (LED_RED_PIN * 2U)));
    GPIOB->BSRR = LED_ALL_PINS << 16;
}

void LED_SetParkingStatus(uint8_t status) {
    GPIOB->BSRR = LED_ALL_PINS << 16;

    if (status == LED_STATUS_VACANT) {
        GPIOB->BSRR = (1U << LED_GREEN_PIN);
    } else if (status == LED_STATUS_OCCUPIED) {
        GPIOB->BSRR = (1U << LED_RED_PIN);
    }
}
