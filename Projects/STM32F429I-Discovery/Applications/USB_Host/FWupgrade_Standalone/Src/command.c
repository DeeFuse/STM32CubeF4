/**
  ******************************************************************************
  * @file    USB_Host/FWupgrade_Standalone/Src/command.c
  * @author  MCD Application Team
  * @brief   This file provides all the IAP command functions.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------ */
#include "main.h"

/* Private typedef ----------------------------------------------------------- */
/* Private defines ----------------------------------------------------------- */
#define UPLOAD_FILENAME            "0:UPLOAD.BIN"
#define DOWNLOAD_FILENAME          "0:image.BIN"

/* Private macros ------------------------------------------------------------ */
/* Private variables --------------------------------------------------------- */
static uint32_t TmpReadSize = 0x00;
static uint32_t RamAddress = 0x00;
static __IO uint32_t LastPGAddress = APPLICATION_ADDRESS;
static uint8_t RAM_Buf[BUFFER_SIZE] = { 0x00 };

FATFS USBH_fatfs;
FIL MyFile;                     /* File object for upload operation */
FIL MyFileR;                    /* File object for download operation */
extern DIR dir;
extern FILINFO fno;

/* Private function prototypes ----------------------------------------------- */
static void COMMAND_ProgramFlashMemory(void);

/* Private functions --------------------------------------------------------- */

/**
  * @brief  IAP Read all flash memory.
  * @param  None
  * @retval None
  */
void COMMAND_Upload(void)
{
  __IO uint32_t address = APPLICATION_ADDRESS;
  __IO uint32_t counterread = 0x00;
  uint32_t tmpcounter = 0x00, indexoffset = 0x00;
  FlagStatus readoutstatus = SET;
  uint16_t byteswritten;

  /* Get the read out protection status */
  readoutstatus = FLASH_If_ReadOutProtectionStatus();
  if (readoutstatus == RESET)
  {
    /* Remove UPLOAD file if it exists on flash disk */
    f_unlink(UPLOAD_FILENAME);

    /* Init written byte counter */
    indexoffset = (APPLICATION_ADDRESS - USER_FLASH_STARTADDRESS);

    /* Open binary file to write on it */
    if ((Appli_state == APPLICATION_READY) &&
        (f_open(&MyFile, UPLOAD_FILENAME, FA_CREATE_ALWAYS | FA_WRITE) ==
         FR_OK))
    {
      /* Upload On Going: Turn LED4 On and LED3 Off */
      BSP_LED_On(LED4);
      BSP_LED_Off(LED3);

      /* Read flash memory */
      while ((indexoffset < USER_FLASH_SIZE) &&
             (Appli_state == APPLICATION_READY))
      {
        for (counterread = 0; counterread < BUFFER_SIZE; counterread++)
        {
          /* Check the read bytes versus the end of flash */
          if (indexoffset + counterread < USER_FLASH_SIZE)
          {
            tmpcounter = counterread;
            RAM_Buf[tmpcounter] = (*(uint8_t *) (address++));
          }
          /* In this case all flash was read */
          else
          {
            break;
          }
        }

        /* Write buffer to file */
        f_write(&MyFile, RAM_Buf, BUFFER_SIZE, (void *)&byteswritten);

        /* Number of byte written */
        indexoffset = indexoffset + counterread;
      }

      /* Turn LED3 On: Upload Done */
      BSP_LED_On(LED3);

      /* Close file and filesystem */
      f_close(&MyFile);
      f_mount(0, 0, 0);
    }
    /* Keep LED4 OFF when Device connected */
    BSP_LED_Off(LED4);
  }
  else
  {
    /* Message ROP active: Turn LED3 On and Toggle LED4 in infinite loop */
    BSP_LED_On(LED3);
    Fail_Handler();
  }
}

/**
  * @brief  IAP Write Flash memory.
  * @param  None
  * @retval None
  */
void COMMAND_Download(void)
{
  /* Open the binary file to be downloaded */
  if (f_open(&MyFileR, DOWNLOAD_FILENAME, FA_OPEN_EXISTING | FA_READ) == FR_OK)
  {
if (f_size(&MyFileR) > USER_FLASH_SIZE)
    {
      /* No available Flash memory size for the binary file: Toggle LED4 in
       * infinite loop */
      Fail_Handler();
    }
    else
    {
      /* Download On Going: Turn LED4 On */
      BSP_LED_On(LED4);
      BSP_LED_Off(LED3);

      /* Erase FLASH sectors to download image */
      if (FLASH_If_EraseSectors(APPLICATION_ADDRESS) != 0x00)
      {
        /* Flash erase error: Toggle LED4 in infinite loop */
        BSP_LED_Off(LED4);
        Erase_Fail_Handler();
      }

      /* Program flash memory */
      COMMAND_ProgramFlashMemory();

      /* Download Done: Turn LED3 On and LED4 Off */
      BSP_LED_On(LED3);
      BSP_LED_Off(LED4);

      /* Close file */
      f_close(&MyFileR);
    }
  }
  else
  {
    /* The binary file is not available: Toggle LED4 in infinite loop */
    Fail_Handler();

  }
}

/**
  * @brief  IAP jump to user program.
  * @param  None
  * @retval None
  */
void COMMAND_Jump(void)
{
  /* Software reset */
  NVIC_SystemReset();
}

/**
  * @brief  Programs the internal Flash memory.
  * @param  None
  * @retval None
  */
static void COMMAND_ProgramFlashMemory(void)
{
  uint32_t programcounter = 0x00;
  uint8_t readflag = TRUE;
  uint16_t bytesread;

  /* RAM Address Initialization */
  RamAddress = (uint32_t) & RAM_Buf;

  /* Erase address init */
  LastPGAddress = APPLICATION_ADDRESS;

  /* While file still contain data */
  while ((readflag == TRUE))
  {
    /* Read maximum 512 Kbyte from the selected file */
    f_read(&MyFileR, RAM_Buf, BUFFER_SIZE, (void *)&bytesread);

    /* Temp variable */
    TmpReadSize = bytesread;

    /* The read data < "BUFFER_SIZE" Kbyte */
    if (TmpReadSize < BUFFER_SIZE)
    {
      readflag = FALSE;
    }

    /* Program flash memory */
    for (programcounter = 0; programcounter < TmpReadSize; programcounter += 4)
    {
      /* Write word into flash memory */
      if (FLASH_If_Write((LastPGAddress + programcounter),
                         *(uint32_t *) (RamAddress + programcounter)) != 0x00)
      {
        /* Flash programming error: Turn LED3 On and Toggle LED4 in infinite
         * loop */
        BSP_LED_On(LED3);
        Fail_Handler();
      }
    }
    /* Update last programmed address value */
    LastPGAddress += TmpReadSize;
  }
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
