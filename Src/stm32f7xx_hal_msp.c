/**
  ******************************************************************************
  * File Name          : stm32f7xx_hal_msp.c
  * Description        : This file provides code for the MSP Initialization 
  *                      and de-Initialization codes.
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2017 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"

extern void _Error_Handler(char *, int);


/* --------------------------------------------------------------------------
 * Name : HAL_MspInit()
 *        Initializes the Global MSP.
 *
 * -------------------------------------------------------------------------- */
void HAL_MspInit(void)
{
   HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

   /* System interrupt init*/
   /* MemoryManagement_IRQn interrupt configuration */
   HAL_NVIC_SetPriority(MemoryManagement_IRQn, 0, 0);
   /* BusFault_IRQn interrupt configuration */
   HAL_NVIC_SetPriority(BusFault_IRQn, 0, 0);
   /* UsageFault_IRQn interrupt configuration */
   HAL_NVIC_SetPriority(UsageFault_IRQn, 0, 0);
   /* SVCall_IRQn interrupt configuration */
   HAL_NVIC_SetPriority(SVCall_IRQn, 0, 0);
   /* DebugMonitor_IRQn interrupt configuration */
   HAL_NVIC_SetPriority(DebugMonitor_IRQn, 0, 0);
   /* PendSV_IRQn interrupt configuration */
   HAL_NVIC_SetPriority(PendSV_IRQn, 0, 0);
   /* SysTick_IRQn interrupt configuration */
   HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* --------------------------------------------------------------------------
 * Name : HAL_LTDC_MspInit()
 *        
 *
 * -------------------------------------------------------------------------- */

static uint32_t FMC_Initialized                          = 0;
static void HAL_FMC_MspInit(void)
{
//   static DMA_HandleTypeDef dma_handle;
   GPIO_InitTypeDef gpio_init_structure;

   if (FMC_Initialized)
   {
      return;
   }
   FMC_Initialized                                       = 1;

   /* Enable FMC clock */
   __HAL_RCC_FMC_CLK_ENABLE();

   /* Enable chosen DMAx clock */
//   __DMAx_CLK_ENABLE();

   /* Enable GPIOs clock */
   __HAL_RCC_GPIOC_CLK_ENABLE();
   __HAL_RCC_GPIOD_CLK_ENABLE();
   __HAL_RCC_GPIOE_CLK_ENABLE();
   __HAL_RCC_GPIOF_CLK_ENABLE();
   __HAL_RCC_GPIOG_CLK_ENABLE();
   __HAL_RCC_GPIOH_CLK_ENABLE();

   /* Common GPIO configuration */
   gpio_init_structure.Mode                              = GPIO_MODE_AF_PP;
   gpio_init_structure.Pull                              = GPIO_PULLUP;
   gpio_init_structure.Speed                             = GPIO_SPEED_FAST;
   gpio_init_structure.Alternate                         = GPIO_AF12_FMC;

   /* GPIOC configuration */
   gpio_init_structure.Pin                               = GPIO_PIN_3;
   HAL_GPIO_Init(GPIOC, &gpio_init_structure);

   /* GPIOD configuration */
   gpio_init_structure.Pin                               = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_8 | GPIO_PIN_9 |
                                                           GPIO_PIN_10 | GPIO_PIN_14 | GPIO_PIN_15;
   HAL_GPIO_Init(GPIOD, &gpio_init_structure);

   /* GPIOE configuration */  
   gpio_init_structure.Pin                               = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_7 | GPIO_PIN_8 |
                                                           GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 |
                                                           GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
   HAL_GPIO_Init(GPIOE, &gpio_init_structure);

   /* GPIOF configuration */  
   gpio_init_structure.Pin                               = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 |
                                                           GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_11 | GPIO_PIN_12 |
                                                           GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
   HAL_GPIO_Init(GPIOF, &gpio_init_structure);

   /* GPIOG configuration */  
   gpio_init_structure.Pin                               = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5 |
                                                           GPIO_PIN_8 | GPIO_PIN_15;
   HAL_GPIO_Init(GPIOG, &gpio_init_structure);

   /* GPIOH configuration */  
   gpio_init_structure.Pin                               = GPIO_PIN_3 | GPIO_PIN_5;
   HAL_GPIO_Init(GPIOH, &gpio_init_structure); 

#if 0
   /* Configure common DMA parameters */
   dma_handle.Init.Channel             = SDRAM_DMAx_CHANNEL;
   dma_handle.Init.Direction           = DMA_MEMORY_TO_MEMORY;
   dma_handle.Init.PeriphInc           = DMA_PINC_ENABLE;
   dma_handle.Init.MemInc              = DMA_MINC_ENABLE;
   dma_handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
   dma_handle.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
   dma_handle.Init.Mode                = DMA_NORMAL;
   dma_handle.Init.Priority            = DMA_PRIORITY_HIGH;
   dma_handle.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;         
   dma_handle.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
   dma_handle.Init.MemBurst            = DMA_MBURST_SINGLE;
   dma_handle.Init.PeriphBurst         = DMA_PBURST_SINGLE; 

   dma_handle.Instance = SDRAM_DMAx_STREAM;

   /* Associate the DMA handle */
   __HAL_LINKDMA(hsdram, hdma, dma_handle);

   /* Deinitialize the stream for new transfer */
   HAL_DMA_DeInit(&dma_handle);

   /* Configure the DMA stream */
   HAL_DMA_Init(&dma_handle); 

   /* NVIC configuration for DMA transfer complete interrupt */
   HAL_NVIC_SetPriority(SDRAM_DMAx_IRQn, 0x0F, 0);
   HAL_NVIC_EnableIRQ(SDRAM_DMAx_IRQn);
#endif
}

/* --------------------------------------------------------------------------
 * Name : HAL_SDRAM_MspInit()
 *        
 *
 * -------------------------------------------------------------------------- */
void HAL_SDRAM_MspInit(SDRAM_HandleTypeDef* hsdram)
{
   HAL_FMC_MspInit();
}

/* --------------------------------------------------------------------------
 * Name : HAL_FMC_MspDeInit()
 *        
 *
 * -------------------------------------------------------------------------- */
static void HAL_FMC_MspDeInit(void)
{
   __HAL_RCC_FMC_CLK_DISABLE();
}

/* --------------------------------------------------------------------------
 * Name : HAL_SDRAM_MspDeInit()
 *        
 *
 * -------------------------------------------------------------------------- */
void HAL_SDRAM_MspDeInit(SDRAM_HandleTypeDef* hsdram)
{
   HAL_FMC_MspDeInit();
}

/* --------------------------------------------------------------------------
 * Name : HAL_LTDC_MspInit()
 *        
 *
 * -------------------------------------------------------------------------- */
void HAL_LTDC_MspInit(LTDC_HandleTypeDef* hltdc)
{
   GPIO_InitTypeDef gpio_init_structure;
   static RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;

   /* Enable the LTDC clocks */
   __HAL_RCC_LTDC_CLK_ENABLE();

   /* LCD clock configuration */
   /* PLLSAI_VCO Input = HSE_VALUE/PLL_M = 1 Mhz */
   /* PLLSAI_VCO Output = PLLSAI_VCO Input * PLLSAIN = 429 Mhz */
   /* PLLLCDCLK = PLLSAI_VCO Output/PLLSAIR = 429/5 = 85 Mhz */
   /* LTDC clock frequency = PLLLCDCLK / LTDC_PLLSAI_DIVR_2 = 85/4 = 21 Mhz */
   PeriphClkInitStruct.PeriphClockSelection              = RCC_PERIPHCLK_LTDC;
   PeriphClkInitStruct.PLLSAI.PLLSAIN                    = 192;
   PeriphClkInitStruct.PLLSAI.PLLSAIR                    = 5;
   PeriphClkInitStruct.PLLSAIDivR                        = RCC_PLLSAIDIVR_4;
   HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);   

   /* Enable GPIOs clock */
   __HAL_RCC_GPIOE_CLK_ENABLE();
   __HAL_RCC_GPIOG_CLK_ENABLE();
   __HAL_RCC_GPIOI_CLK_ENABLE();
   __HAL_RCC_GPIOJ_CLK_ENABLE();
   __HAL_RCC_GPIOK_CLK_ENABLE();

   /*** LTDC Pins configuration ***/
   /* GPIOE configuration */
   gpio_init_structure.Pin                               = GPIO_PIN_4;
   gpio_init_structure.Mode                              = GPIO_MODE_AF_PP;
   gpio_init_structure.Pull                              = GPIO_NOPULL;
   gpio_init_structure.Speed                             = GPIO_SPEED_FAST;
   gpio_init_structure.Alternate                         = GPIO_AF14_LTDC;
   HAL_GPIO_Init(GPIOE, &gpio_init_structure);

   /* GPIOG configuration */
   gpio_init_structure.Pin                               = GPIO_PIN_12;
   gpio_init_structure.Mode                              = GPIO_MODE_AF_PP;
   gpio_init_structure.Alternate                         = GPIO_AF9_LTDC;
   HAL_GPIO_Init(GPIOG, &gpio_init_structure);

   /* GPIOI LTDC alternate configuration */
   gpio_init_structure.Pin                               = GPIO_PIN_9 | GPIO_PIN_10 | \
                                                           GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
   gpio_init_structure.Mode                              = GPIO_MODE_AF_PP;
   gpio_init_structure.Alternate                         = GPIO_AF14_LTDC;
   HAL_GPIO_Init(GPIOI, &gpio_init_structure);

   /* GPIOJ configuration */  
   gpio_init_structure.Pin                               = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | \
                                                           GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | \
                                                           GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | \
                                                           GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
   gpio_init_structure.Mode                              = GPIO_MODE_AF_PP;
   gpio_init_structure.Alternate                         = GPIO_AF14_LTDC;
   HAL_GPIO_Init(GPIOJ, &gpio_init_structure);  

   /* GPIOK configuration */  
   gpio_init_structure.Pin                               = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_4 | \
                                                           GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
   gpio_init_structure.Mode                              = GPIO_MODE_AF_PP;
   gpio_init_structure.Alternate                         = GPIO_AF14_LTDC;
   HAL_GPIO_Init(GPIOK, &gpio_init_structure);

   /* LCD_DISP GPIO configuration */
   gpio_init_structure.Pin                               = GPIO_PIN_12;          /* LCD_DISP pin has to be manually controlled */
   gpio_init_structure.Mode                              = GPIO_MODE_OUTPUT_PP;
   HAL_GPIO_Init(GPIOI, &gpio_init_structure);

   /* LCD_BL_CTRL GPIO configuration */
   gpio_init_structure.Pin                               = GPIO_PIN_3;           /* LCD_BL_CTRL pin has to be manually controlled */
   gpio_init_structure.Mode                              = GPIO_MODE_OUTPUT_PP;
   HAL_GPIO_Init(GPIOK, &gpio_init_structure);

   /* Set LTDC Interrupt to the lowest priority */
   HAL_NVIC_SetPriority(LTDC_IRQn, 0xE, 0);

   /* Enable LTDC Interrupt */
   HAL_NVIC_EnableIRQ(LTDC_IRQn);
}

/* --------------------------------------------------------------------------
 * Name : HAL_LTDC_MspDeInit()
 *        
 *
 * -------------------------------------------------------------------------- */
void HAL_LTDC_MspDeInit(LTDC_HandleTypeDef* hltdc)
{
   /* Reset peripherals */
   /* Enable LTDC reset state */
   __HAL_RCC_LTDC_FORCE_RESET();

   /* Release LTDC from reset state */ 
   __HAL_RCC_LTDC_RELEASE_RESET();
}

/* --------------------------------------------------------------------------
 * Name : HAL_DMA2D_MspInit()
 *        
 *
 * -------------------------------------------------------------------------- */
void HAL_DMA2D_MspInit(DMA2D_HandleTypeDef* hdma2d)
{
   if(hdma2d->Instance==DMA2D)
   {
      /* Peripheral clock enable */
      __HAL_RCC_DMA2D_CLK_ENABLE();
   }
}

/* --------------------------------------------------------------------------
 * Name : HAL_DMA2D_MspDeInit()
 *        
 *
 * -------------------------------------------------------------------------- */
void HAL_DMA2D_MspDeInit(DMA2D_HandleTypeDef* hdma2d)
{
   if(hdma2d->Instance==DMA2D)
   {
      /* Peripheral clock disable */
      __HAL_RCC_DMA2D_CLK_DISABLE();
   }
}

/* --------------------------------------------------------------------------
 * Name : HAL_TIM_Base_MspInit()
 *        
 *
 * -------------------------------------------------------------------------- */
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_base)
{
   if (htim_base->Instance == TIM6)
   {
      /* Peripheral clock enable */
      __HAL_RCC_TIM6_CLK_ENABLE();
   }
   else if (htim_base->Instance == TIM1)
   {
      /* Peripheral clock enable */
      __HAL_RCC_TIM1_CLK_ENABLE();
   }
   else if(htim_base->Instance == TIM12)
   {
      /* Peripheral clock enable */
      __HAL_RCC_TIM12_CLK_ENABLE();
   }
}

/* --------------------------------------------------------------------------
 * Name : HAL_TIM_Base_MspDeInit()
 *        
 *
 * -------------------------------------------------------------------------- */
void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* htim_base)
{
   if (htim_base->Instance == TIM6)
   {
      /* Peripheral clock disable */
      __HAL_RCC_TIM6_CLK_DISABLE();
   }
}

/* --------------------------------------------------------------------------
 * Name : HAL_UART_MspInit()
 *        
 *
 * -------------------------------------------------------------------------- */
void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
#if defined(DEBUG_OUTPUT_UART)
   if (huart->Instance == USART6)
   {
      /* Peripheral clock enable */
      __HAL_RCC_USART6_CLK_ENABLE();
  }
#endif
}

/* --------------------------------------------------------------------------
 * Name : HAL_UART_MspDeInit()
 *        
 *
 * -------------------------------------------------------------------------- */
void HAL_UART_MspDeInit(UART_HandleTypeDef* huart)
{
#if defined(DEBUG_OUTPUT_UART)
   if (huart->Instance == USART6)
   {
      /* Peripheral clock disable */
      __HAL_RCC_USART6_CLK_DISABLE();
   }
#endif
}

/* --------------------------------------------------------------------------
 * Name : HAL_I2C_MspInit()
 *        
 *
 * -------------------------------------------------------------------------- */
void HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c)
{
   GPIO_InitTypeDef GPIO_InitStruct;
   if (hi2c->Instance == I2C3)
   {
      /**I2C3 GPIO Configuration    
      PH7     ------> I2C3_SCL
      PH8     ------> I2C3_SDA 
      */
      GPIO_InitStruct.Pin                                = GPIO_PIN_7 | GPIO_PIN_8;
      GPIO_InitStruct.Mode                               = GPIO_MODE_AF_OD;
      GPIO_InitStruct.Pull                               = GPIO_PULLUP;
      GPIO_InitStruct.Speed                              = GPIO_SPEED_FREQ_VERY_HIGH;
      GPIO_InitStruct.Alternate                          = GPIO_AF4_I2C3;
      HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

      /* Peripheral clock enable */
      __HAL_RCC_I2C3_CLK_ENABLE();
   }
}

/* --------------------------------------------------------------------------
 * Name : HAL_I2C_MspDeInit()
 *        
 *
 * -------------------------------------------------------------------------- */
void HAL_I2C_MspDeInit(I2C_HandleTypeDef* hi2c)
{
   if (hi2c->Instance == I2C3)
   {
      /* Peripheral clock disable */
      __HAL_RCC_I2C3_CLK_DISABLE();

      /**I2C3 GPIO Configuration    
      PH7     ------> I2C3_SCL
      PH8     ------> I2C3_SDA 
      */
      HAL_GPIO_DeInit(GPIOH, GPIO_PIN_7 | GPIO_PIN_8);
   }
}


