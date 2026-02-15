/*
 * lcd.c
 *
 *  Created on: 15.02.2026 г.
 *      Author: Valio
 */

#include "lcd.h"
#include "lcd_config.h"
#include <string.h>

// Internal state to track display type
static uint8_t _is_16x1_split = 0;

// Helper: 8-bit bus write
static void LCD_WriteBus(uint8_t data) {
    HAL_GPIO_WritePin(LCD_DATA_PORT_0, LCD_DATA_PIN_0, (data & 0x01));
    HAL_GPIO_WritePin(LCD_DATA_PORT_1, LCD_DATA_PIN_1, (data & 0x02));
    HAL_GPIO_WritePin(LCD_DATA_PORT_2, LCD_DATA_PIN_2, (data & 0x04));
    HAL_GPIO_WritePin(LCD_DATA_PORT_3, LCD_DATA_PIN_3, (data & 0x08));
    HAL_GPIO_WritePin(LCD_DATA_PORT_4, LCD_DATA_PIN_4, (data & 0x10));
    HAL_GPIO_WritePin(LCD_DATA_PORT_5, LCD_DATA_PIN_5, (data & 0x20));
    HAL_GPIO_WritePin(LCD_DATA_PORT_6, LCD_DATA_PIN_6, (data & 0x40));
    HAL_GPIO_WritePin(LCD_DATA_PORT_7, LCD_DATA_PIN_7, (data & 0x80));
}

static void LCD_PulseEnable(void) {
    HAL_GPIO_WritePin(LCD_EN_PORT, LCD_EN_PIN, GPIO_PIN_SET);
    // Enable pulse width must be > 450ns.
    // At STM32 speeds, a few NOPs or a tiny loop is enough.
    for(volatile int i=0; i<50; i++);
    HAL_GPIO_WritePin(LCD_EN_PORT, LCD_EN_PIN, GPIO_PIN_RESET);
}

void LCD_SendCommand(uint8_t cmd) {
    HAL_GPIO_WritePin(LCD_RS_PORT, LCD_RS_PIN, GPIO_PIN_RESET); // RS Low = Command
    HAL_GPIO_WritePin(LCD_RW_PORT, LCD_RW_PIN, GPIO_PIN_RESET); // RW Low = Write
    LCD_WriteBus(cmd);
    LCD_PulseEnable();
    HAL_Delay(2); // Commands need time
}

void LCD_SendData(uint8_t data) {
    HAL_GPIO_WritePin(LCD_RS_PORT, LCD_RS_PIN, GPIO_PIN_SET);   // RS High = Data
    HAL_GPIO_WritePin(LCD_RW_PORT, LCD_RW_PIN, GPIO_PIN_RESET); // RW Low = Write
    LCD_WriteBus(data);
    LCD_PulseEnable();
    // Data writes are faster, usually 40us, but HAL_Delay(1) is safe
    for(volatile int i=0; i<500; i++);
}

// Initialize with a type parameter
void LCD_Init(LCD_Type_t type) {
    // 1. Force RW Low just in case
    HAL_GPIO_WritePin(LCD_RW_PORT, LCD_RW_PIN, GPIO_PIN_RESET);

    // 2. Initialization Sequence for 8-bit
    HAL_Delay(50);
    LCD_SendCommand(0x30);
    HAL_Delay(5);
    LCD_SendCommand(0x30);
    HAL_Delay(1);
    LCD_SendCommand(0x30);

    // 3. Function Set: 8-bit, 2-line, 5x8 dots
    // Note: Even 16x1 displays usually need "2-line" mode to address the right half
    LCD_SendCommand(0x38);

    LCD_SendCommand(0x0C); // Display ON, Cursor OFF
    LCD_SendCommand(0x01); // Clear
    HAL_Delay(2);
    LCD_SendCommand(0x06); // Entry mode: Increment

    if (type == LCD_TYPE_16x1_SPLIT) {
        _is_16x1_split = 1;
    } else {
        _is_16x1_split = 0;
    }
}

void LCD_SetCursor(uint8_t row, uint8_t col) {
    uint8_t addr = 0;

    if (_is_16x1_split) {
        // For the 16x1 split, everything is visually on "Row 0",
        // but electrically chars 8-15 are at address 0x40 (Line 2)
        if (col < 8) addr = 0x80 + col;
        else addr = 0x80 + 0x40 + (col - 8);
    }
    else {
        // Standard 16x2 or 8x2
        if (row == 0) addr = 0x80 + col;
        else addr = 0x80 + 0x40 + col;
    }
    LCD_SendCommand(addr);
}

// --- Cyrillic Mapping ---
// Maps a UTF-8 wide character to the Winstar LCD byte
// This table assumes standard Winstar Cyrillic ROM.
// If characters look wrong, this table needs adjustment based on the datasheet.
uint8_t Map_Cyrillic(uint16_t utf8_char) {
    // Case 1: Homoglyphs (Look like Latin) - Map to standard ASCII
    // A, B, E, K, M, H, O, P, C, T, Y, X
    // (This saves defining them in the switch case)

    switch (utf8_char) {
        // Upper Case
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
        case 0xD0AC: return 0x62; // ь (soft sign often mapped to 'b' or specific code)
        case 0xD0AD: return 0xB0; // Э
        case 0xD0AE: return 0xB1; // Ю
        case 0xD0AF: return 0xB2; // Я

        // Lower Case (Map to Upper Case or closest available)
        // Many 1602 LCDs only have Upper Case Cyrillic in ROM
        case 0xD0B1: return 0xA0; // б -> Б
        case 0xD0B4: return 0xE0; // д -> Д
        // ... add others as needed

        default: return 0x2A; // Return '*' for unknown
    }
}

void LCD_Print(const char *str) {
    while (*str) {
        uint8_t c = *str;

        // Check for UTF-8 Cyrillic leader byte (0xD0 or 0xD1)
        if (c == 0xD0 || c == 0xD1) {
            uint8_t next = *(str + 1);
            if (next) {
                uint16_t wide = (c << 8) | next;

                // Check if it's a homoglyph (letters that look identical to Latin)
                // A (0x90), E (0x95), etc.
                // Simple optimization: If your ROM has full cyrillic, use Map_Cyrillic.
                // If standard Winstar, use Latin for A, E, T, O, etc.
                if (wide == 0xD090) LCD_SendData('A'); // А
                else if (wide == 0xD092) LCD_SendData('B'); // В
                else if (wide == 0xD095) LCD_SendData('E'); // Е
                else if (wide == 0xD09A) LCD_SendData('K'); // К
                else if (wide == 0xD09C) LCD_SendData('M'); // М
                else if (wide == 0xD09D) LCD_SendData('H'); // Н
                else if (wide == 0xD09E) LCD_SendData('O'); // О
                else if (wide == 0xD0A0) LCD_SendData('P'); // Р
                else if (wide == 0xD0A1) LCD_SendData('C'); // С
                else if (wide == 0xD0A2) LCD_SendData('T'); // Т
                else if (wide == 0xD0A3) LCD_SendData('y'); // У (looks like y)
                else if (wide == 0xD0A5) LCD_SendData('X'); // Х
                else LCD_SendData(Map_Cyrillic(wide));

                str += 2; // Skip both bytes
            } else {
                str++;
            }
        } else {
            // Standard ASCII
            LCD_SendData(c);
            str++;
        }
    }
}
