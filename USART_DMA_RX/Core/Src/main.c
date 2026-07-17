/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usart.h"
#include "string.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

volatile uint8_t rx_flag;

//For circular buffer
volatile uint8_t rx_buffer[RX_BUFFER_SIZE];
volatile uint16_t rx_head = 0;
volatile uint16_t rx_tail = 0;
volatile uint8_t rx_overflow = 0;

volatile uint8_t dma_tx_complete;
volatile uint8_t tx_complete;

uint8_t msg;

const char rsa_private_key[] =
"-----BEGIN PRIVATE KEY-----\r\n"
"MIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQCuiiYwoKuPHn1a\r\n"
"8H8B7JdCWU2ch66gx/xc/4O/ovB8ynyHhxyv8trh/spDI014149yfAROc7NUVNn8\r\n"
"w+XNm3rr+uGCLhbcybx8CN58MIjm6QCve8gwoFsJOmTQTRRjTUCMsrkl15xDvSE3\r\n"
"IxFawczer08S0KsQcuJ8iP8PepZNxQc/jFYTDEb6OgYYDNqHvdxy/jmm/tSwwWZv\r\n"
"aDGMpU0h+qqUeRj+4pLb6F7gQZ+Ku6kcSuROwQ4NDvqSSNhQi8u4aXyPEUwBxR9D\r\n"
"MszWJKbozfhdLKYF5a3RmCNsLIUUvTthATdoe183uVYi7KZKkby3N17apZkl8foW\r\n"
"Okyu8ZoXAgMBAAECggEADLFAWU7sMUpe6uSl4WAkGI+uV8PiJhA4J0cjFiXlZeqP\r\n"
"p1Qvxar1nqbM0v8Kz88IWaOJ6KHI5UjaWH0I0nlym4OxL6p/jR5UFfnLInrUQ2Ta\r\n"
"rbT+eAX60mJfTGqE7ge2U8T3ajh4q50us8TAEVQaItanlTWv17b5bP1e/VaSMYt6\r\n"
"Oc54IKQUANItufs3ic6vki0r0HZ5zbnSSObZeAxz3KtiIp6+xaxRinbK/TQCY2PZ\r\n"
"XMSRviHmzyOz/005qqKtbG70lSinE5yUMVHTR5EjBvdoBj/32mPOjGnYeAmcmNSC\r\n"
"BO3tVVy2ETyE2mdzBkl4sVuuI8eK/friHr//tbu7kQKBgQDfIkeHtm68BaH5FtPh\r\n"
"BzTcPsUlw5ODsTBof2BXJpcyWWCK9ypi8ILsH/TDxkOpQomNiA7N5t33Ucig3jn9\r\n"
"J4LibSoRlANxBIXAJCAIUdAoxP33ykJOlUAD08BAwbiv00WpffMKPm3k3WNCqa06\r\n"
"1nGOadggsW0kAxHeJ5hqrkm8eQKBgQDIP4YV3NiYik/0t24zaB1Sn2D9PWZGNke7\r\n"
"wLiaHeLr9UfXSn6VVNKf2UX7YeV9TYA5Y/rZSv8GN46XVj9Bh3L8knGx/LepJeWZ\r\n"
"3uc0NungngwGakjzlYk9pStNUenQKUSJdlZCj1kWSx0kmjlQ6Rp3cMA6Ls106i3w\r\n"
"NJt0hapHDwKBgEQ3cq7m80vAXRiOBhDR5zM+bX9yH3MvhKEt8AI05hyafo19qxO8\r\n"
"fAo8atQ/lQf2M591bTE917Z45mFoD0p5/eXDgg0Ft1eiDlIDuVdRy9XrA8nxp/vm\r\n"
"XT/OSQSDNTdr2+xJeTg0hdxmYqZKwY8vaGAls9Gq7LcJyTil2k8jIZ/hAoGBAKQa\r\n"
"Ar0p5cpkFpAaPlIccWNDP01oosbJNtHzZpTOByeNM5mH7+Jax9y9Rq9yk4mzGHBv\r\n"
"saOe5Q1oPA9jW6VM5ft723ab/rq8VsNZOym8Er/DZWFgenY7xmNhVvIfPbgcCZGJ\r\n"
"C7myFbTCS5iyVmQrE0hseutzbWXxf0IJKpg77cYbAoGAGJg3LHTjbaNxHeavqkD5\r\n"
"ytf5HMaoYJi2kSvNeqnO7mIwviImyzLs+vAi3prCiAZjGtE0vSyObkYzOnSojUWC\r\n"
"hJ/hJhaBkxD0lYjsoJum2BGjbV80J9o4imbp66xNTSzTrzjk3htlcPk8Np5H+8lO\r\n"
"s6tzPD/Y31JIjRml8RQPjGM=\r\n"
"-----END PRIVATE KEY-----\r\n";



char dmarx_buffer[16];
volatile uint8_t dma_rx_complete;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  /* USER CODE BEGIN 2 */

  USART2_Init();
  USART2_DMA_TX_Init();
  USART2_DMA_RX_Init();

  USART2_SendString("USART Initialized...!!\r\n");


  USART2_DMA_SendString("DMA Initialized for Transmit\r\n");

  // 500msec delay before next DMA operation
  HAL_Delay(500);

  USART2_DMA_SendBuffer(rsa_private_key,strlen(rsa_private_key));

  // 500msec delay before next DMA operation
  HAL_Delay(500);

  USART2_DMA_ReceiveBuffer(dmarx_buffer, 16);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

	  if(dma_rx_complete)
	  {
		  dma_rx_complete = 0;
		  USART2_DMA_SendBuffer(dmarx_buffer,16);
		  USART2_DMA_ReceiveBuffer(dmarx_buffer, 16);
	  }

	  /* if data is received */
	  if(rx_head != rx_tail)
	  {
	      msg = USART2_ReadByte();

	      tx_complete = 0;
	      dma_tx_complete = 0;
	      USART2_DMA_SendByte(msg);

		  if(rx_overflow == 1)
		  {
			  Error_Handler();
		  }
	  }

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 84;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
