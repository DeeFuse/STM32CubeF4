/**
  ******************************************************************************
  * @file    USB_Host/AUDIO_Standalone/Src/stm32f4xx_it.c 
  * @author  MCD Application Team
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------ */
#include "main.h"
#include "stm32f4xx_it.h"

/* Private typedef ----------------------------------------------------------- */
/* Private define ------------------------------------------------------------ */
/* Private macro ------------------------------------------------------------- */
/* Private variables --------------------------------------------------------- */
extern HCD_HandleTypeDef hhcd;
/* uSD handler declared in "stm32412g_discovery_sd.c" file */
extern SD_HandleTypeDef uSdHandle;
/* Private function prototypes ----------------------------------------------- */
/* Private functions --------------------------------------------------------- */

/******************************************************************************/
/* Cortex-M4 Processor Exceptions Handlers */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
  HAL_IncTick();
  Toggle_Leds();
}

/******************************************************************************/
/* STM32F4xx Peripherals Interrupt Handlers */
/* Add here the Interrupt Handler for the used peripheral(s) (PPP), for the */
/* available peripheral interrupt handler's name please refer to the startup */
/* file (startup_stm32f4xx.s).  */
/******************************************************************************/

/**
  * @brief  This function handles USB-On-The-Go FS/HS global interrupt request.
  * @param  None
  * @retval None
  */
void OTG_FS_IRQHandler(void)
{
  HAL_HCD_IRQHandler(&hhcd);
}

/**
  * @brief  This function handles DMA2 Stream 3 interrupt request.
  * @param  None
  * @retval None
  */
void DMA2_Stream3_IRQHandler(void)
{
  HAL_DMA_IRQHandler(uSdHandle.hdmarx);
}

/**
  * @brief  This function handles DMA2 Stream 6 interrupt request.
  * @param  None
  * @retval None
  */
void DMA2_Stream6_IRQHandler(void)
{
  HAL_DMA_IRQHandler(uSdHandle.hdmatx);
}

/**
  * @brief  This function handles SDIO interrupt request.
  * @param  None
  * @retval None
  */
void SDIO_IRQHandler(void)
{
  /* Process All SDIO Interrupt Sources */
  HAL_SD_IRQHandler(&uSdHandle);
}

/**
  * @brief  This function handles External line 0 interrupt request.
  * @param  None
  * @retval None
  */
void EXTI0_IRQHandler(void)
{
  if (HAL_GPIO_ReadPin(UP_JOY_GPIO_PORT, UP_JOY_PIN) == GPIO_PIN_SET)
  {
    HAL_GPIO_EXTI_IRQHandler(UP_JOY_PIN);
  }
  else
  {
    HAL_GPIO_EXTI_IRQHandler(SEL_JOY_PIN);
  }
}

/**
  * @brief  This function handles External line 1 interrupt request.
  * @param  None
  * @retval None
  */
void EXTI1_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(DOWN_JOY_PIN);
}

/**
  * @brief  This function handles External line 10..15 interrupts request.
  * @param  None
  * @retval None
  */
void EXTI15_10_IRQHandler(void)
{
  if (HAL_GPIO_ReadPin(RIGHT_JOY_GPIO_PORT, RIGHT_JOY_PIN) == GPIO_PIN_SET)
  {
    HAL_GPIO_EXTI_IRQHandler(RIGHT_JOY_PIN);
  }
  else
  {
    HAL_GPIO_EXTI_IRQHandler(LEFT_JOY_PIN);
  }
}

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/* void PPP_IRQHandler(void) { } */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
