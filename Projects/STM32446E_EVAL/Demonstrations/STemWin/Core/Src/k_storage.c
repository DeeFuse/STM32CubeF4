/**
  ******************************************************************************
  * @file    k_storage.c
  * @author  MCD Application Team
  * @brief   This file provides the kernel storage functions
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
/* Includes ------------------------------------------------------------------*/
#include "k_storage.h"

/** @addtogroup CORE
  * @{
  */

/** @defgroup KERNEL_STORAGE
  * @brief Kernel storage routines
  * @{
  */


/* External variables --------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
static struct {
  U32 Mask;
  char c;
} _aAttrib[] = {
  { AM_RDO, 'R' },
  { AM_HID, 'H' },
  { AM_SYS, 'S' },
  { AM_DIR, 'D' },
  { AM_ARC, 'A' },
};
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
FATFS USBDISK_FatFs;         /* File system object for USB disk logical drive */
FATFS mSDDISK_FatFs;         /* File system object for USB disk logical drive */
char USBDISK_Drive[4];       /* USB Host logical drive number */
char mSDDISK_Drive[4];       /* USB Host logical drive number */
USBH_HandleTypeDef  hUSB_Host;
osMessageQId StorageEvent;
DIR dir;
static char         acAttrib[10];
static char         acExt[FILEMGR_MAX_EXT_SIZE];

static uint32_t StorageStatus[NUM_DISK_UNITS];
extern uint8_t  CAMERA_Configured;
/* Private function prototypes -----------------------------------------------*/
static void StorageThread(void const * argument);
static void USBH_UserProcess  (USBH_HandleTypeDef *phost, uint8_t id);
static void GetExt(char * pFile, char * pExt);
/* Private functions ---------------------------------------------------------*/


/**
  * @brief  Storage drives initialization
  * @param  None 
  * @retval None
  */
void k_StorageInit(void)
{
  /* Link the USB Host disk I/O driver */
   FATFS_LinkDriver(&USBH_Driver, USBDISK_Drive);
  
  /* Link the micro SD disk I/O driver */
   FATFS_LinkDriver(&SD_Driver, mSDDISK_Drive);  

  /* Create USB background task */
  osThreadDef(STORAGE_Thread, StorageThread, osPriorityBelowNormal, 0, 2 * configMINIMAL_STACK_SIZE);
  osThreadCreate (osThread(STORAGE_Thread), NULL);
  
  /* Create Storage Message Queue */
  osMessageQDef(osqueue, 10, uint16_t);
  StorageEvent = osMessageCreate (osMessageQ(osqueue), NULL);
  
  /* Init Host Library */
  USBH_Init(&hUSB_Host, USBH_UserProcess, 0);
  
  /* Add Supported Class */
  USBH_RegisterClass(&hUSB_Host, USBH_MSC_CLASS);
  
  /* Start Host Process */
  USBH_Start(&hUSB_Host);
  
  /* Enable SD Interrupt mode */
  BSP_SD_Init();
  BSP_SD_ITConfig();
  
  if(BSP_SD_IsDetected())
  {
    osMessagePut ( StorageEvent, MSDDISK_CONNECTION_EVENT, 0);
  }
}

/**
  * @brief  Storage Thread
  * @param  argument: pointer that is passed to the thread function as start argument.
  * @retval None
  */
static void StorageThread(void const * argument)
{
  osEvent event;
  
  for( ;; )
  {
    event = osMessageGet( StorageEvent, osWaitForever );
    
    if( event.status == osEventMessage )
    {
      switch(event.value.v)
      {
      case USBDISK_CONNECTION_EVENT:
        f_mount(&USBDISK_FatFs,USBDISK_Drive,  0);
        StorageStatus[USB_DISK_UNIT] = 1;
        break;
        
      case USBDISK_DISCONNECTION_EVENT:
        f_mount(0, USBDISK_Drive, 0);
        StorageStatus[USB_DISK_UNIT] = 0;
        break;  
        
      case MSDDISK_CONNECTION_EVENT:
        f_mount(&mSDDISK_FatFs, mSDDISK_Drive, 0);
        StorageStatus[MSD_DISK_UNIT] = 1;
        break;
        
      case MSDDISK_DISCONNECTION_EVENT:
        f_mount(0, mSDDISK_Drive, 0);
        StorageStatus[MSD_DISK_UNIT] = 0;        
        break;  
        
      }
    }
  }
}

/**
  * @brief  Storage get status
  * @param  unit: logical storage unit index.
  * @retval int
  */
uint8_t k_StorageGetStatus (uint8_t unit)
{  
  return StorageStatus[unit];
}

/**
  * @brief  Storage get capacity
  * @param  unit: logical storage unit index.
  * @retval int
  */
uint32_t k_StorageGetCapacity (uint8_t unit)
{  
  uint32_t   tot_sect = 0;
  FATFS *fs;
  
  if(unit == 0)
  {
    fs = &USBDISK_FatFs;
    tot_sect = (fs->n_fatent - 2) * fs->csize;
    
  }
  else if (unit == 1)
  {
    fs = &mSDDISK_FatFs;
    tot_sect = (fs->n_fatent - 2) * fs->csize;
    
  }
  return (tot_sect);
}

/**
  * @brief  Storage get free space
  * @param  unit: logical storage unit index. 
  * @retval int
  */
uint32_t k_StorageGetFree (uint8_t unit)
{ 
  uint32_t   fre_clust = 0;
  FATFS *fs;
  FRESULT res = FR_INT_ERR;
  
  if(unit == 0)
  {
    fs = &USBDISK_FatFs;
    res = f_getfree("0:", (DWORD *)&fre_clust, &fs);
  }
  else if (unit == 1)
  {
    fs = &mSDDISK_FatFs;
    res = f_getfree("1:", (DWORD *)&fre_clust, &fs);
  }
  if(res == FR_OK)
  {
    return (fre_clust * fs->csize);
  }
  else
  {
    return 0;
  }
}
/**
  * @brief  User Process
  * @param  phost: Host handle
  * @param  id:    Host Library user message ID
  * @retval None
  */
static void USBH_UserProcess  (USBH_HandleTypeDef *phost, uint8_t id)
{  
  switch (id)
  { 
  case HOST_USER_SELECT_CONFIGURATION:
    break;
    
  case HOST_USER_DISCONNECTION:
    osMessagePut ( StorageEvent, USBDISK_DISCONNECTION_EVENT, 0);
    break;
    
  case HOST_USER_CLASS_ACTIVE:
    osMessagePut ( StorageEvent, USBDISK_CONNECTION_EVENT, 0);
    break;
  }
}

/**
  * @brief EXTI line detection callbacks.
  * @param GPIO_Pin: Specifies the pins connected EXTI line
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if(GPIO_Pin == MFX_IRQOUT_PIN)
  {   
    if(BSP_SD_IsDetected())
    {
      if(CAMERA_Configured == 0)
      {
        BSP_SD_Init();
      }
        osMessagePut ( StorageEvent, MSDDISK_CONNECTION_EVENT, 0);     
    }
    else
    {
      osMessagePut ( StorageEvent, MSDDISK_DISCONNECTION_EVENT, 0);
    }
  }
}

/**
  * @brief  Return file extension and removed from file name.
  * @param  pFile: pointer to the file name.
  * @param  pExt:  pointer to the file extension
  * @retval None
  */
static void GetExt(char * pFile, char * pExt) 
{
  int Len;
  int i;
  int j;
  
  /* Search beginning of extension */
  Len = strlen(pFile);
  for (i = Len; i > 0; i--) {
    if (*(pFile + i) == '.') {
      *(pFile + i) = '\0';     /* Cut extension from filename */
      break;
    }
  }
  
  /* Copy extension */
  j = 0;
  while (*(pFile + ++i) != '\0') {
    *(pExt + j) = *(pFile + i);
    j++;
  }
  *(pExt + j) = '\0';          /* Set end of string */
}

/**
  * @brief  Return the extension Only
  * @param  pFile: pointer to the file name.
  * @param  pExt:  pointer to the file extension
  * @retval None
  */
void k_GetExtOnly(char * pFile, char * pExt) 
{
  int Len;
  int i;
  int j;
  
  /* Search beginning of extension */
  Len = strlen(pFile);
  for (i = Len; i > 0; i--) {
    if (*(pFile + i) == '.') {
      break;
    }
  }
  
  /* Copy extension */
  j = 0;
  while (*(pFile + ++i) != '\0') {
    *(pExt + j) = *(pFile + i);
    j++;
  }
  *(pExt + j) = '\0';          /* Set end of string */
}
/**
  * @brief  This function is responsible to pass information about the requested file
  * @param  pInfo: Pointer to structure which contains all details of the requested file.
  * @retval None
  */
int k_GetData(CHOOSEFILE_INFO * pInfo)
{
  char                c;
  int                 i;
  char               tmp[CHOOSEFILE_MAXLEN];  
  FRESULT res = FR_INT_ERR;
  char *fn;
  FILINFO fno; 
  
  switch (pInfo->Cmd) 
  {
  case CHOOSEFILE_FINDFIRST:
    f_closedir(&dir); 
    
    /* reformat path */
    memset(tmp, 0, CHOOSEFILE_MAXLEN);
    strcpy(tmp, pInfo->pRoot);
    
    for(i= CHOOSEFILE_MAXLEN; i > 0; i--)
    {
      if(tmp[i] == '/')
      {
        tmp[i] = 0;
        break;
      }
    }
    
    res = f_opendir(&dir, tmp);
    
    if (res == FR_OK)
    {
      
      res = f_readdir(&dir, &fno);
    }
    break;
    
  case CHOOSEFILE_FINDNEXT:
    res = f_readdir(&dir, &fno);
    break;
  }
  
  if (res == FR_OK)
  {
    fn = fno.fname;    
    
    while (((fno.fattrib & AM_DIR) == 0) && (res == FR_OK))
    {
      
      if((strstr(pInfo->pMask, ".img")))
      {
        if((strstr(fn, ".bmp")) || (strstr(fn, ".jpg")) || (strstr(fn, ".BMP")) || (strstr(fn, ".JPG")))
        {
          break;
        }
        else
        {
          res = f_readdir(&dir, &fno);
          
          if (res != FR_OK || fno.fname[0] == 0)
          {
            f_closedir(&dir); 
            return 1;
          }
          else
          {
            fn = fno.fname;
          }
        }
        
      }
      else if((strstr(pInfo->pMask, ".audio")))
      {
        if((strstr(fn, ".wav")) || (strstr(fn, ".WAV")))
        {
          break;
        }
        else
        {
          res = f_readdir(&dir, &fno);
          
          if (res != FR_OK || fno.fname[0] == 0)
          {
            f_closedir(&dir); 
            return 1;
          }
          else
          {
            fn = fno.fname;
          }
        }
        
      }     
      else if(strstr(fn, pInfo->pMask) == NULL)
      {
        
        res = f_readdir(&dir, &fno);
        
        if (res != FR_OK || fno.fname[0] == 0)
        {
          f_closedir(&dir); 
          return 1;
        }
        else
        {
          fn = fno.fname;
        }
      }
      else
      {
        break;
      }
    }   
    
    if(fn[0] == 0)
    {
      f_closedir(&dir); 
      return 1;
    } 
    
    pInfo->Flags = ((fno.fattrib & AM_DIR) == AM_DIR) ? CHOOSEFILE_FLAG_DIRECTORY : 0;
    
    for (i = 0; i < GUI_COUNTOF(_aAttrib); i++)
    {
      if (fno.fattrib & _aAttrib[i].Mask)
      {
        c = _aAttrib[i].c;
      }
      else
      {
        c = '-';
      }
      acAttrib[i] = c;
    }
    if((fno.fattrib & AM_DIR) == AM_DIR)
    {
      acExt[0] = 0;
    }
    else
    {
      GetExt(fn, acExt);
    }
    pInfo->pAttrib = acAttrib;
    pInfo->pName = fn;
    pInfo->pExt = acExt;
    pInfo->SizeL = fno.fsize;
    pInfo->SizeH = 0;
    
  }
  return res;
}

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
