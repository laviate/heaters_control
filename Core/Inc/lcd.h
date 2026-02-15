/*
 * lcd.h
 *
 *  Created on: 15.02.2026 г.
 *      Author: Valio
 */

#ifndef INC_LCD_H_
#define INC_LCD_H_

#include <stdint.h>

typedef enum {
    LCD_TYPE_16x2,
    LCD_TYPE_8x2,
    LCD_TYPE_16x1_SPLIT // The WH1601B
} LCD_Type_t;

void LCD_Init(LCD_Type_t type);
void LCD_SendCommand(uint8_t cmd);
void LCD_SendData(uint8_t data);
void LCD_SetCursor(uint8_t row, uint8_t col);
void LCD_Print(const char *str); // Handles UTF-8 Cyrillic automatically


#endif /* INC_LCD_H_ */
