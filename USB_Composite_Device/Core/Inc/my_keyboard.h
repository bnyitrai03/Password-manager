/*
 * my_keyboard.h
 *
 *  Created on: Apr 27, 2024
 *      Author: Bence
 */

#ifndef INC_MY_KEYBOARD_H_
#define INC_MY_KEYBOARD_H_

#include "main.h"

typedef enum{
	MOD_NO_MODIFIER = 0,
	MOD_SHIFT_LEFT = 0x02,
	MOD_ALT_RIGHT = 0x40
}MODIFIER;

typedef enum{
	ENG = 0,
	HUN = 1
}Language;

typedef struct{
	uint16_t ascii_code;
	uint16_t keycode;
	MODIFIER mode;
}Keys;

typedef struct {
	uint8_t MODIFIER;
	uint8_t RESERVED;
	uint8_t KEYCODE1;
	uint8_t KEYCODE2;
	uint8_t KEYCODE3;
	uint8_t KEYCODE4;
	uint8_t KEYCODE5;
	uint8_t KEYCODE6;
} Keyboard;

#define NumberOfHunKeys 124
#define NumberOfEngKeys 104

#endif /* INC_MY_KEYBOARD_H_ */
