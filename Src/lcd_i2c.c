/*
 * lcd_i2c.c
 *
 *  Created on: Oct 21, 2026
 *      Author: ASUS
 */


#include "lcd_i2c.h"

// Initialize I2C1 hardware on pins PB6 and PB7
void I2C1_Init(void) {
    // 1. Enable peripheral clocks
    RCC->AHB1ENR |= (1 << 1);  // Enable GPIOB clock
    RCC->APB1ENR |= (1 << 21); // Enable I2C1 clock

    // 2. Configure PB6 and PB7 as Alternate Function (AF4)
    GPIOB->MODER &= ~((3U << 12) | (3U << 14));
    GPIOB->MODER |= ((2U << 12) | (2U << 14)); // Set Alternate Function mode

    GPIOB->OTYPER |= ((1 << 6) | (1 << 7));    // Configure as Open-Drain (required for I2C)
    GPIOB->PUPDR &= ~((3U << 12) | (3U << 14));
    GPIOB->PUPDR |= ((1U << 12) | (1U << 14)); // Enable pull-up resistors

    GPIOB->AFRL &= ~((0xFU << 24) | (0xFU << 28));
    GPIOB->AFRL |= ((4U << 24) | (4U << 28));  // Select AF4 for I2C1

    // 3. Configure I2C1 (Standard Mode, 100 kHz)
    I2C1->CR1 = (1 << 15); // Reset I2C
    I2C1->CR1 = 0;         // Release reset

    I2C1->CR2 = 16;        // APB1 clock frequency is 16 MHz
    I2C1->CCR = 80;        // Set speed to 100 kHz (16 MHz / (2 * 100 kHz) = 80)
    I2C1->TRISE = 17;      // Set maximum rise time (16 + 1)

    I2C1->CR1 |= (1 << 0); // Enable the I2C peripheral (PE = 1)
}

// Transmit one byte over I2C
void I2C_Write(uint8_t data) {
    // Wait until the bus is no longer busy
    while (I2C1->SR2 & (1 << 1));

    I2C1->CR1 |= (1 << 8); // Generate START condition
    while (!(I2C1->SR1 & (1 << 0))); // Wait for the SB (Start Bit) flag

    I2C1->DR = LCD_ADDR; // Send the LCD address
    while (!(I2C1->SR1 & (1 << 1))); // Wait for the ADDR flag
    uint32_t temp = I2C1->SR2; // Read SR2 to clear the ADDR flag
    (void)temp;

    I2C1->DR = data; // Send the data byte
    while (!(I2C1->SR1 & (1 << 7))); // Wait for the TxE (Transmit Data Register Empty) flag

    I2C1->CR1 |= (1 << 9); // Generate STOP condition
}

// Send data to the PCF8574 chip
void LCD_Write_Nibble(uint8_t nibble, uint8_t rs) {
    uint8_t data = (nibble << 4) | (rs << 0) | (1 << 3); // Include the backlight flag (bit 3)
    I2C_Write(data | (1 << 2)); // Set the Enable (EN) pin to 1
    for(volatile int i=0; i<5000; i++); // Short delay
    I2C_Write(data & ~(1 << 2)); // Set the Enable (EN) pin to 0
    for(volatile int i=0; i<5000; i++);
}

void LCD_Command(uint8_t cmd) {
    LCD_Write_Nibble(cmd >> 4, 0); // Send the upper 4 bits (RS = 0)
    LCD_Write_Nibble(cmd & 0x0F, 0); // Send the lower 4 bits (RS = 0)
}

void LCD_Data(uint8_t data) {
    LCD_Write_Nibble(data >> 4, 1); // Send the upper 4 bits (RS = 1)
    LCD_Write_Nibble(data & 0x0F, 1); // Send the lower 4 bits (RS = 1)
}

void LCD_Init(void) {
    I2C1_Init();
    for(volatile int i=0; i<50000; i++); // Wait for the LCD to initialize

    // HD44780 4-bit initialization sequence
    LCD_Write_Nibble(0x03, 0);
    for(volatile int i=0; i<50000; i++);
    LCD_Write_Nibble(0x03, 0);
    LCD_Write_Nibble(0x03, 0);
    LCD_Write_Nibble(0x02, 0); // Switch to 4-bit mode

    LCD_Command(0x28); // Configure 2 lines with a 5x8 character font
    LCD_Command(0x0C); // Turn on the display and turn off the cursor
    LCD_Command(0x01); // Clear the display
    for(volatile int i=0; i<50000; i++);
}

void LCD_SetCursor(uint8_t row, uint8_t col) {
    uint8_t addr = (row == 0) ? 0x80 : 0xC0;
    LCD_Command(addr + col);
}

void LCD_Print(char *str) {
    while (*str) {
        LCD_Data(*str++);
    }
}

void LCD_Clear(void) {
    LCD_Command(0x01);
    for(volatile int i=0; i<50000; i++);
}

void LCD_PrintInt(int num) {
    char buffer[10];
    int i = 0, isNegative = 0;
    if (num == 0) { LCD_Data('0'); return; }
    if (num < 0) { isNegative = 1; num = -num; }
    while (num != 0) {
        buffer[i++] = (num % 10) + '0';
        num = num / 10;
    }
    if (isNegative) buffer[i++] = '-';
    while (i > 0) LCD_Data(buffer[--i]);
}

