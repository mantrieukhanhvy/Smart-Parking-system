#ifndef MY_STM32F4_H
#define MY_STM32F4_H

#include <stdint.h>

/* 1. DEFINE BASE ADDRESSES (Refer to Section 2.3: Memory Map) */
#define RCC_BASE      0x40023800UL
#define GPIOA_BASE    0x40020000UL
#define GPIOB_BASE    0x40020400UL
#define GPIOD_BASE    0x40020C00UL
#define GPIOE_BASE    0x40021000UL
#define TIM2_BASE     0x40000000UL
#define TIM3_BASE     0x40000400UL
#define I2C1_BASE     0x40005400UL

/* 2. DEFINE THE GPIO REGISTER STRUCTURE (Refer to Section 8.4: GPIO Registers) */
typedef struct {
    volatile uint32_t MODER;    // Offset: 0x00
    volatile uint32_t OTYPER;   // Offset: 0x04
    volatile uint32_t OSPEEDR;  // Offset: 0x08
    volatile uint32_t PUPDR;    // Offset: 0x0C
    volatile uint32_t IDR;      // Offset: 0x10
    volatile uint32_t ODR;      // Offset: 0x14
    volatile uint32_t BSRR;     // Offset: 0x18
    volatile uint32_t LCKR;     // 0x1C
    volatile uint32_t AFRL;     // 0x20
    volatile uint32_t AFRH;     // 0x24
} GPIO_TypeDef;

/* 3. DEFINE THE RCC REGISTER STRUCTURE (Refer to Section 7.3: RCC Registers) */
typedef struct {
	volatile uint32_t CR;       // 0x00
	    volatile uint32_t PLLCFGR;  // 0x04
	    volatile uint32_t CFGR;     // 0x08
	    volatile uint32_t CIR;      // 0x0C
	    volatile uint32_t AHB1RSTR; // 0x10
	    volatile uint32_t AHB2RSTR; // 0x14
	    volatile uint32_t AHB3RSTR; // 0x18
	    uint32_t RESERVED0;         // 0x1C
	    volatile uint32_t APB1RSTR; // 0x20
	    volatile uint32_t APB2RSTR; // 0x24
	    uint32_t RESERVED1[2];      // 0x28-0x2C
	    volatile uint32_t AHB1ENR;  // 0x30
	    volatile uint32_t AHB2ENR;  // 0x34
	    volatile uint32_t AHB3ENR;  // 0x38
	    uint32_t RESERVED2;         // 0x3C
	    volatile uint32_t APB1ENR;  // 0x40
} RCC_TypeDef;

/* DEFINE THE TIM2 STRUCTURE */

typedef struct {
    volatile uint32_t CR1;      // 0x00
    volatile uint32_t CR2;      // 0x04
    volatile uint32_t SMCR;     // 0x08
    volatile uint32_t DIER;     // 0x0C
    volatile uint32_t SR;       // 0x10
    volatile uint32_t EGR;      // 0x14
    volatile uint32_t CCMR1;    // 0x18
    volatile uint32_t CCMR2;    // 0x1C
    volatile uint32_t CCER;     // 0x20
    volatile uint32_t CNT;      // 0x24 (counter)
    volatile uint32_t PSC;      // 0x28 (frequency divider)
    volatile uint32_t ARR;      // 0x2C (max count)
    volatile uint32_t RCR;      // 0x30
    volatile uint32_t CCR1;     // 0x34
    volatile uint32_t CCR2;     // 0x38
    volatile uint32_t CCR3;     // 0x3C
    volatile uint32_t CCR4;     // 0x40
} TIM_TypeDef;

typedef struct {
    volatile uint32_t CR1;      // 0x00
    volatile uint32_t CR2;      // 0x04
    volatile uint32_t OAR1;     // 0x08
    volatile uint32_t OAR2;     // 0x0C
    volatile uint32_t DR;       // 0x10
    volatile uint32_t SR1;      // 0x14
    volatile uint32_t SR2;      // 0x18
    volatile uint32_t CCR;      // 0x1C
    volatile uint32_t TRISE;    // 0x20
    volatile uint32_t FLTR;     // 0x24
} I2C_TypeDef;

/* 4. CAST THE BASE ADDRESSES TO STRUCT POINTERS */
#define RCC    ((RCC_TypeDef *) RCC_BASE)
#define GPIOA  ((GPIO_TypeDef *) GPIOA_BASE)
#define GPIOB  ((GPIO_TypeDef *) GPIOB_BASE)
#define GPIOD  ((GPIO_TypeDef *) GPIOD_BASE)
#define GPIOE  ((GPIO_TypeDef *) GPIOE_BASE)
#define TIM2   ((TIM_TypeDef *) TIM2_BASE)
#define TIM3   ((TIM_TypeDef *) TIM3_BASE)
#define I2C1   ((I2C_TypeDef *) I2C1_BASE)

#endif /* MY_STM32F4_H */
