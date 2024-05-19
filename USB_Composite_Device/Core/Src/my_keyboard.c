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

/* USER FUNCTION PROTOTYPES BEGIN */
USBD_StatusTypeDef Send_Keystrokes(const uint8_t *buff);
uint8_t Set_Keyboard_Language(uint8_t langId);
static USBD_StatusTypeDef Convert_char_to_Keystroke(uint8_t ascii);
/* USER FUNCTION PROTOTYPES END */

/* USER PRIVATE TYPES BEGIN */
extern USBD_HandleTypeDef hUsbDevice;

static Keyboard keycodes = { 0, 0, 0, 0, 0, 0, 0, 0 };
static Keys hunKeys[NumberOfHunKeys] = {
	{'0',53, MOD_NO_MODIFIER},
	{'1',30, MOD_NO_MODIFIER},
	{'2',31, MOD_NO_MODIFIER},
	{'3',32, MOD_NO_MODIFIER},
	{'4',33, MOD_NO_MODIFIER},
	{'5',34, MOD_NO_MODIFIER},
	{'6',35, MOD_NO_MODIFIER},
	{'7',36, MOD_NO_MODIFIER},
	{'8',37, MOD_NO_MODIFIER},
	{'9',38, MOD_NO_MODIFIER},
	{182,39, MOD_NO_MODIFIER}, //ö
	{188,45, MOD_NO_MODIFIER}, //ü
	{179,46, MOD_NO_MODIFIER}, //ó
	{0xa7,53, MOD_SHIFT_LEFT}, //§
	{'\'',30, MOD_SHIFT_LEFT},
	{'"',31, MOD_SHIFT_LEFT},
	{'+',32, MOD_SHIFT_LEFT},
	{'!',33, MOD_SHIFT_LEFT},
	{'%',34, MOD_SHIFT_LEFT},
	{'/',35, MOD_SHIFT_LEFT},
	{'=',36, MOD_SHIFT_LEFT},
	{'(',37, MOD_SHIFT_LEFT},
	{')',38, MOD_SHIFT_LEFT},
	{150,39, MOD_SHIFT_LEFT}, //Ö
	{156,45, MOD_SHIFT_LEFT}, //Ü
	{147,46, MOD_SHIFT_LEFT}, //Ó
	{'~',30, MOD_ALT_RIGHT},
	{'^',32, MOD_ALT_RIGHT},
	{'`',36, MOD_ALT_RIGHT},
	{'\t',43, MOD_NO_MODIFIER},
	{'\n',40, MOD_NO_MODIFIER},
	{' ',44, MOD_NO_MODIFIER},
	{'a',4, MOD_NO_MODIFIER},
	{'s',22, MOD_NO_MODIFIER},
	{'d',7, MOD_NO_MODIFIER},
	{'f',9, MOD_NO_MODIFIER},
	{'g',10, MOD_NO_MODIFIER},
	{'h',11, MOD_NO_MODIFIER},
	{'j',13, MOD_NO_MODIFIER},
	{'k',14, MOD_NO_MODIFIER},
	{'l',15, MOD_NO_MODIFIER},
	{169,51, MOD_NO_MODIFIER}, //é
	{161,52, MOD_NO_MODIFIER}, //á
	{'A',4, MOD_SHIFT_LEFT},
	{'S',22, MOD_SHIFT_LEFT},
	{'D',7, MOD_SHIFT_LEFT},
	{'F',9, MOD_SHIFT_LEFT},
	{'G',10, MOD_SHIFT_LEFT},
	{'H',11, MOD_SHIFT_LEFT},
	{'J',13, MOD_SHIFT_LEFT},
	{'K',14, MOD_SHIFT_LEFT},
	{'L',15, MOD_SHIFT_LEFT},
	{137,51, MOD_SHIFT_LEFT}, //É
	{129,52, MOD_SHIFT_LEFT}, //Á
	{'[',9, MOD_ALT_RIGHT},
	{']',10, MOD_ALT_RIGHT},
	{'$',51, MOD_ALT_RIGHT},
	{159,52, MOD_ALT_RIGHT}, //ß
	{'q',20, MOD_NO_MODIFIER},
	{'w',26, MOD_NO_MODIFIER},
	{'e',8, MOD_NO_MODIFIER},
	{'r',21, MOD_NO_MODIFIER},
	{'t',23, MOD_NO_MODIFIER},
	{'z',28, MOD_NO_MODIFIER},
	{'u',24, MOD_NO_MODIFIER},
	{'i',12, MOD_NO_MODIFIER},
	{'o',18, MOD_NO_MODIFIER},
	{'p',19, MOD_NO_MODIFIER},
	{145,47, MOD_NO_MODIFIER}, //ő
	{186,48, MOD_NO_MODIFIER}, //ú
	{177,49, MOD_NO_MODIFIER}, //ű
	{'Q',20, MOD_SHIFT_LEFT},
	{'W',26, MOD_SHIFT_LEFT},
	{'E',8, MOD_SHIFT_LEFT},
	{'R',21, MOD_SHIFT_LEFT},
	{'T',23, MOD_SHIFT_LEFT},
	{'Z',28, MOD_SHIFT_LEFT},
	{'U',24, MOD_SHIFT_LEFT},
	{'I',12, MOD_SHIFT_LEFT},
	{'O',18, MOD_SHIFT_LEFT},
	{'P',19, MOD_SHIFT_LEFT},
	{144,47, MOD_SHIFT_LEFT}, //Ő
	{154,48, MOD_SHIFT_LEFT}, //Ú
	{176,49, MOD_SHIFT_LEFT}, //Ű
	{'\\',20, MOD_ALT_RIGHT},
	{'|',26, MOD_ALT_RIGHT},
	{132,8, MOD_ALT_RIGHT}, //Ä
	{172,24, MOD_ALT_RIGHT}, //€
	{183,47, MOD_ALT_RIGHT}, //÷
	{151,48, MOD_ALT_RIGHT}, //×
	{'y',29, MOD_NO_MODIFIER},
	{'x',27, MOD_NO_MODIFIER},
	{'c',6, MOD_NO_MODIFIER},
	{'v',25, MOD_NO_MODIFIER},
	{'b',5, MOD_NO_MODIFIER},
	{'n',17, MOD_NO_MODIFIER},
	{'m',16, MOD_NO_MODIFIER},
	{',',54, MOD_NO_MODIFIER},
	{'.',55, MOD_NO_MODIFIER},
	{'-',56, MOD_NO_MODIFIER},
	{'Y',29, MOD_SHIFT_LEFT},
	{'X',27, MOD_SHIFT_LEFT},
	{'C',6, MOD_SHIFT_LEFT},
	{'V',25, MOD_SHIFT_LEFT},
	{'B',5, MOD_SHIFT_LEFT},
	{'N',17, MOD_SHIFT_LEFT},
	{'M',16, MOD_SHIFT_LEFT},
	{'?',54, MOD_SHIFT_LEFT},
	{':',55, MOD_SHIFT_LEFT},
	{'_',56, MOD_SHIFT_LEFT},
	{'>',29, MOD_ALT_RIGHT},
	{'#',27, MOD_ALT_RIGHT},
	{'&',6, MOD_ALT_RIGHT},
	{'@',25, MOD_ALT_RIGHT},
	{'{',5, MOD_ALT_RIGHT},
	{'}',17, MOD_ALT_RIGHT},
	{'<',16, MOD_ALT_RIGHT},
	{';',54, MOD_ALT_RIGHT},
	{'>',55, MOD_ALT_RIGHT},
	{'*',56, MOD_ALT_RIGHT},
	{173,100, MOD_NO_MODIFIER}, //í
	{141,100, MOD_SHIFT_LEFT}, //Í
	{'<',100, MOD_ALT_RIGHT},
	{'\0',0, MOD_NO_MODIFIER}, //END OF STRUCT
};

//stores the english keyboard characters and keyboard codes
static Keys engKeys[NumberOfEngKeys] = {
	{'0',39, MOD_NO_MODIFIER},
	{'1',30, MOD_NO_MODIFIER},
	{'2',31, MOD_NO_MODIFIER},
	{'3',32, MOD_NO_MODIFIER},
	{'4',33, MOD_NO_MODIFIER},
	{'5',34, MOD_NO_MODIFIER},
	{'6',35, MOD_NO_MODIFIER},
	{'7',36, MOD_NO_MODIFIER},
	{'8',37, MOD_NO_MODIFIER},
	{'9',38, MOD_NO_MODIFIER},
	{182,19, MOD_ALT_RIGHT}, //ö
	{188,28, MOD_ALT_RIGHT}, //ü
	{179,18, MOD_ALT_RIGHT}, //ó
	{'\'',49, MOD_NO_MODIFIER},
	{'"',52, MOD_NO_MODIFIER},
	{'+',46, MOD_SHIFT_LEFT},
	{'!',30, MOD_SHIFT_LEFT},
	{'%',34, MOD_SHIFT_LEFT},
	{'/',56, MOD_NO_MODIFIER},
	{'=',46, MOD_NO_MODIFIER},
	{'(',37, MOD_SHIFT_LEFT},
	{')',38, MOD_SHIFT_LEFT},
	{'~',53, MOD_SHIFT_LEFT},
	{'^',35, MOD_SHIFT_LEFT},
	{'`',53, MOD_NO_MODIFIER},
	{'\t',43, MOD_NO_MODIFIER},
	{'\n', 40, MOD_NO_MODIFIER},
	{' ',44, MOD_NO_MODIFIER},
	{'a',4, MOD_NO_MODIFIER},
	{'s',22, MOD_NO_MODIFIER},
	{'d',7, MOD_NO_MODIFIER},
	{'f',9, MOD_NO_MODIFIER},
	{'g',10, MOD_NO_MODIFIER},
	{'h',11, MOD_NO_MODIFIER},
	{'j',13, MOD_NO_MODIFIER},
	{'k',14, MOD_NO_MODIFIER},
	{'l',15, MOD_NO_MODIFIER},
	{169,8, MOD_ALT_RIGHT}, //é
	{0xa1,4, MOD_ALT_RIGHT}, //á
	{'A',4, MOD_SHIFT_LEFT},
	{'S',22, MOD_SHIFT_LEFT},
	{'D',7, MOD_SHIFT_LEFT},
	{'F',9, MOD_SHIFT_LEFT},
	{'G',10, MOD_SHIFT_LEFT},
	{'H',11, MOD_SHIFT_LEFT},
	{'J',13, MOD_SHIFT_LEFT},
	{'K',14, MOD_SHIFT_LEFT},
	{'L',15, MOD_SHIFT_LEFT},
	{'[',47, MOD_NO_MODIFIER},
	{']',48, MOD_NO_MODIFIER},
	{'$',33, MOD_SHIFT_LEFT},
	{'q',20, MOD_NO_MODIFIER},
	{'w',26, MOD_NO_MODIFIER},
	{'e',8, MOD_NO_MODIFIER},
	{'r',21, MOD_NO_MODIFIER},
	{'t',23, MOD_NO_MODIFIER},
	{'z',29, MOD_NO_MODIFIER},
	{'u',24, MOD_NO_MODIFIER},
	{'i',12, MOD_NO_MODIFIER},
	{'o',18, MOD_NO_MODIFIER},
	{'p',19, MOD_NO_MODIFIER},
	{186,24, MOD_ALT_RIGHT}, //ú
	{'Q',20, MOD_SHIFT_LEFT},
	{'W',26, MOD_SHIFT_LEFT},
	{'E',8, MOD_SHIFT_LEFT},
	{'R',21, MOD_SHIFT_LEFT},
	{'T',23, MOD_SHIFT_LEFT},
	{'Z',29, MOD_SHIFT_LEFT},
	{'U',24, MOD_SHIFT_LEFT},
	{'I',12, MOD_SHIFT_LEFT},
	{'O',18, MOD_SHIFT_LEFT},
	{'P',19, MOD_SHIFT_LEFT},
	{'\\',49, MOD_NO_MODIFIER},
	{'|',49, MOD_SHIFT_LEFT},
	{'y',28, MOD_NO_MODIFIER},
	{'x',27, MOD_NO_MODIFIER},
	{'c',6, MOD_NO_MODIFIER},
	{'v',25, MOD_NO_MODIFIER},
	{'b',5, MOD_NO_MODIFIER},
	{'n',17, MOD_NO_MODIFIER},
	{'m',16, MOD_NO_MODIFIER},
	{',',54, MOD_NO_MODIFIER},
	{'.',55, MOD_NO_MODIFIER},
	{'-',45, MOD_NO_MODIFIER},
	{'Y',28, MOD_SHIFT_LEFT},
	{'X',27, MOD_SHIFT_LEFT},
	{'C',6, MOD_SHIFT_LEFT},
	{'V',25, MOD_SHIFT_LEFT},
	{'B',5, MOD_SHIFT_LEFT},
	{'N',17, MOD_SHIFT_LEFT},
	{'M',16, MOD_SHIFT_LEFT},
	{'?',56, MOD_SHIFT_LEFT},
	{':',51, MOD_SHIFT_LEFT},
	{'_',45, MOD_SHIFT_LEFT},
	{'>',55, MOD_SHIFT_LEFT},
	{'#',32, MOD_SHIFT_LEFT},
	{'&',36, MOD_SHIFT_LEFT},
	{'@',31, MOD_SHIFT_LEFT},
	{'{',47, MOD_SHIFT_LEFT},
	{'}',48, MOD_SHIFT_LEFT},
	{'<',54, MOD_SHIFT_LEFT},
	{';',51, MOD_NO_MODIFIER},
	{'*',37, MOD_SHIFT_LEFT},
	{'\0',0, MOD_NO_MODIFIER}, //END OF STRUCT
};

/* stores the selected keyboard,
   hungarian by default */
static Keys *selectedKeyboard = hunKeys;
static uint8_t Length = NumberOfHunKeys;
static uint8_t langID = HUN;
/* USER PRIVATE TYPES END */

/* USER CODE BEGIN */
/* Converts and sends the file
   through USB*/
USBD_StatusTypeDef Send_Keystrokes(const uint8_t *buff) {
	for (uint16_t i = 0; buff[i] != '~'; i++) {
		if(buff[i] == '\r'){
			continue; // ENTER key consists stored as two chars: '\n' and '\r', we only send ENTER once
		}

		if(buff[i] == 195 || buff[i] == 197){
			continue; // these characters signal that the next character is special
		}

		if(Convert_char_to_Keystroke(buff[i]) != USBD_OK){
			return USBD_FAIL;
		}

		if(USBD_HID_Keyboard_SendReport(&hUsbDevice,&keycodes,sizeof(keycodes)) != USBD_OK){ // press the key
			return USBD_FAIL;
		}
		keycodes.KEYCODE1 = 0;
		keycodes.MODIFIER = 0;
		if(USBD_HID_Keyboard_SendReport(&hUsbDevice,&keycodes,sizeof(keycodes)) != USBD_OK){ // release the key
			return USBD_FAIL;
		}
		HAL_Delay(10);

		if(buff[i] == '\n'){
			HAL_Delay(200); // after hitting ENTER leave time for the website to load
		}
	}

	keycodes.KEYCODE1 = 0;
	keycodes.MODIFIER = 0;
	if (USBD_HID_Keyboard_SendReport(&hUsbDevice, &keycodes, sizeof(keycodes)) != USBD_OK){
		return USBD_FAIL;
	}

	return USBD_OK;
}

/* Converts the given data into
 a keyboard character*/
static USBD_StatusTypeDef Convert_char_to_Keystroke(uint8_t ascii) {
	for (uint8_t i = 0; i < Length; i++) {
		if (selectedKeyboard[i].ascii_code == ascii) {
			keycodes.MODIFIER = selectedKeyboard[i].mode;
			keycodes.KEYCODE1 = selectedKeyboard[i].keycode;
			return USBD_OK;
		}
	}

	// There are no matching characters
	keycodes.MODIFIER = 0;
	keycodes.KEYCODE1 = 0;
	return USBD_FAIL;
}

/* Sets the keyboard language
   to the given one*/
uint8_t Set_Keyboard_Language(uint8_t langId) {
	uint8_t ret = 1;

	switch (langId) {
	case HUN:
		selectedKeyboard = hunKeys;
		Length = NumberOfHunKeys;
		langID = HUN;
		break;

	case ENG:
		selectedKeyboard = engKeys;
		Length = NumberOfEngKeys;
		langID = ENG;
		break;

	default:
		ret = 0;
		break;
	}

	return ret;
}
/* USER CODE END */
