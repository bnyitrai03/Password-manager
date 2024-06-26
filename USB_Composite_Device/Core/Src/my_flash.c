/*
 * my_flash.c
 *
 *  Created on: Apr 20, 2024
 *      Author: Bence
 */
#include "my_flash.h"
#include "fatfs.h"

/*    USER FUNCTION PROTOTYPES BEGIN    */
void Flash_Read_Init();
HAL_StatusTypeDef Flash_Write();
HAL_StatusTypeDef Flash_Sector_Erase(uint8_t *sector, uint8_t num);
HAL_StatusTypeDef Flash_PIN_Read(uint16_t *pin);
HAL_StatusTypeDef Flash_PIN_Write(uint16_t pin);

static uint8_t Validate_Record(uint8_t *buff, char *filename);
static HAL_StatusTypeDef Verify_Sector(uint8_t *seqnum10, uint8_t *seqnum11, uint32_t *ACTIVE_SECTOR, uint8_t *buff);
static HAL_StatusTypeDef Write_Seqnum(uint8_t seqnum, uint8_t value);
static HAL_StatusTypeDef Flash_Write_Sector10(uint8_t *sector, uint8_t *seqnum10);
static HAL_StatusTypeDef Flash_Write_Sector11(uint8_t *sector, uint8_t *seqnum11);
/*    USER FUNCTION PROTOTYPES END    */

/* Initialize Fatfs with the contents of
   the internal Flash memory */
void Flash_Read_Init() {
	uint8_t buff[RECORD];
	char filename[13];
	uint8_t seqnum10;
	uint8_t seqnum11;
	uint32_t ACTIVE_SECTOR;

	memcpy(&seqnum11, (const void*) SEQUENCE_NUM_SECTOR_11, 1);
	memcpy(&seqnum10, (const void*) SEQUENCE_NUM_SECTOR_10, 1);
	// deciding which sector is active
	if ((seqnum11 > seqnum10)
			|| ((seqnum11 < seqnum10) && (seqnum10 - seqnum11 == 255))) // or it's ...255->0
		ACTIVE_SECTOR = SECTOR_11; // sector 11 is currently active
	else
		ACTIVE_SECTOR = SECTOR_10; // sector 10 is currently active

	// verifying the validity of the active sector
	if (Verify_Sector(&seqnum10, &seqnum11, &ACTIVE_SECTOR, buff) != HAL_OK)
		return;

	for (uint32_t i = 0; i < 192; i++) { // copy the whole Flash to FatFs
		memcpy(buff, (const void*) (ACTIVE_SECTOR + (i * RECORD)), RECORD); // load records from Flash to buffer, one record is 512 byte
		if (Validate_Record(buff, filename) != 1) // everything from flash has been moved to FatFs
			break;
		if (Fat_Write_Init((strcat(filename, ".txt")), (const uint8_t*) buff)
				!= FR_OK)
			break;
	}

}

/* Returns the PIN
   from the internal Flash */
HAL_StatusTypeDef Flash_PIN_Read(uint16_t *pin){
	memcpy(pin, (const void*) PIN, 2); // PIN is 4 characters long -> 2 bytes
	if(*pin == 0xFFFF) // PIN is uninitialized thus invalid
		return HAL_ERROR;
	else
		return HAL_OK;
}

/* Saves the PIN
   to the internal Flash */
HAL_StatusTypeDef Flash_PIN_Write(uint16_t pin){
	HAL_StatusTypeDef ret;
	uint8_t erase_sector = PIN_STORAGE;

	if ((ret = HAL_FLASH_Unlock()) != HAL_OK)
		  return ret;
	if ((ret = Flash_Sector_Erase(&erase_sector, 1)) != HAL_OK)
		return ret;
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, PIN, pin); // save the PIN to Flash
	ret = HAL_FLASH_Lock();
	return ret;
}

/* Erases the sectors given in the
   sector array from the internal Flash */
HAL_StatusTypeDef Flash_Sector_Erase(uint8_t *sector, uint8_t num){
	HAL_StatusTypeDef ret;
	FLASH_EraseInitTypeDef EraseInitStruct;
	uint32_t SectorError;
	EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
	EraseInitStruct.NbSectors = 1;
	EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
	EraseInitStruct.Banks = FLASH_BANK_1;

	for(uint8_t i = 0; i < num; i++){
		EraseInitStruct.Sector = sector[i];
		if((ret = HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError)) != HAL_OK) // erase the given sectors one by one
			return ret;
	}
	return ret;
}

/* Saves the current files from Fatfs to the
   internal Flash memory in two seperate copies */
HAL_StatusTypeDef Flash_Write() {
	uint8_t sector[RECORD] = {0};
	uint8_t seqnum10;
	uint8_t seqnum11;
	uint32_t ACTIVE_SECTOR;
	HAL_StatusTypeDef ret = HAL_ERROR;

	// deciding which sector to write first
	memcpy(&seqnum11, (const void*) SEQUENCE_NUM_SECTOR_11, 1);
	memcpy(&seqnum10, (const void*) SEQUENCE_NUM_SECTOR_10, 1);
	// deciding which sector is active
	if ((seqnum11 > seqnum10)
			|| ((seqnum11 < seqnum10) && (seqnum10 - seqnum11 == 255))) // or it's ...255->0
		ACTIVE_SECTOR = SECTOR_11; // sector 11 is currently active
	else
		ACTIVE_SECTOR = SECTOR_10; // sector 10 is currently active


	if (ACTIVE_SECTOR == SECTOR_11)
		if ((ret = Flash_Write_Sector10(sector, &seqnum10)) == HAL_OK) // if seqnum10 is older we write this sector first
			ret = Flash_Write_Sector11(sector, &seqnum11);
	if (ACTIVE_SECTOR == SECTOR_10)
		if ((ret = Flash_Write_Sector11(sector, &seqnum11)) == HAL_OK) // else sector11 is older
			ret = Flash_Write_Sector10(sector, &seqnum10);

	return ret;
}

/* Checks wheter the last writing of the Flash
   happenned with consistent data */
static HAL_StatusTypeDef Verify_Sector(uint8_t *seqnum10, uint8_t *seqnum11, uint32_t *ACTIVE_SECTOR, uint8_t *buff) {
	uint8_t checksum;
	uint8_t verificationsum = 0xFF;
	char filename[13];

	if (*ACTIVE_SECTOR == SECTOR_11) {
		memcpy(&checksum, (const void*) CHECKSUM_SECTOR_11, 1); // reading the checksum
		verificationsum ^= *seqnum11; // starting the calculation of the verification sum
	} else if (*ACTIVE_SECTOR == SECTOR_10) {
		memcpy(&checksum, (const void*) CHECKSUM_SECTOR_10, 1);
		verificationsum ^= *seqnum10;
	}

	for (uint32_t i = 0; i < 192; i++) {
		memcpy(buff, (const void*) (*ACTIVE_SECTOR + (i * RECORD)), RECORD); // load records from Flash to buffer, one record is 512 byte
		if (Validate_Record(buff, filename) != 1) // everything from flash has been moved to FatFs
			break;
		for (uint32_t j = 0; j < RECORD; j++) {
			verificationsum ^= buff[j];
		}
	}

	if (checksum != verificationsum) {     // if the checksum doesn't match
		if (*ACTIVE_SECTOR == SECTOR_11) {
			*ACTIVE_SECTOR = SECTOR_10;    // the other sector is active
			if (HAL_FLASH_Unlock() != HAL_OK) {
				return HAL_ERROR;
			}
			if (Write_Seqnum(11, (*seqnum10) - 1) != HAL_OK) // correcting the sequence numbers to be adjacent
				return HAL_ERROR;

		} else if (*ACTIVE_SECTOR == SECTOR_10) {
			*ACTIVE_SECTOR = SECTOR_11;
			if (HAL_FLASH_Unlock() != HAL_OK) {
				return HAL_ERROR;
			}
			if (Write_Seqnum(10, (*seqnum11) - 1) != HAL_OK) // correcting the sequence numbers to be adjacent
				return HAL_ERROR;
		}
	}
	return HAL_OK;
}

/* Validates the record based on
   the name of the file */
static uint8_t Validate_Record(uint8_t *buff, char *filename) {
	uint8_t name_char = 0;     // move the filename from buff
	uint8_t valid = 1;
	for (uint8_t i = 0; i < 13; i++) { // clear the filename
		filename[i] = '\0';
	}
	for (uint8_t *c = buff; *c != '.'; c++) { // record starts with filename, after the name comes a '.' character
		filename[name_char] = buff[name_char];
		name_char++;
		if (name_char == 13) { // if we didn't find the name at the start of the record, there are no more records
			valid = 0;
			return valid;
		}
	}
	return valid;
}

/* Saves the sequence number
   to the internal Flash memory */
static HAL_StatusTypeDef Write_Seqnum(uint8_t seqnum, uint8_t value){
	HAL_StatusTypeDef ret;
	uint8_t *seqnum_sector[2];
	seqnum_sector[0] = SEQUENCE_NUM_SECTOR_11_STORAGE;
	seqnum_sector[1] = SEQUENCE_NUM_SECTOR_10_STORAGE;

	if (seqnum == 11) {
		if ((ret = Flash_Sector_Erase(seqnum_sector, 1)) == HAL_OK) // erase the sector where seqnum11 is stored
			ret = HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE,
					SEQUENCE_NUM_SECTOR_11, value); // then set the sector value
	} else {
		if ((ret = Flash_Sector_Erase((seqnum_sector + 1), 1)) == HAL_OK) // or where seqnum10 is stored
			ret = HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE,
					SEQUENCE_NUM_SECTOR_10, value);
	}

	HAL_FLASH_Lock();
	return ret;
}

/* Implements the saving mechanisms
   to Flash Sector10 */
static HAL_StatusTypeDef Flash_Write_Sector10(uint8_t *sector, uint8_t *seqnum10) {
	HAL_StatusTypeDef ret;
	uint8_t checksum = 0xFF;
	uint8_t erase_sector = FLASH_SECTOR_10;
	FRESULT filestatus;
	DIR dir;
	FILINFO fileinfo;

	if ((ret = HAL_FLASH_Unlock()) != HAL_OK)
	  return ret;
	if ((ret = Flash_Sector_Erase(&erase_sector, 1)) != HAL_OK)
		return ret;

	filestatus = f_opendir(&dir, (TCHAR const*) USERPath); /* Open the directory */
	if (filestatus == FR_OK) {
		for (uint32_t j = 0; j < 192; j++) {

			filestatus = f_readdir(&dir, &fileinfo); /* Read a directory item */
			if (filestatus != FR_OK || fileinfo.fname[0] == 0)
				break; /* Break on error or end of dir */
			if(!strcmp(fileinfo.fname,"SYSTEM~1")){ // Fatfs is empty at the start
				j--;
				continue;
			}

			if (Fat_Read(fileinfo.fname, (void*) sector) != FR_OK)
				break;

			for (uint32_t i = 0; i < RECORD; i++) { //Write one record to the Flash, one cycle writes a byte, so 512 cycles for a whole record
				ret = HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE,
						SECTOR_10 + i + (j * RECORD), sector[i]);
				if (ret != HAL_OK) {
					HAL_FLASH_Lock();
					f_closedir(&dir);
					return ret;
				}
				checksum ^= sector[i];  // calculate the checksum
			}
		}
		f_closedir(&dir);

		*seqnum10 += 2;         // adding 2 makes this sector active
		checksum ^= *seqnum10;
		ret = HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, CHECKSUM_SECTOR_10, checksum); // save the checksum
		if (ret != HAL_OK) {
			HAL_FLASH_Lock();
			return ret;
		}
		ret = Write_Seqnum(10, *seqnum10);   // lastly set this sector active
		return ret;
	}

	ret = HAL_FLASH_Lock();
	return ret;
}

/* Implements the saving mechanisms
   to Flash Sector11 */
static HAL_StatusTypeDef Flash_Write_Sector11(uint8_t *sector, uint8_t *seqnum11){
	HAL_StatusTypeDef ret;
	uint8_t checksum = 0xFF;
	uint8_t erase_sector = FLASH_SECTOR_11;
	FRESULT filestatus;
	DIR dir;
	FILINFO fileinfo;

	if ((ret = HAL_FLASH_Unlock()) != HAL_OK)
		return ret;
	if ((ret = Flash_Sector_Erase(&erase_sector, 1)) != HAL_OK)
		return ret;

	filestatus = f_opendir(&dir, (TCHAR const*) USERPath); /* Open the directory */
	if (filestatus == FR_OK) {
		for (uint32_t j = 0; j < 192; j++) {

			filestatus = f_readdir(&dir, &fileinfo); /* Read a directory item */
			if (filestatus != FR_OK || fileinfo.fname[0] == 0)
				break; /* Break on error or end of dir */
			if (!strcmp(fileinfo.fname, "SYSTEM~1")) // if Fatfs is empty at the start
				if (f_unlink("SYSTEM~1") != FR_OK) {
					j--;
					continue;
				}

			if (Fat_Read(fileinfo.fname, (void*) sector) != FR_OK){
				ret = HAL_ERROR;
				return ret;
			}

			for (uint32_t i = 0; i < RECORD; i++) { //Write one record to the Flash, one cycle writes a byte, so 512 cycles for a whole record
				ret = HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE,
						SECTOR_11 + i + (j * RECORD), sector[i]);
				if (ret != HAL_OK) {
					HAL_FLASH_Lock();
					f_closedir(&dir);
					return ret;
				}
				checksum ^= sector[i];  // calculate the checksum
			}
		}
		f_closedir(&dir);

		*seqnum11 += 2;         // adding 2 makes this sector active
		checksum ^= *seqnum11;
		ret = HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, CHECKSUM_SECTOR_11, checksum); // save the checksum
		if (ret != HAL_OK) {
			HAL_FLASH_Lock();
			return ret;
		}
		ret = Write_Seqnum(11, *seqnum11);   // lastly set this sector active
		return ret;
	}

	ret = HAL_FLASH_Lock();
	return ret;
}
