/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file   fatfs.c
  * @brief  Code for fatfs applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* USER CODE BEGIN PRIVATE_DEFINES */
#define SECTOR_11 ( 0x080E0000 )    //Flash Address (Sector 11) 128kByte
#define CHECKSUM_SECTOR_11 ( 0x080F8000 ) // Address of the Sector 11 checksum
#define SEQUENCE_NUM_SECTOR_11 ( 0x080A0000 ) // Address of the Sector 11  sequence number

#define SECTOR_10 ( 0x080C0000 )    //Flash Address (Sector 10) 128kByte
#define CHECKSUM_SECTOR_10 ( 0x080D8000 ) // Address of the Sector 10 checksum
#define SEQUENCE_NUM_SECTOR_10 ( 0x08080000 ) // Address of the Sector 10 sequence number

#define RECORD   ( 512 )
/* USER CODE END PRIVATE_DEFINES */

#include "fatfs.h"

uint8_t retUSER;    /* Return value for USER */
char USERPath[4];   /* USER logical drive path */
FATFS USERFatFS;    /* File system object for USER logical drive */
FATFS* fat_ptr = &USERFatFS;
FIL USERFile;       /* File object for USER */


/* USER CODE BEGIN Variables */

/* USER CODE BEGIN PFP */
FRESULT Fat_Write(const char* filename,const uint8_t* write);
FRESULT Fat_Write_Init(const char* filename, const uint8_t* write);
FRESULT Fat_Read(const char* filename, uint8_t* buff);
void Flash_Read_Init();
uint8_t Validate_Record(uint8_t *buff, char *filename);
HAL_StatusTypeDef Verify_Sector(uint8_t *seqnum10, uint8_t *seqnum11, uint32_t *ACTIVE_SECTOR, uint8_t *buff);
HAL_StatusTypeDef Flash_Write();
HAL_StatusTypeDef Write_Seqnum(uint8_t seqnum, uint8_t value);
HAL_StatusTypeDef Flash_Write_Sector10(uint8_t *sector, uint8_t *seqnum10);
HAL_StatusTypeDef Flash_Write_Sector11(uint8_t *sector, uint8_t *seqnum11);
/* USER CODE END PFP */

/* USER CODE END Variables */

void MX_FATFS_Init(void)
{
	/*## FatFS: Link the USER driver ###########################*/
	retUSER = FATFS_LinkDriver(&USER_Driver, USERPath);

	/* USER CODE BEGIN Init */
	/* additional user code for init */
	BYTE buff[RECORD];       // Work area
	if (f_mkfs((TCHAR const*) USERPath, FM_ANY, 0, buff, sizeof buff) == FR_OK) // format the RAM as FatFs
		if (f_mount(&USERFatFS, (TCHAR const*) USERPath, 1) == FR_OK) { // registers filesystem object to FatFs
			 Flash_Read_Init();
			 return;
			}

  FATFS_UnLinkDriver(USERPath);
  /* USER CODE END Init */
}

/**
  * @brief  Gets Time from RTC
  * @param  None
  * @retval Time in DWORD
  */
DWORD get_fattime(void)
{
  /* USER CODE BEGIN get_fattime */
  return 0;
  /* USER CODE END get_fattime */
}

/* USER CODE BEGIN Application */

FRESULT Fat_Write(const char *filename, const uint8_t *write){
	FRESULT ret;
	if ((ret = Fat_Write_Init(filename, write)) == FR_OK)
		if (Flash_Write() != HAL_OK) // saves the data in flash as well
			ret = FR_DISK_ERR;
	return ret;
}

FRESULT Fat_Write_Init(const char *filename, const uint8_t *write){
	FRESULT ret;
	DWORD free_clusters;  // Number of free clusters
	uint32_t wbytes;     // File write counts
	if ( (ret = f_getfree((TCHAR const*) USERPath, &free_clusters, &fat_ptr))
			== FR_OK)
		if ( (ret = f_open(&USERFile, filename, FA_CREATE_ALWAYS | FA_WRITE)) == FR_OK
				&& free_clusters)
			if ( (ret = f_write(&USERFile,(const void *) write, RECORD, (void*) &wbytes))
					== FR_OK) {}
	f_close(&USERFile);
	return ret;
}

FRESULT Fat_Read(const char *filename, uint8_t *buff){
	FRESULT ret;
	uint32_t rbytes;     // File read counts
	if ((ret = f_open(&USERFile, filename, FA_OPEN_EXISTING | FA_READ))
			== FR_OK)
		if ((ret = (f_read(&USERFile, (void*) buff, RECORD, (void*) &rbytes)))
				== FR_OK) {
			f_close(&USERFile);
			return ret;
		}
	f_close(&USERFile);
	return ret;

}

void Flash_Read_Init(){
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
	if(Verify_Sector(&seqnum10, &seqnum11, &ACTIVE_SECTOR, buff) != HAL_OK)
		return;

	for (uint32_t i = 0; i < 192; i++) { // copy the whole Flash to FatFs
		memcpy(buff, (const void*) (ACTIVE_SECTOR + (i * RECORD)), RECORD); // load records from Flash to buffer, one record is 512 byte
		if (Validate_Record(buff, filename) != 1) // everything from flash has been moved to FatFs
			break;
		if (Fat_Write_Init((strcat(filename, ".txt")), (const uint8_t*) buff) != FR_OK)
			break;
	}

}

uint8_t Validate_Record(uint8_t *buff, char *filename){
	uint8_t name_char = 0;     // move the filename from buff
	uint8_t valid = 1;
	for (uint8_t i = 0; i < 13; i++) { // clear the filename
				filename[i] = '\0';
			}
	for (uint8_t *c = buff; *c != '.'; c++) { // record starts with filename, after the name comes a '.' character
		filename[name_char] = buff[name_char];
		name_char++;
		if (name_char == 13){ // if we didn't find the name at the start of the record, there are no more records
			valid = 0;
			return valid;
		}
	}
	return valid;
}

HAL_StatusTypeDef Verify_Sector(uint8_t *seqnum10, uint8_t *seqnum11, uint32_t *ACTIVE_SECTOR, uint8_t *buff){
	uint8_t checksum;
	uint8_t verificationsum = 0xFF;
	char filename[13];

	if (*ACTIVE_SECTOR == SECTOR_11){
		memcpy(&checksum, (const void*) CHECKSUM_SECTOR_11, 1); // reading the checksum
		verificationsum ^= *seqnum11;                           // starting the calculation of the verification sum
	}
	else if (*ACTIVE_SECTOR == SECTOR_10){
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
			if (HAL_FLASH_Unlock() != HAL_OK) { return RES_ERROR; }
			if (Write_Seqnum(11, (*seqnum10)-1) != HAL_OK) // correcting the sequence numbers to be adjacent
				return RES_ERROR;

		} else if (*ACTIVE_SECTOR == SECTOR_10) {
			*ACTIVE_SECTOR = SECTOR_11;
			if (HAL_FLASH_Unlock() != HAL_OK) { return RES_ERROR; }
			if (Write_Seqnum(10, (*seqnum11)-1) != HAL_OK) // correcting the sequence numbers to be adjacent
				return RES_ERROR;
		}
	}
	return HAL_OK;
}

HAL_StatusTypeDef Write_Seqnum(uint8_t seqnum, uint8_t value){
	HAL_StatusTypeDef ret;

	FLASH_EraseInitTypeDef EraseInitStruct;
	uint32_t SectorError;
	EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
	EraseInitStruct.NbSectors = 1;
	EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
	EraseInitStruct.Banks = FLASH_BANK_1;

	if(seqnum == 11)
		EraseInitStruct.Sector = FLASH_SECTOR_9; // erase the sector where seqnum11 is stored
	else
		EraseInitStruct.Sector = FLASH_SECTOR_8; // where seqnum10 is stored

	ret = HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError);

	if (seqnum == 11)
		ret = HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, SEQUENCE_NUM_SECTOR_11,
				value);   // lastly set the sector value
	else
		ret = HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, SEQUENCE_NUM_SECTOR_10,
				value);

	HAL_FLASH_Lock();
	return ret;
}

HAL_StatusTypeDef Flash_Write() {
	uint8_t sector[RECORD] = {0};
	uint8_t seqnum10;
	uint8_t seqnum11;
	uint32_t ACTIVE_SECTOR;
	HAL_StatusTypeDef ret = HAL_OK;

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

HAL_StatusTypeDef Flash_Write_Sector10(uint8_t *sector, uint8_t *seqnum10) {
	HAL_StatusTypeDef ret;
	uint8_t checksum = 0xFF;
	FRESULT filestatus;
	DIR dir;
	FILINFO fileinfo;

	ret = HAL_FLASH_Unlock();
	if (ret != HAL_OK) { return RES_ERROR;}
	FLASH_EraseInitTypeDef EraseInitStruct;
	uint32_t SectorError;
	EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
	EraseInitStruct.Sector = FLASH_SECTOR_10;
	EraseInitStruct.NbSectors = 1;                    //erase 1 sector(10)
	EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
	EraseInitStruct.Banks = FLASH_BANK_1;
	ret = HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError);

	filestatus = f_opendir(&dir, (TCHAR const*) USERPath); /* Open the directory */
	if (filestatus == FR_OK) {
		for (uint32_t j = 0; j < 128; j++) {

			filestatus = f_readdir(&dir, &fileinfo); /* Read a directory item */
			if (filestatus != FR_OK || fileinfo.fname[0] == 0)
				break; /* Break on error or end of dir */
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

HAL_StatusTypeDef Flash_Write_Sector11(uint8_t *sector, uint8_t *seqnum11){
	HAL_StatusTypeDef ret;
	uint8_t checksum = 0xFF;
	FRESULT filestatus;
	DIR dir;
	FILINFO fileinfo;
	ret = HAL_FLASH_Unlock();
	if (ret != HAL_OK) {
		return RES_ERROR;
	}
	FLASH_EraseInitTypeDef EraseInitStruct;
	uint32_t SectorError;
	EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
	EraseInitStruct.Sector = FLASH_SECTOR_11;
	EraseInitStruct.NbSectors = 1;                    //erase 1 sector(11)
	EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
	EraseInitStruct.Banks = FLASH_BANK_1;
	ret = HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError);

	filestatus = f_opendir(&dir, (TCHAR const*) USERPath); /* Open the directory */
	if (filestatus == FR_OK) {
		for (uint32_t j = 0; j < 128; j++) {

			filestatus = f_readdir(&dir, &fileinfo); /* Read a directory item */
			if (filestatus != FR_OK || fileinfo.fname[0] == 0)
				break; /* Break on error or end of dir */
			if (Fat_Read(fileinfo.fname, (void*) sector) != FR_OK)
				break;

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

/* USER CODE END Application */
