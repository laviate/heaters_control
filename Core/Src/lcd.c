/*
 * lcd.c
 *
 *  Created on: 15.02.2026 г.
 *      Author: Valio
 */

#include "lcd.h"


// --- Low Level Hardware ---
static void LCD_WriteBus(uint8_t data) {
    HAL_GPIO_WritePin(LCD_D0_PORT, LCD_D0_PIN, (data & 0x01));
    HAL_GPIO_WritePin(LCD_D1_PORT, LCD_D1_PIN, (data & 0x02));
    HAL_GPIO_WritePin(LCD_D2_PORT, LCD_D2_PIN, (data & 0x04));
    HAL_GPIO_WritePin(LCD_D3_PORT, LCD_D3_PIN, (data & 0x08));
    HAL_GPIO_WritePin(LCD_D4_PORT, LCD_D4_PIN, (data & 0x10));
    HAL_GPIO_WritePin(LCD_D5_PORT, LCD_D5_PIN, (data & 0x20));
    HAL_GPIO_WritePin(LCD_D6_PORT, LCD_D6_PIN, (data & 0x40));
    HAL_GPIO_WritePin(LCD_D7_PORT, LCD_D7_PIN, (data & 0x80));
}

static void LCD_PulseEnable(void) {
    HAL_GPIO_WritePin(LCD_EN_PORT, LCD_EN_PIN, GPIO_PIN_SET);
    for(volatile int i=0; i<20; i++);
    HAL_GPIO_WritePin(LCD_EN_PORT, LCD_EN_PIN, GPIO_PIN_RESET);
    for(volatile int i=0; i<20; i++);
}

void LCD_SendCommand(uint8_t cmd) {
    HAL_GPIO_WritePin(LCD_RS_PORT, LCD_RS_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_RW_PORT, LCD_RW_PIN, GPIO_PIN_RESET);
    LCD_WriteBus(cmd);
    LCD_PulseEnable();
    HAL_Delay(2);
}

void LCD_SendData(uint8_t data) {
    HAL_GPIO_WritePin(LCD_RS_PORT, LCD_RS_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LCD_RW_PORT, LCD_RW_PIN, GPIO_PIN_RESET);
    LCD_WriteBus(data);
    LCD_PulseEnable();
    for(volatile int i=0; i<100; i++); // Fast delay
}

// --- API Functions ---
void LCD_Init(void) {
    HAL_GPIO_WritePin(LCD_RW_PORT, LCD_RW_PIN, GPIO_PIN_RESET);
    HAL_Delay(50);
    LCD_SendCommand(0x30); HAL_Delay(5);
    LCD_SendCommand(0x30); HAL_Delay(1);
    LCD_SendCommand(0x30);
    LCD_SendCommand(0x38); // 8-bit, 2 Line
    LCD_SendCommand(0x0C); // Display ON
    LCD_SendCommand(0x01); // Clear
    HAL_Delay(2);
    LCD_SendCommand(0x06); // Increment cursor
}

void LCD_SetCursor(uint8_t row, uint8_t col) {
    LCD_SendCommand((row == 0 ? 0x80 : 0xC0) + col);
}

// Ultra-lean print: just sends bytes directly until \0 is hit
void LCD_PrintRaw(const char *str) {
    while (*str) {
        LCD_SendData(*str++);
    }
}

// Send spaces to clear leftover digits
void LCD_PadSpaces(uint8_t count) {
    while(count--) LCD_SendData(' ');
}

// Integer printer
void LCD_PrintInt(int32_t number) {
    char str[10];
    int i = 0;

    if (number == 0) {
        LCD_SendData('0');
        return;
    }
    if (number < 0) {
        LCD_SendData('-');
        number = -number;
    }
    while (number > 0) {
        str[i++] = (number % 10) + '0';
        number /= 10;
    }
    while (--i >= 0) {
        LCD_SendData(str[i]);
    }
}
