#ifndef MY_STM32F4_H
#define MY_STM32F4_H

#include <stdint.h>

/* 1. DEFINE BASE ADDRESSES (Refer to Section 2.3: Memory Map) */
#define RCC_BASE      0x40023800UL
#define GPIOE_BASE    0x40021000UL

/* 2. DEFINE THE GPIO REGISTER STRUCTURE (Refer to Section 8.4: GPIO Registers) */
typedef struct {
    volatile uint32_t MODER;    // Offset: 0x00
    volatile uint32_t OTYPER;   // Offset: 0x04
    volatile uint32_t OSPEEDR;  // Offset: 0x08
    volatile uint32_t PUPDR;    // Offset: 0x0C
    volatile uint32_t IDR;      // Offset: 0x10
    volatile uint32_t ODR;      // Offset: 0x14
    // Registers BSRR, LCKR, AFR, etc. are omitted for simplicity.
} GPIO_TypeDef;

/* 3. DEFINE THE RCC REGISTER STRUCTURE (Refer to Section 7.3: RCC Registers) */
typedef struct {
    volatile uint32_t CR;       // Offset: 0x00
    volatile uint32_t PLLCFGR;  // Offset: 0x04
    volatile uint32_t CFGR;     // Offset: 0x08
    volatile uint32_t CIR;      // Offset: 0x0C
    volatile uint32_t AHB1RSTR; // Offset: 0x10
    volatile uint32_t AHB2RSTR; // Offset: 0x14
    volatile uint32_t AHB3RSTR; // Offset: 0x18
    uint32_t RESERVED0;         // Offset: 0x1C (Reserved space)
    volatile uint32_t APB1RSTR; // Offset: 0x20
    volatile uint32_t APB2RSTR; // Offset: 0x24
    uint32_t RESERVED1[2];      // Offsets: 0x28–0x2C
    volatile uint32_t AHB1ENR;  // Offset: 0x30
} RCC_TypeDef;

/* 4. CAST THE BASE ADDRESSES TO STRUCT POINTERS */
#define RCC    ((RCC_TypeDef *) RCC_BASE)
#define GPIOE  ((GPIO_TypeDef *) GPIOE_BASE)

#endif /* MY_STM32F4_H */
