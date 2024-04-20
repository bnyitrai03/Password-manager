/*
 * global_functions.h
 *
 *  Created on: Apr 19, 2024
 *      Author: Bence
 */
#include "my_flash.h"

#ifndef SRC_GLOBAL_FUNCTIONS_H_
#define SRC_GLOBAL_FUNCTIONS_H_

/*    USER FUNCTION PROTOTYPES BEGIN    */
void Transmit(uint8_t *msg);
void List_all_commands();
HAL_StatusTypeDef Start_Device();
/*    USER FUNCTION PROTOTYPES END    */

#endif /* SRC_GLOBAL_FUNCTIONS_H_ */
