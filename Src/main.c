/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "usart.h"
#include "gpio.h"
#include "stdio.h"

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);


/* Function processing DMA Rx data. Counts how many capital and small letters are in sentence.
 * Result is supposed to be stored in global variable of type "letter_count_" that is defined in "main.h"
 *
 * @param1 - received sign
 */
uint8_t length(uint8_t *str);
void sendInfo(uint16_t bufferSize, uint16_t dataLength);
void processDmaData(uint8_t *sign, uint16_t len);

/* Space for your global variables. */

	// type your global variables here:
letter_count_ g_letter;
//uint8_t tx_data[] = "Data to send over UART DMA!\n\r";


int main(void)
{
  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
  NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

  /* Configure the system clock */
  SystemClock_Config();
  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART2_UART_Init();
  USART2_RegisterCallback(processDmaData);

  /* Space for your local variables, callback registration ...*/

  	  //type your code here:

  while (1)
  {
	  /* Periodic transmission of information about DMA Rx buffer state.
	   * Transmission frequency - 5Hz.
	   * Message format - "Buffer capacity: %d bytes, occupied memory: %d bytes, load [in %]: %f%"
	   * Example message (what I wish to see in terminal) - Buffer capacity: 1000 bytes, occupied memory: 231 bytes, load [in %]: 23.1%
	   */

  	  	  	  //type your code here:
#if POLLING
	//Polling for new data, no interrupts
	USART2_CheckDmaReception();
	LL_mDelay(10);
#else
	  sendInfo(DMA_USART2_BUFFER_SIZE, LL_DMA_GetDataLength(DMA1, LL_DMA_CHANNEL_6));
	  LL_mDelay(200);
#endif
  }
  /* USER CODE END 3 */
}


void SystemClock_Config(void)
{
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_0);

  if(LL_FLASH_GetLatency() != LL_FLASH_LATENCY_0)
  {
  Error_Handler();  
  }
  LL_RCC_HSI_Enable();

   /* Wait till HSI is ready */
  while(LL_RCC_HSI_IsReady() != 1)
  {
    
  }
  LL_RCC_HSI_SetCalibTrimming(16);
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSI);

   /* Wait till System clock is ready */
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSI)
  {
  
  }
  LL_Init1msTick(8000000);
  LL_SYSTICK_SetClkSource(LL_SYSTICK_CLKSOURCE_HCLK);
  LL_SetSystemCoreClock(8000000);
}

/*
 * Implementation of function processing data received via USART.
 */

uint8_t length(uint8_t *str) {
	uint8_t len = 0;

	while (str[len] != '\0') {
		len++;
	}

	return len;
}

void sendInfo(uint16_t bufferSize, uint16_t dataLength) {
	uint8_t data[30];	// Vacsi retazec ako 30 znakov by nikdy nemal nastat.
	uint16_t occupiedMemory = bufferSize - dataLength;

	sprintf((char *)data, "Buffer capacity: %d bytes, ", bufferSize);
	USART2_PutBuffer(data, length(data)*sizeof(uint8_t));		// sizeof(uint8_t) == 1 -> nasobenie len pre uplnost, keby ma retazec iny typ
	while(LL_DMA_IsEnabledChannel(DMA1, LL_DMA_CHANNEL_7));

	sprintf((char *)data, "occupied memory: %d bytes, ", occupiedMemory);
	USART2_PutBuffer(data, length(data)*sizeof(uint8_t));
	while(LL_DMA_IsEnabledChannel(DMA1, LL_DMA_CHANNEL_7));

	sprintf((char *)data, "load [in %%]: %3.2f %%\n\r", (float) occupiedMemory/bufferSize*100);
	USART2_PutBuffer(data, length(data)*sizeof(uint8_t));
	while(LL_DMA_IsEnabledChannel(DMA1, LL_DMA_CHANNEL_7));
}

void processDmaData(uint8_t *sign, uint16_t len) {
	/* Process received data */
		// type your algorithm here:
	static uint8_t s_count = 0, s_start = 0, s_capital_letter = 0, s_small_letter = 0;

	for (uint16_t n = 0; n < len; n++) {
		// Pocitadlo sa resetuje zakazdym, ked prijime startovaci znak. (Nebolo specifikovane v zadani, co sa ma robit pri viacerych startovacich znakoch, tak sa zvolil tento sposob.)
		if (sign[n] == '#') {
			s_capital_letter = 0;
			s_small_letter = 0;
			s_count = 0;
			s_start = 1;
		}

		if (s_start) {
			if (sign[n] >= 'A' && sign[n] <= 'Z') {
				s_capital_letter++;
			}
			else if (sign[n] >= 'a' && sign[n] <= 'z') {
				s_small_letter++;
			}

			// Ked pride ukoncovaci znak, tak zapise spracovany subor znakov.
			// Premenne sa prepisuju, vzdy pre novy subor znakov. (Nebolo specifikovane v zadani, ci sa ma priratavat, tak sa zvolil tento sposob.)
			if (sign[n] == '$') {
				g_letter.capital_letter = s_capital_letter;
				g_letter.small_letter = s_small_letter;
				s_start = 0;
			}
			else if (++s_count > 35) {
				s_start = 0;
			}
		}
	}
}

void Error_Handler(void)
{

}

#ifdef  USE_FULL_ASSERT

void assert_failed(char *file, uint32_t line)
{ 

}

#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
