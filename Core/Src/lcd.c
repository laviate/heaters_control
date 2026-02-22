/*
 * lcd.c
 *
 *  Created on: 15.02.2026 г.
 *      Author: Valio
 */

#include "lcd.h"
#include "lcd_config.h"

// Helper to set all 8 pins
static void LCD_WriteBus(uint8_t data) {
    HAL_GPIO_WritePin(LCD_D0_PORT, LCD_D0_PIN, (data & 0x01));
    HAL_GPIO_WritePin(LCD_D1_PORT, LCD_D1_PIN, (data & 0x02));
    HAL_GPIO_WritePin(LCD_D2_PORT, LCD_D2_PIN, (data & 0x04));
    HAL_GPIO_WritePin(LCD_D3_PORT, LCD_D3_PIN, (data & 0x08));
    HAL_GPIO_WritePin(LCD_D4_PORT, LCD_D4_PIN, (data & 0x10));
    HAL_GPIO_WritePin(LCD_D5_PORT, LCD_D5_PIN, (data & 0x20));
    HAL_GPIO_WritePin(LCD_D6_PORT, LCD_D6_PIN, (data & 0x40));
    HAL_GPIO_WritePin(LCD_D7_PORТ, LCD_D7_PIN, (data & 0x80));
}

// The latch signal - CRITICAL
static void LCD_PulseEnable(void) {
    HAL_GPIO_WritePin(LCD_EN_PORT, LCD_EN_PIN, GPIO_PIN_SET);
    for(volatile int i=0; i<100; i++); // Wait > 450ns
    HAL_GPIO_WritePin(LCD_EN_PORT, LCD_EN_PIN, GPIO_PIN_RESET); // Falling edge latches data
    for(volatile int i=0; i<100; i++); // Wait for execution
}

void LCD_SendCommand(uint8_t cmd) {
    HAL_GPIO_WritePin(LCD_RS_PORT, LCD_RS_PIN, GPIO_PIN_RESET); // RS=0 Command
    HAL_GPIO_WritePin(LCD_RW_PORT, LCD_RW_PIN, GPIO_PIN_RESET); // RW=0 Write
    LCD_WriteBus(cmd);
    LCD_PulseEnable();
    HAL_Delay(2);
}

void LCD_SendData(uint8_t data) {
    HAL_GPIO_WritePin(LCD_RS_PORT, LCD_RS_PIN, GPIO_PIN_SET);   // RS=1 Data
    HAL_GPIO_WritePin(LCD_RW_PORT, LCD_RW_PIN, GPIO_PIN_RESET); // RW=0 Write
    LCD_WriteBus(data);
    LCD_PulseEnable();
    for(volatile int i=0; i<500; i++); // Data is fast (~37us)
}

void LCD_Init(void) {
    // Force RW low
    HAL_GPIO_WritePin(LCD_RW_PORT, LCD_RW_PIN, GPIO_PIN_RESET);
    HAL_Delay(50);

    // Standard 8-bit initialization sequence
    LCD_SendCommand(0x30);
    HAL_Delay(5);
    LCD_SendCommand(0x30);
    HAL_Delay(1);
    LCD_SendCommand(0x30);

    // Function Set: 8-bit, 2 Line, 5x8 Font
    LCD_SendCommand(0x38);

    // Display ON, Cursor OFF, Blink OFF
    LCD_SendCommand(0x0C);

    // Clear Display
    LCD_SendCommand(0x01);
    HAL_Delay(2);

    // Entry Mode: Increment cursor automatically
    LCD_SendCommand(0x06);
}

void LCD_SetCursor(uint8_t row, uint8_t col) {
    // 16x2 addressing: Line 1 = 0x80, Line 2 = 0xC0
    if (row == 0)
        LCD_SendCommand(0x80 + col);
    else
        LCD_SendCommand(0xC0 + col);
}

void LCD_Clear(void) {
    LCD_SendCommand(0x01);
    HAL_Delay(2);
}

// Simple internal Cyrillic mapper (Winstar Standard)
static uint8_t Map_Cyrillic(uint16_t wide_char) {
    switch (wide_char) {
        case 0xD091: return 0xA0; // Б
        case 0xD093: return 0xA1; // Г
        case 0xD094: return 0xE0; // Д
        case 0xD096: return 0xA3; // Ж
        case 0xD097: return 0xA4; // З
        case 0xD098: return 0xA5; // И
        case 0xD099: return 0xA6; // Й
        case 0xD09B: return 0xA7; // Л
        case 0xD09F: return 0xA8; // П
        case 0xD0A4: return 0xA9; // Ф
        case 0xD0A6: return 0xE1; // Ц
        case 0xD0A7: return 0xAB; // Ч
        case 0xD0A8: return 0xAC; // Ш
        case 0xD0A9: return 0xE2; // Щ
        case 0xD0AA: return 0xAD; // Ъ
        case 0xD0AB: return 0xAE; // Ы
        case 0xD0AC: return 0x62; // ь
        case 0xD0AD: return 0xB0; // Э
        case 0xD0AE: return 0xB1; // Ю
        case 0xD0AF: return 0xB2; // Я
        default: return 0x2A;     // *
    }
}

void LCD_Print(const char *str) {
    while (*str) {
        uint8_t c = *str;
        // Detect UTF-8 Cyrillic (starts with 0xD0 or 0xD1)
        if (c == 0xD0 || c == 0xD1) {
            uint8_t next = *(str + 1);
            if (next) {
                uint16_t wide = (c << 8) | next;

                // Homoglyph fix (Map Cyrillic A -> Latin A, etc.)
                if (wide == 0xD090) LCD_SendData('A');
                else if (wide == 0xD092) LCD_SendData('B');
                else if (wide == 0xD095) LCD_SendData('E');
                else if (wide == 0xD09A) LCD_SendData('K');
                else if (wide == 0xD09C) LCD_SendData('M');
                else if (wide == 0xD09D) LCD_SendData('H');
                else if (wide == 0xD09E) LCD_SendData('O');
                else if (wide == 0xD0A0) LCD_SendData('P');
                else if (wide == 0xD0A1) LCD_SendData('C');
                else if (wide == 0xD0A2) LCD_SendData('T');
                else if (wide == 0xD0A3) LCD_SendData('y');
                else if (wide == 0xD0A5) LCD_SendData('X');
                else LCD_SendData(Map_Cyrillic(wide));

                str += 2;
            } else str++;
        } else {
            LCD_SendData(c);
            str++;
        }
    }
}

void LCD_PrintInt(int32_t number) {
    char str[12]; // Local stack buffer, only exists during this function call
    int i = 0;

    if (number == 0) {
        LCD_SendData('0');
        return;
    }

    if (number < 0) {
        LCD_SendData('-');
        number = -number;
    }

    // Extract digits in reverse order
    while (number > 0) {
        str[i++] = (number % 10) + '0'; // Convert digit to ASCII
        number /= 10;
    }

    // Send to LCD in correct order
    while (--i >= 0) {
        LCD_SendData(str[i]);
    }
}

void LCD_PrintLine(uint8_t row, const char *str){
	LCD_SetCursor(row, 0);
	uint8_t count = 0;

	while (*str && count < 16){
		LCD_SendData(*str++);
		count++;
	}
	while (count < 16){
		LCD_SendData(' ');
		count++;
	}
}
