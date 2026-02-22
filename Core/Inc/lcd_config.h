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
#define LCD_D0_PORT  DB0_GPIO_Port
#define LCD_D0_PIN   DB0_Pin
#define LCD_D1_PORT  DB1_GPIO_Port
#define LCD_D1_PIN   DB1_Pin
#define LCD_D2_PORT  DB2_GPIO_Port
#define LCD_D2_PIN   DB2_Pin
#define LCD_D3_PORT  DB3_GPIO_Port
#define LCD_D3_PIN   DB3_Pin
#define LCD_D4_PORT  DB4_GPIO_Port
#define LCD_D4_PIN   DB4_Pin
#define LCD_D5_PORT  DB5_GPIO_Port
#define LCD_D5_PIN   DB5_Pin
#define LCD_D6_PORT  DB6_GPIO_Port
#define LCD_D6_PIN   DB6_Pin
#define LCD_D7_PORТ  DB7_GPIO_Port
#define LCD_D7_PIN   DB7_Pin

#define LCD_RS_PORT      MDO_RS_GPIO_Port
#define LCD_RS_PIN       MDO_RS_Pin

#define LCD_RW_PORT      MDO_RW_GPIO_Port
#define LCD_RW_PIN       MDO_RW_Pin

#define LCD_EN_PORT      MDO_RW_GPIO_Port  //Cut EN and RW tracks. Solder EN connector output to the TVS's RW pin. Tie RW output to GND.
#define LCD_EN_PIN       MDO_RW_Pin


#endif /* INC_LCD_CONFIG_H_ */
