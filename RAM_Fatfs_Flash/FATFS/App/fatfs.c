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
#define FLASH_START   (  0x080E0000 )    //Flash Address (Sector 11) 128kByte
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
FRESULT Fat_Write(const char* filename,const uint8_t* write, uint32_t length);
FRESULT Fat_Read(const char* filename, uint8_t* buff, uint32_t length);
void Flash_Read_Init();
HAL_StatusTypeDef Flash_Write();
/* USER CODE END PFP */

/* USER CODE END Variables */

void MX_FATFS_Init(void)
{
	/*## FatFS: Link the USER driver ###########################*/
	retUSER = FATFS_LinkDriver(&USER_Driver, USERPath);

	/* USER CODE BEGIN Init */
	/* additional user code for init */
	BYTE buff[RECORD];       // Work area (larger is better for processing time)
	/*char filename[15] = { 0 };
	uint8_t name_char = 0;     // move the filename from buff
	uint8_t flash_empty;       // true if everything from flash has been moved to FatFs*/

	if (f_mkfs((TCHAR const*) USERPath, FM_ANY, 0, buff, sizeof buff) == FR_OK) // format the RAM as FatFs
		if (f_mount(&USERFatFS, (TCHAR const*) USERPath, 1) == FR_OK) { // registers filesystem object to FatFs

			/*for (uint32_t i = 0; i < 192; i++) { // copy the whole Flash to FatFs
				name_char = 0;
				memcpy(buff, (const void*) (FLASH_START + (i * RECORD)), RECORD); // load records from Flash to buffer, one record is 512 byte
				for (char *c = buff; *c != '.'; c++) { // record starts with filename, after the name comes a '.' character
					filename[name_char] = buff[name_char];
					name_char++;
					if (name_char == 15) { // if we didn't find the name at the start of the record, there are no more records
						flash_empty = 1;
						break;
					}
				}
				if (flash_empty)
					break;
				if (Fat_Write((strcat(filename, ".txt")), (const uint8_t*) buff, // load the data from the buff to FatFs
						512) != FR_OK)
					break;*/
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

FRESULT Fat_Write(const char* filename, const uint8_t* write, uint32_t length){
	FRESULT ret;
	DWORD free_clusters;  // Number of free clusters
	uint32_t wbytes;     // File write counts
	if ( (ret = f_getfree((TCHAR const*) USERPath, &free_clusters, &fat_ptr))
			== FR_OK)
		if ( (ret = f_open(&USERFile, filename, FA_CREATE_ALWAYS | FA_WRITE)) == FR_OK
				&& free_clusters)
			if ( (ret = f_write(&USERFile,(const void *) write, length, (void*) &wbytes))
					== FR_OK) {
				f_close(&USERFile);
				if (Flash_Write() != HAL_OK)
					ret = FR_DISK_ERR;
				return ret;
			}
	f_close(&USERFile);
	return ret;
}

FRESULT Fat_Write_Init(const char* filename, const uint8_t* write, uint32_t length){
	FRESULT ret;
	DWORD free_clusters;  // Number of free clusters
	uint32_t wbytes;     // File write counts
	if ( (ret = f_getfree((TCHAR const*) USERPath, &free_clusters, &fat_ptr))
			== FR_OK)
		if ( (ret = f_open(&USERFile, filename, FA_CREATE_ALWAYS | FA_WRITE)) == FR_OK
				&& free_clusters)
			if ( (ret = f_write(&USERFile,(const void *) write, length, (void*) &wbytes))
					== FR_OK) {}
	f_close(&USERFile);
	return ret;
}

FRESULT Fat_Read(const char* filename, uint8_t* buff, uint32_t length){
	FRESULT ret;
	uint32_t rbytes;     // File read counts
	if ((ret = f_open(&USERFile, filename, FA_OPEN_EXISTING | FA_READ))
			== FR_OK)
		if ((ret = (f_read(&USERFile, (void*) buff, length, (void*) &rbytes)))
				== FR_OK) {
			f_close(&USERFile);
			return ret;
		}
	f_close(&USERFile);
	return ret;

}

void Flash_Read_Init(){
	uint8_t buff[RECORD];
	char filename[13] = { 0 };
	uint8_t name_char = 0;     // move the filename from buff
	uint8_t flash_empty; // true if everything from flash has been moved to FatFs

	for (uint32_t i = 0; i < 192; i++) { // copy the whole Flash to FatFs
		name_char = 0;
		memcpy(buff, (const void*) (FLASH_START + (i * RECORD)), RECORD); // load records from Flash to buffer, one record is 512 byte
		for (char *c = buff; *c != '.'; c++) { // record starts with filename, after the name comes a '.' character
			filename[name_char] = buff[name_char];
			name_char++;
			if (name_char == 13) { // if we didn't find the name at the start of the record, there are no more records
				flash_empty = 1;
				break;
			}
		}
		if (flash_empty)
			break;
		if (Fat_Write_Init((strcat(filename, ".txt")), (const uint8_t*) buff, // load the data from the buff to FatFs
				512) != FR_OK)
			break;
	}

}

HAL_StatusTypeDef Flash_Write() {
	uint8_t sector[RECORD] = {0};
	HAL_StatusTypeDef ret = HAL_OK;
	FRESULT file;
	DIR dir;
	FILINFO info;

	ret = HAL_FLASH_Unlock();
	if (ret != HAL_OK) { return RES_ERROR; }

	FLASH_EraseInitTypeDef EraseInitStruct;
	uint32_t SectorError;
	EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
	EraseInitStruct.Sector = FLASH_SECTOR_11;
	EraseInitStruct.NbSectors = 1;                    //erase 1 sector(11)
	EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
	EraseInitStruct.Banks = FLASH_BANK_1;
	ret = HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError);

	file = f_opendir(&dir, (TCHAR const*) USERPath); /* Open the directory */
	if (file == FR_OK) {
		for (uint32_t j = 0; j < 128; j++) {

			file = f_readdir(&dir, &info); /* Read a directory item */
			if (file != FR_OK || info.fname[0] == 0) break; /* Break on error or end of dir */
			if (Fat_Read(info.fname, (void*) sector, 512) != FR_OK) break;

			for (uint32_t i = 0; i < RECORD; i++) { //Write one record to the Flash, one cycle writes a DWORD, so 8 cycles for a whole record
				ret = HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE,
						FLASH_START + i + (j * RECORD), sector[i]);
				if (ret != HAL_OK) {
					HAL_FLASH_Lock();
					f_closedir(&dir);
					return ret;
				}
			}
		}
		f_closedir(&dir);
	}

	ret = HAL_FLASH_Lock();
	return ret;
}
/* USER CODE END Application */
