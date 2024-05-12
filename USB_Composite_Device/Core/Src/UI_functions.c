/*
 * globalfunctions.c
 *
 *  Created on: Apr 19, 2024
 *      Author: Bence
 */
#include "UI_functions.h"
#include "usbd_cdc_acm_if.h"
#include "my_flash.h"
#include "fatfs.h"
#include  <math.h>


extern uint8_t message;
extern uint8_t usb_RX_Buff[];
extern volatile uint8_t button;

/*    USER FUNCTION PROTOTYPES BEGIN    */
void Transmit(uint8_t *msg);
void List_all_commands();
void Reset();
void Enter_password();
HAL_StatusTypeDef Start_Device();
static HAL_StatusTypeDef Convert_to_PIN(uint16_t *pin);
static HAL_StatusTypeDef PIN_Check(uint16_t pin);
static HAL_StatusTypeDef PIN_Configure();
static HAL_StatusTypeDef Format_Password(uint8_t *password);
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

	if(c != '\0'){ // message is too long to send
		CDC_Transmit(0, "Error: Message was too long!", sizeof("Error: Message was too long!"));
		return;
	}

	CDC_Transmit(0, msg, num);
	HAL_Delay(10); // wait for the transmit
}

/* Lists all the commands through VCP
 for the user */
void List_all_commands(){
	Transmit((uint8_t *)"\n\rListing available commands:\n\r");
	Transmit((uint8_t *)"l : Lists the control commands.\n\r");
	Transmit((uint8_t *)"e : Enters the desired website.\n\r");
	Transmit((uint8_t *)"r : Resets the device. Deletes everything!\n\r");
}

/* Wipes the flash memory of the device
   then resets it */
void Reset(){
	uint8_t sectors[] = {7, 8, 9, 10, 11};

	if (HAL_FLASH_Unlock() != HAL_OK)
		return;
	Transmit("\n\rStarting the erasure.\n\r");
	if (Flash_Sector_Erase(sectors, 5) == HAL_OK) {
		HAL_FLASH_Lock();
		Transmit("\n\rFlash memory has been erased.\n\r");
	} else {
		Transmit("\n\rFlash memory failed to erase.\n\r");
		return;
	}

	Transmit("\n\rSoft resetting the device...\n\r");
	NVIC_SystemReset();
}

/* Starting configuration such as
   validating the PIN */
HAL_StatusTypeDef Start_Device(){
	uint16_t pin = 0;
	HAL_StatusTypeDef ret;

	Transmit((uint8_t *)"\n\rBeginning the start configuration.");

	if(Flash_PIN_Read(&pin) == HAL_OK){ // if there is a valid PIN in the Flash
		ret = PIN_Check(pin); // we compare it to user input
	}
	else{                      // if there isn't
		ret = PIN_Configure(); // the user configures the PIN
	}

	return ret;
}

/* Logs in the user to the
  desired website */
void Enter_password(){
	uint8_t filebuff[512] = {};
	FRESULT ret;

	Transmit((uint8_t*) "\n\rSelect the website you want to enter:");
	if ((ret = Fat_Read_Filenames()) != FR_OK) { //lists all the websites
		if (ret != FR_INVALID_PARAMETER)
			Transmit("\n\rError: Couldn't read all the filenames!");
		return;
	}

	while (!message) {} // wait for user response
	message = 0;        // message received

	if(Fat_Read(usb_RX_Buff, filebuff) != FR_OK){  // reads the file data
		Transmit("\n\rError: Couldn't read the file!");
		return;
	}

	if (Format_Password(filebuff) != HAL_OK) {
		Transmit("\n\rError: Wrong file format!");
		return;
	}

	Transmit("\n\rPress the button to transmit the password!\n\r");
	while(button){};

	if(Send_Keystrokes(filebuff) == USBD_OK)
		Transmit("\n\rPassword successfully typed.\n\r");
	else
		Transmit("\n\rError: Couldn't type the password!\n\r");

	button = 1; // for debouncing purposes enable the flag here
}

/* Removes the filename from
 the password */
static HAL_StatusTypeDef Format_Password(uint8_t *password) {
	HAL_StatusTypeDef ret = HAL_ERROR;

	uint8_t *tilde_pos = strchr(password, '~'); // After the first tilde is the password
	if (tilde_pos != NULL) {
		memmove(password, tilde_pos + 1, strlen(tilde_pos) + 1); // +1 to include the null terminator
		ret = HAL_OK;
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
		Transmit((uint8_t*) "\n\Start the device again!\n\r");
		return HAL_ERROR;
    }

	if (pin == userpin) {
		Transmit((uint8_t *)"\n\rDevice unlocked successfully!");
		return HAL_OK;
	} else {
		Transmit((uint8_t *)"\n\rError: The PINs don't match!");
		Transmit((uint8_t *)"\n\rStart the device again!\n\r");
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
		 Transmit((uint8_t *)"\n\rStart the device again!\n\r");
		 return HAL_ERROR;
	 }

	if (Flash_PIN_Write(userpin) == HAL_OK) {
		Transmit((uint8_t *)"\n\rPIN saved and device unlocked successfully!\n\r");
		return HAL_OK;
	} else {
		Transmit((uint8_t *)"\n\rError: The PIN couldn't be saved!");
		Transmit((uint8_t *)"\n\rStart the device again!\n\r");
		return HAL_ERROR;
	}
}
