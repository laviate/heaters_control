/*
 * lcd.h
 *
 *  Created on: 15.02.2026 г.
 *      Author: Valio
 */

#ifndef INC_LCD_H_
#define INC_LCD_H_

#include <stdint.h>

void LCD_Init(void);
void LCD_Clear(void);
void LCD_SetCursor(uint8_t row, uint8_t col);
void LCD_Print(const char *str);
void LCD_PrintInt(int32_t number);
void LCD_PrintLine(uint8_t row, const char *str);

// Lower level commands if needed
void LCD_SendCommand(uint8_t cmd);
void LCD_SendData(uint8_t data);

#endif /* INC_LCD_H_ */
