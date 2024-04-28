/*
 * my_keyboard.c
 *
 *  Created on: Apr 27, 2024
 *      Author: Bence
 */
#include "my_keyboard.h"
#include "my_usbd_hid_keyboard.h"
#include "usb_device.h"
#include "UI_functions.h"

USBD_StatusTypeDef Send_Keystrokes(const uint8_t *buff);
USBD_StatusTypeDef Convert_char_to_Keystroke(uint8_t index);

extern USBD_HandleTypeDef hUsbDevice;

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

Keyboard keycodes = { 0, 0, 0, 0, 0, 0, 0, 0 };

const char ascii_to_hid_key_map[95][2] = {                                         // index with ascii
    {0, KEY_SPACE}, {KEY_MOD_LSHIFT, KEY_1}, {KEY_MOD_LSHIFT, KEY_APOSTROPHE},
    {KEY_MOD_LSHIFT, KEY_3}, {KEY_MOD_LSHIFT, KEY_4}, {KEY_MOD_LSHIFT, KEY_5},
    {KEY_MOD_LSHIFT, KEY_7}, {0, KEY_APOSTROPHE}, {KEY_MOD_LSHIFT, KEY_9},
    {KEY_MOD_LSHIFT, KEY_0}, {KEY_MOD_LSHIFT, KEY_8}, {KEY_MOD_LSHIFT, KEY_EQUAL},
    {0, KEY_COMMA}, {0, KEY_MINUS}, {0, KEY_DOT}, {0, KEY_SLASH}, {0, KEY_0},
    {0, KEY_1}, {0, KEY_2}, {0, KEY_3}, {0, KEY_4}, {0, KEY_5}, {0, KEY_6},
    {0, KEY_7}, {0, KEY_8}, {0, KEY_9}, {KEY_MOD_LSHIFT, KEY_SEMICOLON},
    {0, KEY_SEMICOLON}, {KEY_MOD_LSHIFT, KEY_COMMA}, {0, KEY_EQUAL},
    {KEY_MOD_LSHIFT, KEY_DOT}, {KEY_MOD_LSHIFT, KEY_SLASH}, {KEY_MOD_LSHIFT, KEY_2},
    {KEY_MOD_LSHIFT, KEY_A}, {KEY_MOD_LSHIFT, KEY_B}, {KEY_MOD_LSHIFT, KEY_C},
    {KEY_MOD_LSHIFT, KEY_D}, {KEY_MOD_LSHIFT, KEY_E}, {KEY_MOD_LSHIFT, KEY_F},
    {KEY_MOD_LSHIFT, KEY_G}, {KEY_MOD_LSHIFT, KEY_H}, {KEY_MOD_LSHIFT, KEY_I},
    {KEY_MOD_LSHIFT, KEY_J}, {KEY_MOD_LSHIFT, KEY_K}, {KEY_MOD_LSHIFT, KEY_L},
    {KEY_MOD_LSHIFT, KEY_M}, {KEY_MOD_LSHIFT, KEY_N}, {KEY_MOD_LSHIFT, KEY_O},
    {KEY_MOD_LSHIFT, KEY_P}, {KEY_MOD_LSHIFT, KEY_Q}, {KEY_MOD_LSHIFT, KEY_R},
    {KEY_MOD_LSHIFT, KEY_S}, {KEY_MOD_LSHIFT, KEY_T}, {KEY_MOD_LSHIFT, KEY_U},
    {KEY_MOD_LSHIFT, KEY_V}, {KEY_MOD_LSHIFT, KEY_W}, {KEY_MOD_LSHIFT, KEY_X},
    {KEY_MOD_LSHIFT, KEY_Y}, {KEY_MOD_LSHIFT, KEY_Z}, {0, KEY_LEFTBRACE},
    {0, KEY_BACKSLASH}, {0, KEY_RIGHTBRACE}, {KEY_MOD_LSHIFT, KEY_6},
    {KEY_MOD_LSHIFT, KEY_MINUS}, {0, KEY_GRAVE}, {0, KEY_A}, {0, KEY_B},
    {0, KEY_C}, {0, KEY_D}, {0, KEY_E}, {0, KEY_F}, {0, KEY_G}, {0, KEY_H},
    {0, KEY_I}, {0, KEY_J}, {0, KEY_K}, {0, KEY_L}, {0, KEY_M}, {0, KEY_N},
    {0, KEY_O}, {0, KEY_P}, {0, KEY_Q}, {0, KEY_R}, {0, KEY_S}, {0, KEY_T},
    {0, KEY_U}, {0, KEY_V}, {0, KEY_W}, {0, KEY_X}, {0, KEY_Y}, {0, KEY_Z},
    {KEY_MOD_LSHIFT, KEY_LEFTBRACE}, {KEY_MOD_LSHIFT, KEY_BACKSLASH},
    {KEY_MOD_LSHIFT, KEY_RIGHTBRACE}, {KEY_MOD_LSHIFT, KEY_GRAVE}
};

/* Converts and sends the file
   through USB*/
USBD_StatusTypeDef Send_Keystrokes(const uint8_t *buff) {
	for (uint8_t *c = buff; *c != '\0'; c++) {
		if(Convert_char_to_Keystroke(*c) != USBD_OK){// press the key
			return USBD_FAIL;
		}

		if(USBD_HID_Keyboard_SendReport(&hUsbDevice,&keycodes,sizeof(keycodes)) != USBD_OK){
			return USBD_FAIL;
		}

		HAL_Delay(5);
		keycodes.KEYCODE1 = 0; // release the key
		keycodes.MODIFIER = 0;
		if(USBD_HID_Keyboard_SendReport(&hUsbDevice,&keycodes,sizeof(keycodes)) != USBD_OK){
			return USBD_FAIL;
		}
		HAL_Delay(50);
	}
	return USBD_OK;
}

/* Converts the given data into
   a keyboard character*/
USBD_StatusTypeDef Convert_char_to_Keystroke(uint8_t index){
	index -= 32;
	if(index < 32 || 127 < index){ // only map the ascii chars from 32 to 127
		return USBD_FAIL;
	}

	keycodes.MODIFIER = ascii_to_hid_key_map[index][0];
	keycodes.KEYCODE1 = ascii_to_hid_key_map[index][1];
	return USBD_OK;
}
