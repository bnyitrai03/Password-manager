/*
 * global_functions.h
 *
 *  Created on: Apr 19, 2024
 *      Author: Bence
 */
#include "my_flash.h"

#ifndef SRC_GLOBAL_FUNCTIONS_H_
#define SRC_GLOBAL_FUNCTIONS_H_

typedef enum{
	OK = 1,
	FAILURE = 0
}Status;

/*    USER FUNCTION PROTOTYPES BEGIN    */
void Transmit(char *msg);
void List_all_commands();
void Reset();
void Enter_password();
void Change_keyboard_language();
Status Start_Device();
/*    USER FUNCTION PROTOTYPES END    */

#endif /* SRC_GLOBAL_FUNCTIONS_H_ */
