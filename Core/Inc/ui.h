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


#endif /* INC_UI_H_ */

