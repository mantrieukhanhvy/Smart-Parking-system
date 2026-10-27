/*
 * lcd_i2c.h
 *
 *  Created on: Oct 21, 2026
 *      Author: ASUS
 */

#ifndef LCD_I2C_H_
#define LCD_I2C_H_
#include "my_stm32f4.h"

#define LCD_ADDR 0x4E

void I2C1_Init(void);
void LCD_Init(void);
void LCD_Clear(void);
void LCD_SetCursor(uint8_t row, uint8_t col);
void LCD_Print(char *str);
void LCD_PrintInt(int num);


#endif /* LCD_I2C_H_ */
