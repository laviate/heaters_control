/*
 * lcd_config.h
 *
 *  Created on: 15.02.2026 г.
 *      Author: Valio
 */

#ifndef INC_LCD_CONFIG_H_
#define INC_LCD_CONFIG_H_

#include "main.h" // Brings in your GPIO defines

// --- Hardware Abstraction ---
// Using the defines you provided:
#define LCD_DATA_PORT_0  DB0_GPIO_Port
#define LCD_DATA_PIN_0   DB0_Pin
#define LCD_DATA_PORT_1  DB1_GPIO_Port
#define LCD_DATA_PIN_1   DB1_Pin
#define LCD_DATA_PORT_2  DB2_GPIO_Port
#define LCD_DATA_PIN_2   DB2_Pin
#define LCD_DATA_PORT_3  DB3_GPIO_Port
#define LCD_DATA_PIN_3   DB3_Pin
#define LCD_DATA_PORT_4  DB4_GPIO_Port
#define LCD_DATA_PIN_4   DB4_Pin
#define LCD_DATA_PORT_5  DB5_GPIO_Port
#define LCD_DATA_PIN_5   DB5_Pin
#define LCD_DATA_PORT_6  DB6_GPIO_Port
#define LCD_DATA_PIN_6   DB6_Pin
#define LCD_DATA_PORT_7  DB7_GPIO_Port
#define LCD_DATA_PIN_7   DB7_Pin

#define LCD_RS_PORT      MDO_RS_GPIO_Port
#define LCD_RS_PIN       MDO_RS_Pin

#define LCD_RW_PORT      MDO_RW_GPIO_Port
#define LCD_RW_PIN       MDO_RW_Pin


#endif /* INC_LCD_CONFIG_H_ */
