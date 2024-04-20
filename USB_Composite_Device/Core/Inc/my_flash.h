/*
 * my_flash.h
 *
 *  Created on: Apr 20, 2024
 *      Author: Bence
 */

#ifndef SRC_MY_FLASH_H_
#define SRC_MY_FLASH_H_

#include <stdint.h>
#include <string.h>
#include "stm32f4xx_hal.h"

/*    USER DEFINES BEGIN    */
#define SECTOR_11 ( 0x080E0000 )    //Flash Address (Sector 11) 128kByte
#define CHECKSUM_SECTOR_11 ( 0x080F8000 ) // Address of the Sector 11 checksum
#define SEQUENCE_NUM_SECTOR_11 ( 0x080A0000 ) // Address of the Sector 11  sequence number
#define SEQUENCE_NUM_SECTOR_11_STORAGE FLASH_SECTOR_9 // seqnum11 is stored in this sector

#define SECTOR_10 ( 0x080C0000 )    //Flash Address (Sector 10) 128kByte
#define CHECKSUM_SECTOR_10 ( 0x080D8000 ) // Address of the Sector 10 checksum
#define SEQUENCE_NUM_SECTOR_10 ( 0x08080000 ) // Address of the Sector 10 sequence number
#define SEQUENCE_NUM_SECTOR_10_STORAGE FLASH_SECTOR_8; // seqnum10 is stored in this sector

#define PIN ( 0x08060000 )
#define PIN_STORAGE FLASH_SECTOR_7 // PIN will be stored in Flash Sector 7
/*    USER DEFINES END    */

/*    USER FUNCTION PROTOTYPES BEGIN    */
void Flash_Read_Init();
HAL_StatusTypeDef Flash_Write();
HAL_StatusTypeDef Flash_Sector_Erase(uint8_t *sector, uint8_t num);
HAL_StatusTypeDef Flash_PIN_Read(uint16_t *pin);
HAL_StatusTypeDef Flash_PIN_Write(uint16_t pin);
/*    USER FUNCTION PROTOTYPES END    */

#endif /* SRC_MY_FLASH_H_ */
