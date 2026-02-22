/*
 * ui.h
 *
 *  Created on: 21 Feb 2026
 *      Author: user
 */

#ifndef INC_UI_H_
#define INC_UI_H_

#include <main.h>

extern volatile uint8_t oc_detected;
extern volatile uint8_t sc_detected;
extern signed short target_temp;


void Process_UI(void);
void Save_Int16_To_EEPROM(int16_t data);
uint8_t Read_Int16_From_EEPROM(int16_t *data);

#endif /* INC_UI_H_ */

