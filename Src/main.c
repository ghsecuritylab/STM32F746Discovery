/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
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
#include <stdio.h>
#include "main.h"
#include "stm32f7xx_hal.h"
#include "debug_output.h"
#include "lcd_log.h"
#include "touch_ft5536.h"
#include "software_timer.h"
#include "cmsis_os.h"
#include "lwip.h"
#include "network_task.h"
#if defined(DEBUG_OUTPUT_USB)
#include "usb_device.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_cdc.h"
#include "usbd_cdc_if.h"
#endif

// -----------------------------------------------------------------------------
// global variable
TIM_HandleTypeDef htim6;
#if defined(DEBUG_OUTPUT_UART)
UART_HandleTypeDef huart6;
#endif
LTDC_HandleTypeDef hltdc;
DMA2D_HandleTypeDef hdma2d;
SDRAM_HandleTypeDef hsdram1;
DMA_HandleTypeDef hdma_memtomem_dma2_stream0;
TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim12;
#if defined(DEBUG_OUTPUT_USB)
/* USB Device Core handle declaration */
USBD_HandleTypeDef hUsbDeviceFS;
/* Semaphore to signal incoming packets */
osSemaphoreId usb_Cable_detect_Semaphore                 = NULL;
#endif
// -----------------------------------------------------------------------------

#define MILLI_SECOND                                     1000
volatile uint32_t g_System_Start_Second                  = 0;
volatile uint16_t g_System_Start_Milli_Second            = 0;

#define FRAMEBUFFER_SIZE                                 (RK043FN48H_WIDTH * RK043FN48H_HEIGHT)

volatile uint32_t *g_DMA2D_FrameBuffer                   = (uint32_t *) 0xC0700000;
volatile uint32_t *g_FrameBuffer                         = (uint32_t *) 0xC0780000;


#if (configAPPLICATION_ALLOCATED_HEAP == 1)
#if 0
uint8_t* ucHeap                                          = (uint8_t *) 0xC0000000;
#else
uint8_t ucHeap[configTOTAL_HEAP_SIZE]                    = {0, };
#endif
#endif

// 522,240, 0x7F800, x2 = 0xFF000, 1044,480
// volatile uint16_t g_FrameBuffer[FRAMEBUFFER_SIZE];


/* --------------------------------------------------------------------------
 * Name : _putc()
 *
 *
 * -------------------------------------------------------------------------- */
static uint8_t g_start_log                               = 0;
int _putc(unsigned char ch)
{
   if (g_start_log == 0)
   {
      return -1;
   }
   draw_char(ch);
#if defined(DEBUG_OUTPUT_UART)
   if (HAL_UART_Transmit(&huart6, (uint8_t *) &ch, sizeof(ch), 0xFFFF) == HAL_OK)
   {
      return 0;
   }
#endif

#if defined(DEBUG_OUTPUT_USB)
   CDC_Log_Write(ch);
#if 0
   while (CDC_Transmit_FS((uint8_t *) &ch, 1) != USBD_OK)
   {
      if (retry_count++ > 5)
      {
         break;
      }
   }
#endif
#endif

   return -1;
}

int _write(int file, char *ptr, int len)
{
	int DataIdx;

		for (DataIdx = 0; DataIdx < len; DataIdx++)
		{
		   _putc( *ptr++ );
		}
	return len;
}

/* --------------------------------------------------------------------------
 * Name : HAL_GPIO_EXTI_Callback()
 *
 *
 * -------------------------------------------------------------------------- */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
   if (GPIO_Pin == GPIO_PIN_13)
   {
      HAL_GPIO_TogglePin(GPIOI, GPIO_PIN_1);
   }

#if defined(DEBUG_OUTPUT_USB)
   if (GPIO_Pin == GPIO_PIN_12)
   {
      if (usb_Cable_detect_Semaphore != NULL)
      {
         osSemaphoreRelease(usb_Cable_detect_Semaphore);
      }
   }
#endif
}

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_LTDC_Init(void);
static void MX_DMA2D_Init(void);
static void MX_DMA_Init(void);
static void MX_FMC_Init(void);
#if defined(DEBUG_OUTPUT_UART)
static void MX_USART6_UART_Init(void);
#endif
static void MX_TIM6_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM12_Init(void);
#if defined(DEBUG_OUTPUT_USB)
static void MX_USB_DEVICE_Init(void);
#endif


#if 1
/* --------------------------------------------------------------------------
 * Name : test_servo_1_task()
 *
 *
 * -------------------------------------------------------------------------- */
void test_servo_1_task(void const * argument)
{
   TIM_HandleTypeDef* pTimer1                            = (TIM_HandleTypeDef*) argument;
   uint8_t step                                          = 0;

//   debug_output_info("start !!! \r\n");
   while (1)
   {
#if 0
//      HAL_GPIO_WritePin(GPIOG, GPIO_PIN_6, GPIO_PIN_SET);
      HAL_GPIO_TogglePin(GPIOG, GPIO_PIN_6);
      osDelay(100);
#else
      debug_output_info("idle \r\n");
      if (step == 0)
      {
         pTimer1->Instance->ARR                          = 20000;
         pTimer1->Instance->CCR1                         = 3000;
         step                                            = 180;
//         debug_output_info("180 \r\n");
      }
      else
      {
         pTimer1->Instance->ARR                          = 20000;
         pTimer1->Instance->CCR1                         = 1000;
         step                                            = 0;
//         debug_output_info("0 \r\n");
      }
      osDelay(10000);
#endif
   }
   vTaskDelete(NULL);
}

/* --------------------------------------------------------------------------
 * Name : test_servo_2_task()
 *
 *
 * -------------------------------------------------------------------------- */
void test_servo_2_task(void const * argument)
{
   TIM_HandleTypeDef* pTimer1                            = (TIM_HandleTypeDef*) argument;
   uint8_t step                                          = 0;

//   debug_output_info("start !!! \r\n");
   while (1)
   {
//    debug_output_info("idle \r\n");
#if 1
      if (step == 0)
      {
         pTimer1->Instance->ARR                             = 20000;
         pTimer1->Instance->CCR1                            = 3000;
         step                                               = 180;
//         debug_output_info("180 \r\n");
      }
      else
      {
         pTimer1->Instance->ARR                             = 20000;
         pTimer1->Instance->CCR1                            = 1000;
         step                                               = 0;
//         debug_output_info("0 \r\n");
      }
#endif
      osDelay(10000);
   }
   vTaskDelete(NULL);
}


#else
/* --------------------------------------------------------------------------
 * Name : test_servo_timer1()
 *
 *
 * -------------------------------------------------------------------------- */
void test_servo_timer1(void* pData)
{
   TIM_HandleTypeDef* pTimer1                            = (TIM_HandleTypeDef*) pData;
static uint8_t step                                      = 0;

   if (step == 0)
   {
      pTimer1->Instance->ARR                             = 20000;
      pTimer1->Instance->CCR1                            = 5000;
      step                                               = 180;
      debug_output_info("180 \r\n");
   }
   else
   {
      pTimer1->Instance->ARR                             = 20000;
      pTimer1->Instance->CCR1                            = 700;
      step                                               = 0;
      debug_output_info("0 \r\n");
   }
}

/* --------------------------------------------------------------------------
 * Name : test_servo_timer12()
 *
 *
 * -------------------------------------------------------------------------- */
void test_servo_timer12(void* pData)
{
   TIM_HandleTypeDef* pTimer1                            = (TIM_HandleTypeDef*) pData;
static uint8_t step                                      = 0;

   if (step == 0)
   {
      pTimer1->Instance->ARR                             = 20000;
      pTimer1->Instance->CCR1                            = 5000;
      step                                               = 180;
      debug_output_info("180 \r\n");
   }
   else
   {
      pTimer1->Instance->ARR                             = 20000;
      pTimer1->Instance->CCR1                            = 700;
      step                                               = 0;
      debug_output_info("0 \r\n");
   }
}
#endif

/* --------------------------------------------------------------------------
 * Name : CPU_CACHE_Enable()
 *        Configure the MPU attributes as Device for  Ethernet Descriptors in the SRAM1.
 *        The Base Address is 0x20010000 since this memory interface is the AXI.
 *        The Configured Region Size is 256B (size of Rx and Tx ETH descriptors) 
 *
 * -------------------------------------------------------------------------- */
static void MPU_Config(void)
{
   MPU_Region_InitTypeDef MPU_InitStruct;

   /* Disable the MPU */
   HAL_MPU_Disable();

   /* Configure the MPU attributes as Device for Ethernet Descriptors in the SRAM */
   MPU_InitStruct.Enable                                 = MPU_REGION_ENABLE;
   MPU_InitStruct.BaseAddress                            = 0x20010000;
//   MPU_InitStruct.BaseAddress                            = 0xC0010000;
   MPU_InitStruct.Size                                   = MPU_REGION_SIZE_256B;
   MPU_InitStruct.AccessPermission                       = MPU_REGION_FULL_ACCESS;
   MPU_InitStruct.IsBufferable                           = MPU_ACCESS_BUFFERABLE;
   MPU_InitStruct.IsCacheable                            = MPU_ACCESS_NOT_CACHEABLE;
   MPU_InitStruct.IsShareable                            = MPU_ACCESS_SHAREABLE;
   MPU_InitStruct.Number                                 = MPU_REGION_NUMBER0;
   MPU_InitStruct.TypeExtField                           = MPU_TEX_LEVEL0;
   MPU_InitStruct.SubRegionDisable                       = 0x00;
   MPU_InitStruct.DisableExec                            = MPU_INSTRUCTION_ACCESS_ENABLE;

   HAL_MPU_ConfigRegion(&MPU_InitStruct);

   /* Enable the MPU */
   HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

/* --------------------------------------------------------------------------
 * Name : CPU_CACHE_Enable()
 *        CPU L1-Cache enable.
 *
 * -------------------------------------------------------------------------- */
static void CPU_CACHE_Enable(void)
{
   /* Enable I-Cache */
   SCB_EnableICache();	

   /* Enable D-Cache */
   SCB_EnableDCache();
}

/* --------------------------------------------------------------------------
 * Name : main()
 *
 *
 * -------------------------------------------------------------------------- */
int main(void)
{
   MPU_Config();
   CPU_CACHE_Enable();

   HAL_Init();
   SystemClock_Config();

   MX_GPIO_Init();
   MX_DMA_Init();
   MX_FMC_Init();
#if defined(DEBUG_OUTPUT_UART)
   MX_USART6_UART_Init();
#endif

#if defined(DEBUG_OUTPUT_USB)
   MX_USB_DEVICE_Init();
#endif

   MX_TIM6_Init();
   MX_LTDC_Init();
   MX_DMA2D_Init();
   MX_TIM1_Init();
   MX_TIM12_Init();

   // init lcd log
   lcd_log_Init(g_FrameBuffer, RK043FN48H_WIDTH, RK043FN48H_HEIGHT, LTDC_PIXEL_FORMAT_ARGB8888);
   g_start_log                                           = 1;

#if 0
   // --------------------------------------------------------------------------
   // init software timer
   init_software_timer();
#endif

   // --------------------------------------------------------------------------
   debug_output_info("===================================================== \r\n");
   debug_output_info("BUILD   : %s %s \r\n", __DATE__, __TIME__);
   debug_output_info("VERSION : ver0.1.4 \r\n");
   debug_output_info("===================================================== \r\n");

   // init touch driver
   touch_init();

   // --------------------------------------------------------------------------
   HAL_GPIO_WritePin(GPIOI, GPIO_PIN_12, GPIO_PIN_SET);     // Display on
   HAL_GPIO_WritePin(GPIOK, GPIO_PIN_3, GPIO_PIN_SET);      // Backlight on

   // --------------------------------------------------------------------------
   HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
   HAL_TIM_PWM_Start(&htim12, TIM_CHANNEL_1);

   // --------------------------------------------------------------------------
   // Initialize for LwIP
   MX_LWIP_Init();

   // --------------------------------------------------------------------------
   /* Thread definition for tcp server */
   osThreadDef(tcp_server_task, network_tcp_server_task, osPriorityNormal, 0, configMINIMAL_STACK_SIZE * 2);
   if (osThreadCreate(osThread(tcp_server_task), (void *) NULL) == NULL)
   {
      debug_output_error("Can't create thread : network_tcp_server_task !!!");
   }

   // --------------------------------------------------------------------------
   /* Thread definition for communication of tcp */
   osThreadDef(tcp_task, network_tcp_task, osPriorityNormal, 0, configMINIMAL_STACK_SIZE * 2);
   if (osThreadCreate(osThread(tcp_task), (void *) NULL) == NULL)
   {
      debug_output_error("Can't create thread : tcp_task !!!");
   }

   // --------------------------------------------------------------------------
   /* Thread definition for udp server */
   osThreadDef(udp_server_task, network_udp_server_task, osPriorityNormal, 0, configMINIMAL_STACK_SIZE * 2);
   if (osThreadCreate(osThread(udp_server_task), (void *) NULL) == NULL)
   {
      debug_output_error("Can't create thread : network_udp_server_task !!!");
   }

#if defined(DEBUG_OUTPUT_USB)
   // --------------------------------------------------------------------------
   osSemaphoreDef(SEM);
   usb_Cable_detect_Semaphore                            = osSemaphoreCreate(osSemaphore(SEM), 1);

   /* Thread definition for usb cable detect */
   osThreadDef(usb_cable_detect_task, USBD_LL_Cable_Detect, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
   if (osThreadCreate(osThread(usb_cable_detect_task), (void *) &hUsbDeviceFS) == NULL)
   {
      debug_output_error("Can't create thread : USBD_LL_Cable_Detect !!!");
   }

   osThreadDef(cdc_log_write_task, CDC_Log_Write_Task, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
   if (osThreadCreate(osThread(cdc_log_write_task), (void *) &hUsbDeviceFS) == NULL)
   {
      debug_output_error("Can't create thread : USBD_LL_Cable_Detect !!!");
   }
#endif


#if 1
   // --------------------------------------------------------------------------
   /* Thread 1 definition */
   osThreadDef(servo_task_1, test_servo_1_task, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
   osThreadDef(servo_task_2, test_servo_2_task, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);

   /* Start thread 1 */
   if (osThreadCreate(osThread(servo_task_1), (void *) &htim1) == NULL)
   {
      debug_output_error("Can't create thread : test_servo_1_task !!!");
   }

   /* Start thread 2 */
   if (osThreadCreate(osThread(servo_task_2), (void *) &htim12) == NULL)
   {
      debug_output_error("Can't create thread : test_servo_2_task !!!");
   }
#endif

   // --------------------------------------------------------------------------
   /* Start scheduler */
   osKernelStart();

   for(;;);

}


/* --------------------------------------------------------------------------
 * Name : HAL_SYSTICK_Callback()
 *
 *
 * -------------------------------------------------------------------------- */
void HAL_SYSTICK_Callback(void)
{
   g_System_Start_Milli_Second++;

   if (g_System_Start_Milli_Second >= MILLI_SECOND)
   {
      g_System_Start_Second++;
      g_System_Start_Milli_Second                        = 0;
   }

   if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == 0)
   {
      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, 1);
   }
   else
   {
      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, 0);
   }
}


void SystemClock_Config(void)
{
   RCC_OscInitTypeDef RCC_OscInitStruct;
   RCC_ClkInitTypeDef RCC_ClkInitStruct;
#if defined(SYSTEM_CLOCK_200MHZ)
   RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;
#endif

   /**Configure the main internal regulator output voltage 
   */
   __HAL_RCC_PWR_CLK_ENABLE();
   __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

#if defined(SYSTEM_CLOCK_MAX_216MHZ)                                             // HCLK : 216 MHz
   /**Initializes the CPU, AHB and APB busses clocks 
   */
   RCC_OscInitStruct.OscillatorType                      = RCC_OSCILLATORTYPE_HSE;
   RCC_OscInitStruct.HSEState                            = RCC_HSE_ON;
   RCC_OscInitStruct.HSIState                            = RCC_HSI_OFF;
   RCC_OscInitStruct.PLL.PLLState                        = RCC_PLL_ON;
   RCC_OscInitStruct.PLL.PLLSource                       = RCC_PLLSOURCE_HSE;
   RCC_OscInitStruct.PLL.PLLM                            = 25;
   RCC_OscInitStruct.PLL.PLLN                            = 432;
   RCC_OscInitStruct.PLL.PLLP                            = RCC_PLLP_DIV2;
   RCC_OscInitStruct.PLL.PLLQ                            = 9;
   if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
   {
      _Error_Handler(__FILE__, __LINE__);
   }

   /**Activate the Over-Drive mode 
   */
   if (HAL_PWREx_EnableOverDrive() != HAL_OK)
   {
      _Error_Handler(__FILE__, __LINE__);
   }

   /**Initializes the CPU, AHB and APB busses clocks 
   */
   RCC_ClkInitStruct.ClockType                           = RCC_CLOCKTYPE_HCLK    |
                                                           RCC_CLOCKTYPE_SYSCLK  |
                                                           RCC_CLOCKTYPE_PCLK1   |
                                                           RCC_CLOCKTYPE_PCLK2;
   RCC_ClkInitStruct.SYSCLKSource                        = RCC_SYSCLKSOURCE_PLLCLK;
   RCC_ClkInitStruct.AHBCLKDivider                       = RCC_SYSCLK_DIV1;
   RCC_ClkInitStruct.APB1CLKDivider                      = RCC_HCLK_DIV4;
   RCC_ClkInitStruct.APB2CLKDivider                      = RCC_HCLK_DIV2;

   if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK)
   {
      _Error_Handler(__FILE__, __LINE__);
   }

#elif defined(SYSTEM_CLOCK_200MHZ)                                               // HCLK : 200 MHz
   /**Initializes the CPU, AHB and APB busses clocks 
   */
   RCC_OscInitStruct.OscillatorType                      = RCC_OSCILLATORTYPE_HSE;
   RCC_OscInitStruct.HSEState                            = RCC_HSE_ON;
   RCC_OscInitStruct.PLL.PLLState                        = RCC_PLL_ON;
   RCC_OscInitStruct.PLL.PLLSource                       = RCC_PLLSOURCE_HSE;
   RCC_OscInitStruct.PLL.PLLM                            = 25;
   RCC_OscInitStruct.PLL.PLLN                            = 400;
   RCC_OscInitStruct.PLL.PLLP                            = RCC_PLLP_DIV2;
   RCC_OscInitStruct.PLL.PLLQ                            = 2;
   if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
   {
      _Error_Handler(__FILE__, __LINE__);
   }

   /**Activate the Over-Drive mode 
   */
   if (HAL_PWREx_EnableOverDrive() != HAL_OK)
   {
      _Error_Handler(__FILE__, __LINE__);
   }

   /**Initializes the CPU, AHB and APB busses clocks 
   */
   RCC_ClkInitStruct.ClockType                           = RCC_CLOCKTYPE_HCLK    |
                                                           RCC_CLOCKTYPE_SYSCLK  |
                                                           RCC_CLOCKTYPE_PCLK1   |
                                                           RCC_CLOCKTYPE_PCLK2;
   RCC_ClkInitStruct.SYSCLKSource                        = RCC_SYSCLKSOURCE_PLLCLK;
   RCC_ClkInitStruct.AHBCLKDivider                       = RCC_SYSCLK_DIV1;
   RCC_ClkInitStruct.APB1CLKDivider                      = RCC_HCLK_DIV4;
   RCC_ClkInitStruct.APB2CLKDivider                      = RCC_HCLK_DIV2;

   if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_6) != HAL_OK)
   {
      _Error_Handler(__FILE__, __LINE__);
   }

   PeriphClkInitStruct.PeriphClockSelection              = RCC_PERIPHCLK_LTDC | RCC_PERIPHCLK_CLK48;
   PeriphClkInitStruct.PLLSAI.PLLSAIN                    = 192;
   PeriphClkInitStruct.PLLSAI.PLLSAIR                    = 5;
   PeriphClkInitStruct.PLLSAI.PLLSAIQ                    = 2;
   PeriphClkInitStruct.PLLSAI.PLLSAIP                    = RCC_PLLSAIP_DIV4;
   PeriphClkInitStruct.PLLSAIDivQ                        = 1;
   PeriphClkInitStruct.PLLSAIDivR                        = RCC_PLLSAIDIVR_4;
   PeriphClkInitStruct.Clk48ClockSelection               = RCC_CLK48SOURCE_PLLSAIP;
   if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
   {
   _Error_Handler(__FILE__, __LINE__);
   }
#else
#error "Not defined clock"
#endif

   /**Configure the Systick interrupt time 
   */
   HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000);

   /**Configure the Systick 
   */
   HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

   /* SysTick_IRQn interrupt configuration */
   HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}


/* --------------------------------------------------------------------------
 * Name : MX_LTDC_Init()
 *        LTDC init function
 *
 * -------------------------------------------------------------------------- */
static void MX_LTDC_Init(void)
{
   LTDC_LayerCfgTypeDef pLayerCfg;

   hltdc.Instance                                        = LTDC;
   hltdc.Init.HSPolarity                                 = LTDC_HSPOLARITY_AL;
   hltdc.Init.VSPolarity                                 = LTDC_VSPOLARITY_AL;
   hltdc.Init.DEPolarity                                 = LTDC_DEPOLARITY_AL;
   hltdc.Init.PCPolarity                                 = LTDC_PCPOLARITY_IPC;
   hltdc.Init.HorizontalSync                             = (RK043FN48H_HSYNC - 1);
   hltdc.Init.VerticalSync                               = (RK043FN48H_VSYNC - 1);
   hltdc.Init.AccumulatedHBP                             = (RK043FN48H_HSYNC + RK043FN48H_HBP - 1);;
   hltdc.Init.AccumulatedVBP                             = (RK043FN48H_VSYNC + RK043FN48H_VBP- 1);;
   hltdc.Init.AccumulatedActiveW                         = (RK043FN48H_WIDTH + RK043FN48H_HSYNC + RK043FN48H_HBP - 1);
   hltdc.Init.AccumulatedActiveH                         = (RK043FN48H_HEIGHT + RK043FN48H_VSYNC + RK043FN48H_VBP - 1);
   hltdc.Init.TotalWidth                                 = (RK043FN48H_WIDTH + RK043FN48H_HSYNC + RK043FN48H_HBP + RK043FN48H_HFP - 1);
   hltdc.Init.TotalHeigh                                 = (RK043FN48H_HEIGHT + RK043FN48H_VSYNC + RK043FN48H_VBP + RK043FN48H_VFP - 1);;
   hltdc.Init.Backcolor.Blue                             = 0;
   hltdc.Init.Backcolor.Green                            = 0;
   hltdc.Init.Backcolor.Red                              = 0;
   if (HAL_LTDC_Init(&hltdc) != HAL_OK)
   {
      _Error_Handler(__FILE__, __LINE__);
   }

   pLayerCfg.WindowX0                                    = 0;
   pLayerCfg.WindowX1                                    = (RK043FN48H_WIDTH - 1);
   pLayerCfg.WindowY0                                    = 0;
   pLayerCfg.WindowY1                                    = (RK043FN48H_HEIGHT - 1);
   pLayerCfg.PixelFormat                                 = LTDC_PIXEL_FORMAT_ARGB8888;
   pLayerCfg.Alpha                                       = 255;
   pLayerCfg.Alpha0                                      = 0;
   pLayerCfg.BlendingFactor1                             = LTDC_BLENDING_FACTOR1_CA;
   pLayerCfg.BlendingFactor2                             = LTDC_BLENDING_FACTOR2_CA;
   pLayerCfg.FBStartAdress                               = (uint32_t) g_FrameBuffer;
   pLayerCfg.ImageWidth                                  = RK043FN48H_WIDTH;
   pLayerCfg.ImageHeight                                 = RK043FN48H_HEIGHT;
   pLayerCfg.Backcolor.Blue                              = 0;
   pLayerCfg.Backcolor.Green                             = 0;
   pLayerCfg.Backcolor.Red                               = 0;
   if (HAL_LTDC_ConfigLayer(&hltdc, &pLayerCfg, 0) != HAL_OK)
   {
      _Error_Handler(__FILE__, __LINE__);
   }

#if 0
#define LTDC_PIXEL_FORMAT_ARGB8888                  ((uint32_t)0x00000000U)      /*!< ARGB8888 LTDC pixel format */
#define LTDC_PIXEL_FORMAT_RGB888                    ((uint32_t)0x00000001U)      /*!< RGB888 LTDC pixel format   */
#endif

}

/* --------------------------------------------------------------------------
 * Name : MX_DMA2D_Init()
 *
 *
 * -------------------------------------------------------------------------- */
static void MX_DMA_Init(void)
{
   /* DMA controller clock enable */
   __HAL_RCC_DMA2_CLK_ENABLE();

   /* Configure DMA request hdma_memtomem_dma2_stream0 on DMA2_Stream0 */
   hdma_memtomem_dma2_stream0.Instance                   = DMA2_Stream0;
   hdma_memtomem_dma2_stream0.Init.Channel               = DMA_CHANNEL_0;
   hdma_memtomem_dma2_stream0.Init.Direction             = DMA_MEMORY_TO_MEMORY;
   hdma_memtomem_dma2_stream0.Init.PeriphInc             = DMA_PINC_ENABLE;
   hdma_memtomem_dma2_stream0.Init.MemInc                = DMA_MINC_ENABLE;
   hdma_memtomem_dma2_stream0.Init.PeriphDataAlignment   = DMA_PDATAALIGN_BYTE;
   hdma_memtomem_dma2_stream0.Init.MemDataAlignment      = DMA_MDATAALIGN_BYTE;
   hdma_memtomem_dma2_stream0.Init.Mode                  = DMA_NORMAL;
   hdma_memtomem_dma2_stream0.Init.Priority              = DMA_PRIORITY_LOW;
   hdma_memtomem_dma2_stream0.Init.FIFOMode              = DMA_FIFOMODE_ENABLE;
   hdma_memtomem_dma2_stream0.Init.FIFOThreshold         = DMA_FIFO_THRESHOLD_FULL;
   hdma_memtomem_dma2_stream0.Init.MemBurst              = DMA_MBURST_SINGLE;
   hdma_memtomem_dma2_stream0.Init.PeriphBurst           = DMA_PBURST_SINGLE;
   if (HAL_DMA_Init(&hdma_memtomem_dma2_stream0) != HAL_OK)
   {
      _Error_Handler(__FILE__, __LINE__);
   }
}

/* --------------------------------------------------------------------------
 * Name : MX_DMA2D_Init()
 *
 *
 * -------------------------------------------------------------------------- */
static void MX_DMA2D_Init(void)
{
   hdma2d.Instance                                       = DMA2D;
   hdma2d.Init.Mode                                      = DMA2D_M2M;
   hdma2d.Init.ColorMode                                 = DMA2D_OUTPUT_ARGB8888;
   hdma2d.Init.OutputOffset                              = 0;
   hdma2d.LayerCfg[1].InputOffset                        = 0;
   hdma2d.LayerCfg[1].InputColorMode                     = DMA2D_INPUT_ARGB8888;
   hdma2d.LayerCfg[1].AlphaMode                          = DMA2D_NO_MODIF_ALPHA;
   hdma2d.LayerCfg[1].InputAlpha                         = 0;
   if (HAL_DMA2D_Init(&hdma2d) != HAL_OK)
   {
      _Error_Handler(__FILE__, __LINE__);
   }

   if (HAL_DMA2D_ConfigLayer(&hdma2d, 1) != HAL_OK)
   {
      _Error_Handler(__FILE__, __LINE__);
   }
}

/* --------------------------------------------------------------------------
 * Name : MX_DMA2D_LCD_Clear()
 *
 *
 * -------------------------------------------------------------------------- */
void MX_DMA2D_LCD_Clear(void)
{
   if (HAL_DMA2D_Start(&hdma2d, 0x00000000, (uint32_t) g_FrameBuffer, RK043FN48H_WIDTH, RK043FN48H_HEIGHT) == HAL_OK)
   {
      /* Polling For DMA transfer */  
      HAL_DMA2D_PollForTransfer(&hdma2d, 10);
   }
   else
   {
      debug_output_info("MX_DMA2D_LCD_Clear() failed !!! \r\n");
   }
}

/* --------------------------------------------------------------------------
 * Name : BSP_SDRAM_Initialization_sequence()
 *
 *
 * -------------------------------------------------------------------------- */
#define SDRAM_MODEREG_BURST_LENGTH_1                     ((uint16_t) 0x0000)
#define SDRAM_MODEREG_BURST_LENGTH_2                     ((uint16_t) 0x0001)
#define SDRAM_MODEREG_BURST_LENGTH_4                     ((uint16_t) 0x0002)
#define SDRAM_MODEREG_BURST_LENGTH_8                     ((uint16_t) 0x0004)
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL              ((uint16_t) 0x0000)
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED             ((uint16_t) 0x0008)
#define SDRAM_MODEREG_CAS_LATENCY_2                      ((uint16_t) 0x0020)
#define SDRAM_MODEREG_CAS_LATENCY_3                      ((uint16_t) 0x0030)
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD            ((uint16_t) 0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED         ((uint16_t) 0x0000) 
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE             ((uint16_t) 0x0200) 

#define SDRAM_TIMEOUT                                    ((uint32_t) 0xFFFF)

void BSP_SDRAM_Initialization_sequence(uint32_t RefreshCount)
{
   __IO uint32_t tmpmrd                                  = 0;
static FMC_SDRAM_CommandTypeDef Command;

   /* Step 1: Configure a clock configuration enable command */
   Command.CommandMode                                   = FMC_SDRAM_CMD_CLK_ENABLE;
   Command.CommandTarget                                 = FMC_SDRAM_CMD_TARGET_BANK1;
   Command.AutoRefreshNumber                             = 1;
   Command.ModeRegisterDefinition                        = 0;

   /* Send the command */
   HAL_SDRAM_SendCommand(&hsdram1, &Command, SDRAM_TIMEOUT);

   /* Step 2: Insert 100 us minimum delay */ 
   /* Inserted delay is equal to 1 ms due to systick time base unit (ms) */
   HAL_Delay(1);

   /* Step 3: Configure a PALL (precharge all) command */ 
   Command.CommandMode                                   = FMC_SDRAM_CMD_PALL;
   Command.CommandTarget                                 = FMC_SDRAM_CMD_TARGET_BANK1;
   Command.AutoRefreshNumber                             = 1;
   Command.ModeRegisterDefinition                        = 0;

   /* Send the command */
   HAL_SDRAM_SendCommand(&hsdram1, &Command, SDRAM_TIMEOUT);  

   /* Step 4: Configure an Auto Refresh command */ 
   Command.CommandMode                                   = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
   Command.CommandTarget                                 = FMC_SDRAM_CMD_TARGET_BANK1;
   Command.AutoRefreshNumber                             = 8;
   Command.ModeRegisterDefinition                        = 0;

   /* Send the command */
   HAL_SDRAM_SendCommand(&hsdram1, &Command, SDRAM_TIMEOUT);

   /* Step 5: Program the external memory mode register */
   tmpmrd                                                = (uint32_t) SDRAM_MODEREG_BURST_LENGTH_1          |  \
                                                                      SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL   |  \
                                                                      SDRAM_MODEREG_CAS_LATENCY_2           |  \
                                                                      SDRAM_MODEREG_OPERATING_MODE_STANDARD |  \
                                                                      SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;

   Command.CommandMode                                   = FMC_SDRAM_CMD_LOAD_MODE;
   Command.CommandTarget                                 = FMC_SDRAM_CMD_TARGET_BANK1;
   Command.AutoRefreshNumber                             = 1;
   Command.ModeRegisterDefinition                        = tmpmrd;

   /* Send the command */
   HAL_SDRAM_SendCommand(&hsdram1, &Command, SDRAM_TIMEOUT);

   /* Step 6: Set the refresh rate counter */
   /* Set the device refresh rate */
   HAL_SDRAM_ProgramRefreshRate(&hsdram1, RefreshCount); 
}

/* --------------------------------------------------------------------------
 * Name : MX_FMC_Init()
 *        FMC initialization function
 *
 * -------------------------------------------------------------------------- */
#define REFRESH_COUNT                                    ((uint32_t) 0x0603)     /* SDRAM refresh counter (100Mhz SD clock) */
static void MX_FMC_Init(void)
{
   FMC_SDRAM_TimingTypeDef SdramTiming;

   /** Perform the SDRAM1 memory initialization sequence
   */
   hsdram1.Instance                                      = FMC_SDRAM_DEVICE;
   /* hsdram1.Init */
   hsdram1.Init.SDBank                                   = FMC_SDRAM_BANK1;
   hsdram1.Init.ColumnBitsNumber                         = FMC_SDRAM_COLUMN_BITS_NUM_8;
   hsdram1.Init.RowBitsNumber                            = FMC_SDRAM_ROW_BITS_NUM_12;
   hsdram1.Init.MemoryDataWidth                          = FMC_SDRAM_MEM_BUS_WIDTH_16;
   hsdram1.Init.InternalBankNumber                       = FMC_SDRAM_INTERN_BANKS_NUM_4;
   hsdram1.Init.CASLatency                               = FMC_SDRAM_CAS_LATENCY_2;
   hsdram1.Init.WriteProtection                          = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
   hsdram1.Init.SDClockPeriod                            = FMC_SDRAM_CLOCK_PERIOD_2;
   hsdram1.Init.ReadBurst                                = FMC_SDRAM_RBURST_ENABLE;
   hsdram1.Init.ReadPipeDelay                            = FMC_SDRAM_RPIPE_DELAY_0;
   /* SdramTiming */
   SdramTiming.LoadToActiveDelay                         = 2;
   SdramTiming.ExitSelfRefreshDelay                      = 7;
   SdramTiming.SelfRefreshTime                           = 4;
   SdramTiming.RowCycleDelay                             = 7;
   SdramTiming.WriteRecoveryTime                         = 2;
   SdramTiming.RPDelay                                   = 2;
   SdramTiming.RCDDelay                                  = 2;

   if (HAL_SDRAM_Init(&hsdram1, &SdramTiming) != HAL_OK)
   {
      _Error_Handler(__FILE__, __LINE__);
   }

   BSP_SDRAM_Initialization_sequence(REFRESH_COUNT);
}


/* --------------------------------------------------------------------------
 * Name : MX_GPIO_Init()
 *
 *
 * -------------------------------------------------------------------------- */
/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
static void MX_GPIO_Init(void)
{
   GPIO_InitTypeDef GPIO_InitStruct;

   // GPIO Ports Clock Enable
   __HAL_RCC_GPIOA_CLK_ENABLE();
   __HAL_RCC_GPIOB_CLK_ENABLE();
   __HAL_RCC_GPIOC_CLK_ENABLE();
   __HAL_RCC_GPIOD_CLK_ENABLE();
   __HAL_RCC_GPIOE_CLK_ENABLE();
   __HAL_RCC_GPIOF_CLK_ENABLE();
   __HAL_RCC_GPIOG_CLK_ENABLE();
   __HAL_RCC_GPIOH_CLK_ENABLE();
   __HAL_RCC_GPIOI_CLK_ENABLE();
   __HAL_RCC_GPIOJ_CLK_ENABLE();
   __HAL_RCC_GPIOK_CLK_ENABLE();

   // Configure GPIO pin : PI1, D1
   GPIO_InitStruct.Pin                                   = GPIO_PIN_0;
   GPIO_InitStruct.Mode                                  = GPIO_MODE_OUTPUT_PP;
   GPIO_InitStruct.Pull                                  = GPIO_NOPULL;
   GPIO_InitStruct.Speed                                 = GPIO_SPEED_FREQ_LOW;
   HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

#if defined(DEBUG_OUTPUT_UART)
   /**USART6 GPIO Configuration    
   PC6     ------> USART6_TX
   PC7     ------> USART6_RX 
   */
   GPIO_InitStruct.Pin                                   = GPIO_PIN_6 | GPIO_PIN_7;
   GPIO_InitStruct.Mode                                  = GPIO_MODE_AF_PP;
   GPIO_InitStruct.Pull                                  = GPIO_PULLUP;
   GPIO_InitStruct.Speed                                 = GPIO_SPEED_FREQ_VERY_HIGH;
   GPIO_InitStruct.Alternate                             = GPIO_AF8_USART6;
   HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
#endif

   // Configure GPIO pin : PK3, backlight on
   GPIO_InitStruct.Pin                                   = GPIO_PIN_0;
   GPIO_InitStruct.Mode                                  = GPIO_MODE_OUTPUT_PP;
   GPIO_InitStruct.Pull                                  = GPIO_NOPULL;
   GPIO_InitStruct.Speed                                 = GPIO_SPEED_FREQ_LOW;
   HAL_GPIO_Init(GPIOK, &GPIO_InitStruct);

   // Configure GPIO pin : PI1, D1
   // Configure GPIO pin : PI12, Display on
   GPIO_InitStruct.Pin                                   = GPIO_PIN_1 | GPIO_PIN_12;
   GPIO_InitStruct.Mode                                  = GPIO_MODE_OUTPUT_PP;
   GPIO_InitStruct.Pull                                  = GPIO_NOPULL;
   GPIO_InitStruct.Speed                                 = GPIO_SPEED_FREQ_LOW;
   HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);

   // Configure GPIO pin : PI13 (Touch IC IRQ)
   GPIO_InitStruct.Pin                                   = GPIO_PIN_13;
   GPIO_InitStruct.Mode                                  = GPIO_MODE_IT_RISING;
   GPIO_InitStruct.Pull                                  = GPIO_NOPULL;
   GPIO_InitStruct.Speed                                 = GPIO_SPEED_FREQ_LOW;
   HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);

   // lcd & backlight off
   HAL_GPIO_WritePin(GPIOI, GPIO_PIN_12, GPIO_PIN_RESET);     // Display off
   HAL_GPIO_WritePin(GPIOK, GPIO_PIN_3, GPIO_PIN_RESET);      // Backlight off

   // EXTI interrupt init for PI13 (EXTI 13)
   HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
   HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

   // Configure GPIO pin : PG6 for test
   // Configure GPIO pin : Arduino CN4 - pin 3
   GPIO_InitStruct.Pin                                   = GPIO_PIN_6;
   GPIO_InitStruct.Mode                                  = GPIO_MODE_OUTPUT_PP;
   GPIO_InitStruct.Pull                                  = GPIO_NOPULL;
   GPIO_InitStruct.Speed                                 = GPIO_SPEED_FREQ_LOW;
   HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);


}

/* --------------------------------------------------------------------------
 * Name : MX_TIM6_Init()
 *
 *
 * -------------------------------------------------------------------------- */
/* TIM6 init function */
static void MX_TIM6_Init(void)
{
#if 0
   TIM_MasterConfigTypeDef sMasterConfig;

   htim6.Instance                                        = TIM6;
   htim6.Init.Prescaler                                  = 0;
   htim6.Init.CounterMode                                = TIM_COUNTERMODE_UP;
   htim6.Init.Period                                     = 0;
   htim6.Init.AutoReloadPreload                          = TIM_AUTORELOAD_PRELOAD_DISABLE;
   if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
   {
      _Error_Handler(__FILE__, __LINE__);
   }

   sMasterConfig.MasterOutputTrigger                     = TIM_TRGO_RESET;
   sMasterConfig.MasterSlaveMode                         = TIM_MASTERSLAVEMODE_DISABLE;
   if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
   {
      _Error_Handler(__FILE__, __LINE__);
   }
#endif
}

#if defined(DEBUG_OUTPUT_UART)
/* --------------------------------------------------------------------------
 * Name : MX_USART6_UART_Init()
 *
 *
 * -------------------------------------------------------------------------- */
static void MX_USART6_UART_Init(void)
{
   huart6.Instance                                       = USART6;
   huart6.Init.BaudRate                                  = 115200;
   huart6.Init.WordLength                                = UART_WORDLENGTH_8B;
   huart6.Init.StopBits                                  = UART_STOPBITS_1;
   huart6.Init.Parity                                    = UART_PARITY_NONE;
   huart6.Init.Mode                                      = UART_MODE_TX_RX;
   huart6.Init.HwFlowCtl                                 = UART_HWCONTROL_NONE;
   huart6.Init.OverSampling                              = UART_OVERSAMPLING_16;
   huart6.Init.OneBitSampling                            = UART_ONE_BIT_SAMPLE_DISABLE;
   huart6.AdvancedInit.AdvFeatureInit                    = UART_ADVFEATURE_NO_INIT;
   if (HAL_UART_Init(&huart6) != HAL_OK)
   {
      _Error_Handler(__FILE__, __LINE__);
   }
}
#endif

#if defined(DEBUG_OUTPUT_USB)
/* --------------------------------------------------------------------------
 * Name : MX_USB_DEVICE_Init()
 *
 *
 * -------------------------------------------------------------------------- */
static void MX_USB_DEVICE_Init(void)
{
   /* Init Device Library,Add Supported Class and Start the library*/
   USBD_Init(&hUsbDeviceFS, &FS_Desc, DEVICE_FS);
   USBD_RegisterClass(&hUsbDeviceFS, &USBD_CDC);
   USBD_CDC_RegisterInterface(&hUsbDeviceFS, &USBD_Interface_fops_FS);
#if 0
   USBD_Start(&hUsbDeviceFS);
#endif
}
#endif

/* --------------------------------------------------------------------------
 * Name : MX_TIM1_Init()
 *        TIM1 init function
 *
 * -------------------------------------------------------------------------- */
static void MX_TIM1_Init(void)
{
   TIM_ClockConfigTypeDef sClockSourceConfig;
   TIM_MasterConfigTypeDef sMasterConfig;
   TIM_OC_InitTypeDef sConfigOC;
   TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig;
   GPIO_InitTypeDef GPIO_InitStruct;

   htim1.Instance                                        = TIM1;
   // 1 us = 1 / (100 MHz / 100)
   htim1.Init.Prescaler                                  = 100;
   htim1.Init.CounterMode                                = TIM_COUNTERMODE_UP;
   // 20 ms
   htim1.Init.Period                                     = 20000;
   htim1.Init.ClockDivision                              = TIM_CLOCKDIVISION_DIV1;
   htim1.Init.RepetitionCounter                          = 0;
   htim1.Init.AutoReloadPreload                          = TIM_AUTORELOAD_PRELOAD_DISABLE;
   if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
   {
      _Error_Handler(__FILE__, __LINE__);
   }

   sClockSourceConfig.ClockSource                        = TIM_CLOCKSOURCE_INTERNAL;
   if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
   {
      _Error_Handler(__FILE__, __LINE__);
   }

   if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
   {
      _Error_Handler(__FILE__, __LINE__);
   }

   sMasterConfig.MasterOutputTrigger                     = TIM_TRGO_RESET;
   sMasterConfig.MasterOutputTrigger2                    = TIM_TRGO2_RESET;
   sMasterConfig.MasterSlaveMode                         = TIM_MASTERSLAVEMODE_DISABLE;
   if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
   {
      _Error_Handler(__FILE__, __LINE__);
   }

   sConfigOC.OCMode                                      = TIM_OCMODE_PWM1;
   sConfigOC.Pulse                                       = 700;
   sConfigOC.OCPolarity                                  = TIM_OCPOLARITY_HIGH;
   sConfigOC.OCNPolarity                                 = TIM_OCNPOLARITY_HIGH;
   sConfigOC.OCFastMode                                  = TIM_OCFAST_DISABLE;
   sConfigOC.OCIdleState                                 = TIM_OCIDLESTATE_RESET;
   sConfigOC.OCNIdleState                                = TIM_OCNIDLESTATE_RESET;
   if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
   {
      _Error_Handler(__FILE__, __LINE__);
   }

   sBreakDeadTimeConfig.OffStateRunMode                  = TIM_OSSR_DISABLE;
   sBreakDeadTimeConfig.OffStateIDLEMode                 = TIM_OSSI_DISABLE;
   sBreakDeadTimeConfig.LockLevel                        = TIM_LOCKLEVEL_OFF;
   sBreakDeadTimeConfig.DeadTime                         = 0;
   sBreakDeadTimeConfig.BreakState                       = TIM_BREAK_DISABLE;
   sBreakDeadTimeConfig.BreakPolarity                    = TIM_BREAKPOLARITY_HIGH;
   sBreakDeadTimeConfig.BreakFilter                      = 0;
   sBreakDeadTimeConfig.Break2State                      = TIM_BREAK2_DISABLE;
   sBreakDeadTimeConfig.Break2Polarity                   = TIM_BREAK2POLARITY_HIGH;
   sBreakDeadTimeConfig.Break2Filter                     = 0;
   sBreakDeadTimeConfig.AutomaticOutput                  = TIM_AUTOMATICOUTPUT_DISABLE;
   if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
   {
      _Error_Handler(__FILE__, __LINE__);
   }

   /**TIM1 GPIO Configuration    
   PA8     ------> TIM1_CH1 
   */
   GPIO_InitStruct.Pin                                   = GPIO_PIN_8;
   GPIO_InitStruct.Mode                                  = GPIO_MODE_AF_PP;
   GPIO_InitStruct.Pull                                  = GPIO_NOPULL;
   GPIO_InitStruct.Speed                                 = GPIO_SPEED_FREQ_LOW;
   GPIO_InitStruct.Alternate                             = GPIO_AF1_TIM1;
   HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

/* --------------------------------------------------------------------------
 * Name : TIM12 init function()
 *
 *
 * -------------------------------------------------------------------------- */
static void MX_TIM12_Init(void)
{
   TIM_ClockConfigTypeDef sClockSourceConfig;
   TIM_OC_InitTypeDef sConfigOC;
   GPIO_InitTypeDef GPIO_InitStruct;

   htim12.Instance                                       = TIM12;
   // 1 us = 1 / (54 MHz / 54)
   htim12.Init.Prescaler                                 = 54;
   htim12.Init.CounterMode                               = TIM_COUNTERMODE_UP;
   // 20 ms
   htim12.Init.Period                                    = 20000;
   htim12.Init.ClockDivision                             = TIM_CLOCKDIVISION_DIV1;
   htim12.Init.AutoReloadPreload                         = TIM_AUTORELOAD_PRELOAD_DISABLE;
   if (HAL_TIM_Base_Init(&htim12) != HAL_OK)
   {
      _Error_Handler(__FILE__, __LINE__);
   }

   sClockSourceConfig.ClockSource                        = TIM_CLOCKSOURCE_INTERNAL;
   if (HAL_TIM_ConfigClockSource(&htim12, &sClockSourceConfig) != HAL_OK)
   {
      _Error_Handler(__FILE__, __LINE__);
   }

   if (HAL_TIM_PWM_Init(&htim12) != HAL_OK)
   {
    _Error_Handler(__FILE__, __LINE__);
   }

   sConfigOC.OCMode                                      = TIM_OCMODE_PWM1;
   sConfigOC.Pulse                                       = 700;
   sConfigOC.OCPolarity                                  = TIM_OCPOLARITY_HIGH;
   sConfigOC.OCFastMode                                  = TIM_OCFAST_DISABLE;
   if (HAL_TIM_PWM_ConfigChannel(&htim12, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
   {
       _Error_Handler(__FILE__, __LINE__);
   }

   /**TIM12 GPIO Configuration    
   PH6     ------> TIM12_CH1 
   */
   GPIO_InitStruct.Pin                                   = GPIO_PIN_6;
   GPIO_InitStruct.Mode                                  = GPIO_MODE_AF_PP;
   GPIO_InitStruct.Pull                                  = GPIO_NOPULL;
   GPIO_InitStruct.Speed                                 = GPIO_SPEED_FREQ_LOW;
   GPIO_InitStruct.Alternate                             = GPIO_AF9_TIM12;
   HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);
}


/* --------------------------------------------------------------------------
 * Name : _Error_Handler()
 *
 *
 * -------------------------------------------------------------------------- */
/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void _Error_Handler(char* file, int line)
{
   debug_output_error("******************************************************** \r\n");
   debug_output_error("%s - %d \r\n", file, line);
   debug_output_error("******************************************************** \r\n");
   while(1) 
   {
   }
}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
   /* USER CODE BEGIN 6 */
   /* User can add his own implementation to report the file name and line number,
   ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
   /* USER CODE END 6 */

}

#endif

