/*
 * globalfunctions.c
 *
 *  Created on: Apr 19, 2024
 *      Author: Bence
 */
#include "UI_functions.h"
#include "usbd_cdc_acm_if.h"
#include  <ctype.h>

extern uint8_t message;
extern uint8_t usb_RX_Buff[];
/*    USER FUNCTION PROTOTYPES BEGIN    */
void Transmit(uint8_t *msg);
void List_all_commands();
HAL_StatusTypeDef Start_Device();
static HAL_StatusTypeDef Convert_to_PIN(uint16_t *pin);
static HAL_StatusTypeDef PIN_Check(uint16_t pin);
static HAL_StatusTypeDef PIN_Configure();
/*    USER FUNCTION PROTOTYPES END    */

/* Transmits the message to the user
   through VCP*/
void Transmit(uint8_t *msg){
	char c;
	uint8_t num = 0;
	for(; num < 64; num++){
		c = msg[num];
		if(c == '\0')
			break;
	}
	if(c != '\0') // message is too long to send
		return;
	CDC_Transmit(0, msg, num);
}

/* Lists all the commands through VCP
 for the user */
void List_all_commands(){
	Transmit((uint8_t *)"\n\rListing available commands:\n\r");
	HAL_Delay(10);
	Transmit((uint8_t *)"r : Resets the device. Deletes everything!\n\r");
	HAL_Delay(10);
	Transmit((uint8_t *)"l : Lists the control commands.\n\r");
}

/* Starting configuration such as
   validating the PIN */
HAL_StatusTypeDef Start_Device(){
	uint16_t pin = 0;
	HAL_StatusTypeDef ret;

	Transmit((uint8_t *)"\n\rBeginning the start configuration.");
	HAL_Delay(10);

	if(Flash_PIN_Read(&pin) == HAL_OK){ // if there is a valid PIN in the Flash
		ret = PIN_Check(pin); // we compare it to user input
	}
	else{                      // if there isn't
		ret = PIN_Configure(); // the user configures the PIN
	}

	return ret;
}

/* Converts the user input from the TX array
   into a uint16_t */
static HAL_StatusTypeDef Convert_to_PIN(uint16_t *pin){

	for(uint8_t i = 0; i < 4; i++){
		uint8_t c = usb_RX_Buff[i];
		if ('0' <= c && c <= '9')    // the PIN code must consist of digits
			*pin += (uint16_t) (c - '0') * (uint16_t) pow(10, 3 - i); // PIN is the sum of the digits multiplied by their place-values
		else
			return HAL_ERROR;
	}

	return HAL_OK;
}

/* Checks if the PIN given by the user matches
   the PIN saved in Flash and unlocks the device*/
static HAL_StatusTypeDef PIN_Check(uint16_t pin){
	uint16_t userpin = 0;

	Transmit((uint8_t *)"\n\rPlease enter the PIN to unlock the device.\n\r");
	while (!message) {} // wait for user response
	message = 0; // message received

    if(Convert_to_PIN(&userpin) != HAL_OK){
		Transmit((uint8_t*) "\n\rError: The format of the PIN is invalid!");
		HAL_Delay(10);
		Transmit((uint8_t*) "\n\rRestarting the device...\n\r");
		return HAL_ERROR;
    }

	if (pin == userpin) {
		Transmit((uint8_t *)"\n\rDevice unlocked successfully!");
		return HAL_OK;
	} else {
		Transmit((uint8_t *)"\n\rError: The PINs don't match!");
		HAL_Delay(10);
		Transmit((uint8_t *)"\n\rRestarting the device...\n\r");
		return HAL_ERROR;
	}
}

/* Saves the PIN given by the user
   to the Flash and unlocks the device*/
static HAL_StatusTypeDef PIN_Configure(){
	uint16_t userpin = 0;

	Transmit((uint8_t *)"\n\rEnter a PIN code, which must consists of 4 number digits.\n\r");
	while (!message) {} // wait for user response
	message = 0; // message received

	 if(Convert_to_PIN(&userpin) != HAL_OK){
		 Transmit((uint8_t *)"\n\rError: The format of the PIN is invalid!");
		 HAL_Delay(10);
		 Transmit((uint8_t *)"\n\rRestarting the device...\n\r");
		 return HAL_ERROR;
	 }

	if (Flash_PIN_Write(userpin) == HAL_OK) {
		Transmit((uint8_t *)"\n\rPIN saved and device unlocked successfully!\n\r");
		return HAL_OK;
	} else {
		Transmit((uint8_t *)"\n\rError: The PIN couldn't be saved!");
		HAL_Delay(10);
		Transmit((uint8_t *)"\n\rRestarting the device...\n\r");
		return HAL_ERROR;
	}
}
