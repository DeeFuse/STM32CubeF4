/**
  @page DMA_FLASHToRAM DMA FLASH To RAM Example
  
  @verbatim
  ******************** (C) COPYRIGHT 2017 STMicroelectronics *******************
  * @file    DMA/DMA_FLASHToRAM/readme.txt 
  * @author  MCD Application Team
  * @brief   Description of the DMA FLASH to RAM example.
  ******************************************************************************
  *
  * Copyright (c) 2017 STMicroelectronics. All rights reserved.
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                       opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
   @endverbatim

@par Example Description 

This example provides a description of how to use a DMA channel to transfer 
a word data buffer from Flash memory to embedded SRAM through 
the HAL API.

At the beginning of the main program the HAL_Init() function is called to reset 
all the peripherals, initialize the Flash interface and the systick.
Then the SystemClock_Config() function is used to configure the system
clock (SYSCLK) to run at 100 MHz.

DMA2_Stream0 is configured to transfer the contents of a 32-word data 
buffer stored in Flash memory to the reception buffer declared in RAM.

The start of transfer is triggered by software. DMA2_Stream0 memory-to-memory
transfer is enabled. Source and destination addresses incrementing is also enabled.
The transfer is started by setting the stream enable bit for DMA2_Stream0.
At the end of the transfer a Transfer Complete interrupt is generated since it
is enabled and the callback function (customized by user) is called.

Board's LEDs can be used to monitor the transfer status:
 - LED1 is ON when the transfer is complete (into the Transfer Complete interrupt
   routine).
 - LED2 is ON when there is a transfer error 
 - LED3 is ON when a Error_Handler is called
 
It is possible to select a different stream for the DMA transfer
example by modifying defines values in the file main.h.
Note that only DMA2 stream are able to perform Memory-to-Memory transfers.

@note Care must be taken when using HAL_Delay(), this function provides accurate delay (in milliseconds)
      based on variable incremented in SysTick ISR. This implies that if HAL_Delay() is called from
      a peripheral ISR process, then the SysTick interrupt must have higher priority (numerically lower)
      than the peripheral interrupt. Otherwise the caller ISR process will be blocked.
      To change the SysTick interrupt priority you have to use HAL_NVIC_SetPriority() function.
      
@note The application need to ensure that the SysTick time base is always set to 1 millisecond
      to have correct HAL operation.

@par Keywords

System, DMA, Data Transfer, Memory to memory, Stream, Flash, RAM

@par Directory contents 

  - DMA/DMA_FLASHToRAM/Src/system_stm32f4xx.c    STM32F4xx system clock configuration file
  - DMA/DMA_FLASHToRAM/Src/stm32f4xx_it.c        Interrupt handlers
  - DMA/DMA_FLASHToRAM/Src/main.c                Main program
  - DMA/DMA_FLASHToRAM/Inc/stm32f4xx_hal_conf.h  HAL Configuration file
  - DMA/DMA_FLASHToRAM/Inc/stm32f4xx_it.h        Interrupt handlers header file
  - DMA/DMA_FLASHToRAM/Inc/main.h                Main program header file

     
@par Hardware and Software environment

  - This example runs on STM32F412Zx devices.

  - This example has been tested with STMicroelectronics NUCLEO-F412ZG Rev B
    evaluation boards and can be easily tailored to any other supported device 
    and development board.    

@par How to use it ? 

In order to make the program work, you must do the following :
 - Open your preferred toolchain 
 - Rebuild all files and load your image into target memory
 - Run the example

   
 * <h3><center>&copy; COPYRIGHT STMicroelectronics</center></h3>
 */
