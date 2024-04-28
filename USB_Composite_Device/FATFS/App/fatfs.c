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
#include "fatfs.h"

uint8_t retUSER;    /* Return value for USER */
char USERPath[4];   /* USER logical drive path */
FATFS USERFatFS;    /* File system object for USER logical drive */
FIL USERFile;       /* File object for USER */

/* USER CODE BEGIN Variables */
FATFS* fat_ptr = &USERFatFS;
FRESULT ret;
/* USER CODE BEGIN PRIVATE_DEFINES */
/* USER CODE END PRIVATE_DEFINES */

/* USER CODE BEGIN PFP */
FRESULT Fat_Write(const char* filename,const uint8_t* write);
FRESULT Fat_Write_Init(const char* filename, const uint8_t* write);
FRESULT Fat_Read(const char* filename, uint8_t* buff);
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
	if ((ret = Fat_Write_Init(filename, write)) == FR_OK)
		if (Flash_Write() != HAL_OK) // saves the data in flash as well
			ret = FR_DISK_ERR;
	return ret;
}

FRESULT Fat_Write_Init(const char *filename, const uint8_t *write){
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

/* Transmits the name of
   all files in the Fatfs to the user*/
FRESULT Fat_Read_Filenames(){
	DIR dir;
	FILINFO fileinfo;
	uint16_t i = 0;

	if((ret = f_opendir(&dir, (TCHAR const*) USERPath)) != FR_OK)
		return ret;

	for (; i < 192; i++) {
		ret = f_readdir(&dir, &fileinfo); // Read a directory item
		if (ret != FR_OK || fileinfo.fname[0] == 0)
			break; // Break on error or end of dir
		if (!strcmp(fileinfo.fname, "SYSTEM~1")) // if Fatfs is empty at the start
			if (f_unlink("SYSTEM~1") != FR_OK) {
				i--;
				continue;
			}
		Transmit("\n\r");
		Transmit((uint8_t*) fileinfo.fname);
	}
	Transmit("\n\r");

	if(i == 0){
		Transmit("There aren't any passwords saved on the device!\n\r");
		ret = FR_INVALID_PARAMETER;
	}

	f_closedir(&dir);
	return ret;
}
/* USER CODE END Application */
