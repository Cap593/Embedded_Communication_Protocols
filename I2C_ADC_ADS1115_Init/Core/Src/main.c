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
#include "stdio.h"
#include "string.h"
#include "spi.h"
#include "usart.h"
#include "mcp2515.h"
#include "max7219.h"
#include "can.h"
#include "w25q64fv.h"
#include "i2c.h"
#include "at24c256.h"
#include "ads1115.h"

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



extern const uint8_t font_8x8_digits[10][8];
extern const uint8_t font_8x8_alpha[26][8];
extern const uint8_t font_space[8];


volatile uint8_t mcp2515_node2_int_pending;
volatile uint8_t mcp2515_node1_int_pending;
volatile uint8_t can1_rx0_pending = 0;
uint8_t current_txctrl_addr;

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
	uint8_t dlc_len = 0x2;
	uint8_t msg[] = {0x11, 0x12};
	uint8_t msg2[] = {0x21, 0x22};

	uint16_t rx_id = 0;
	uint8_t rx_dlc_len = 0;
	uint8_t rx_msg[8] = {0,0,0,0,0,0,0,0};

	uint16_t rx_id2 = 0;
	uint8_t rx_dlc_len2 = 0;
	uint8_t rx_msg2[8] = {0,0,0,0,0,0,0,0};

	uint16_t rx_id3 = 0;
	uint8_t rx_dlc_len3 = 0;
	uint8_t rx_msg3[8] = {0,0,0,0,0,0,0,0};

	char uart_msg_tx[100];
	char uart_msg_rx[100];
	char uart_msg_rx2[100];
	char uart_msg_rx3[100];

	//Node 3
	uint8_t node3_msg[] = {0x31,0x32};
	//uint16_t node3_id;
	//uint8_t node3_dlc, node3_data[8];

	//EEPROM
	uint8_t read_data  = 0x00;
	char uart_msg[100];

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

  USART2_SendString("UART Initialized....!!\r\n");

  SPI2_GPIO_Init();
  SPI2_Init();

  SPI1_GPIO_Init();
  SPI1_Init();
  USART2_SendString("SPI Initialized.....!!\r\n");

  /*//Enable Exti interrupt in PA0 for node2
  MCP2515_INT_PA0_Init();

  //Enable Exti interrupt in PA1 for node1
  MCP2515_INT_PA1_Init();

  MCP2515_SelectNode(MCP2515_NODE_1);
  MCP2515_SelectSpiBus(MCP2515_SPI_BUS_2);
  SPI2_Test();

  MCP2515_SelectNode(MCP2515_NODE_2);
  MCP2515_SelectSpiBus(MCP2515_SPI_BUS_1);
  SPI1_Test();

  // Initialize MCP node1 and move to normal mode
  MCP2515_SelectNode(MCP2515_NODE_1);
  MCP2515_SelectSpiBus(MCP2515_SPI_BUS_2);
  MCP2515_Init();

  //Enter normal mode
  MCP2515_SetMode(MCP2515_MODE_NORMAL);

  //check and wait till normal mode
  while((MCP2515_ReadRegister(MCP2515_CANSTAT) & MCP2515_MODE_MASK) != MCP2515_MODE_NORMAL);

  uint8_t canstat = MCP2515_ReadRegister(MCP2515_CANSTAT);

  if((canstat & MCP2515_MODE_MASK) == MCP2515_MODE_NORMAL)
  {
      USART2_SendString("MCP2515 Node1 Normal mode OK\r\n");
  }
  else
  {
      USART2_SendString("MCP2515 Node1 Normal mode failed\r\n");
  }

  // Initialize MCP node2 and move to normal mode
  MCP2515_SelectNode(MCP2515_NODE_2);
  MCP2515_SelectSpiBus(MCP2515_SPI_BUS_1);
  MCP2515_Init();

  //Enter Normal mode
  MCP2515_SetMode(MCP2515_MODE_NORMAL);

  //check and wait till loopback mode
  while((MCP2515_ReadRegister(MCP2515_CANSTAT) & MCP2515_MODE_MASK) != MCP2515_MODE_NORMAL);

  uint8_t canstat2 = MCP2515_ReadRegister(MCP2515_CANSTAT);

  if((canstat2 & MCP2515_MODE_MASK) == MCP2515_MODE_NORMAL)
  {
      USART2_SendString("MCP2515 Node2 Normal mode OK\r\n");
  }
  else
  {
      USART2_SendString("MCP2515 Node2 Normal mode failed\r\n");
  }

  // Simple hardware test
  MAX7219_Write(0x0F, 0x01);   // All LEDs ON
  HAL_Delay(2000);
  MAX7219_Write(0x0F, 0x00);   // Test OFF

  MAX7219_Init();
  MAX7219_Clear();

  CAN1_GPIO_Init();
  if(CAN1_Init_Normal_500kbps())
  {
      USART2_SendString("CAN1 Normal Mode init OK\r\n");
  }
  else
  {
      USART2_SendString("CAN1 Normal Mode init failed\r\n");
  }

  CAN1_RX0_Interrupt_Enable();

  //flash self test
  if(W25Q_FlashSelfTest() != 0x00)
  {
	  USART2_SendString("External Flash Error\r\n");
  }
  else
  {
	  USART2_SendString("External Flash Initialized with no Error\r\n");
  }*/


  //I2C --> EEPROM
  I2C1_GPIO_Init();
  I2C1_Init_100kHz();

  USART2_SendString("I2C1 Initialized for AT24C256...!!!!!\r\n");

  //Test EEPROM
  AT24C256_TestAllApis();

  //Test ADC
  ADS1115_Init();
  ADS1115_TestBasic();


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  //One shot ADC conversion
	  	  ADS1115_Test_AIN0();
	  	  HAL_Delay(500);

	  	  //USART2_SendString("\r\n");
	  	  //USART2_SendString("/*********TEST : CAN message transmission and reception**********/\r\n");
	  	  /*USART2_SendString("\r\n");

	  	  //CAN Internal Node3 transmit message
	  	  if(CAN1_SendStdMessage(0x301,node3_msg,2) == E_OK)
	  	  {
	  	      USART2_SendString("CAN1 TX requested\r\n");
			  sprintf(uart_msg_tx,"bxCAN Message Transmitted : ID = 0x%03X, DLC = %u, DATA1 = 0x%02X, DATA2 = 0x%02X\r\n",0x301,2,node3_msg[0],node3_msg[1]);
			  USART2_SendString(uart_msg_tx);
	  	  }
	  	  else
	  	  {
	  	      USART2_SendString("CAN1 TX failed\r\n");
	  	  }

	  	//Node1 transmit message
		  MCP2515_SelectNode(MCP2515_NODE_1);
		  MCP2515_SelectSpiBus(MCP2515_SPI_BUS_2);

		  if(MCP2515_SendMessage(0x101,msg,dlc_len))
		  {
			  HAL_Delay(10);

			  if(MCP2515_PrintTxStatus(current_txctrl_addr))
			  {
				  USART2_SendString("Node1 Message Transmitted Successfully\r\n");
			  }
			  else
			  {
				  USART2_SendString("Node1 Tx Error\r\n");
			  }

			  sprintf(uart_msg_tx,"Node1 Message Transmitted : ID = 0x%03X, DLC = 0x%02X, DATA1 = 0x%02X, DATA2 = 0x%02X\r\n",0x101,dlc_len,msg[0],msg[1]);
			  USART2_SendString(uart_msg_tx);
		  }
		  else
		  {
			  USART2_SendString("Node1 : Message Tx Failed\r\n");
		  }

		  //Node2 transmit message
		  MCP2515_SelectNode(MCP2515_NODE_2);
		  MCP2515_SelectSpiBus(MCP2515_SPI_BUS_1);

		  if(MCP2515_SendMessage(0x201,msg2,dlc_len))
		  {
			  HAL_Delay(10);

			  if(MCP2515_PrintTxStatus(current_txctrl_addr))
			  {
				  USART2_SendString("Node2 Message Transmitted Successfully\r\n");
			  }
			  else
			  {
				  USART2_SendString("Node2 Tx Error\r\n");
			  }
			  sprintf(uart_msg_tx,"Node2 Message Transmitted : ID = 0x%03X, DLC = 0x%02X, DATA1 = 0x%02X, DATA2 = 0x%02X\r\n",0x201,dlc_len,msg2[0],msg2[1]);
			  USART2_SendString(uart_msg_tx);
		  }
		  else
		  {
			  USART2_SendString("Node2 : Message Tx Failed\r\n");
		  }

	  	  HAL_Delay(10);

	  	  if(mcp2515_node1_int_pending)
	  	  {
	  		  mcp2515_node1_int_pending = 0;

	  		  MCP2515_SelectNode(MCP2515_NODE_1);
	  		  MCP2515_SelectSpiBus(MCP2515_SPI_BUS_2);

	  		  //Receiving frame
	  		  while(MCP2515_ReadMessage(&rx_id,rx_msg,&rx_dlc_len))
	  		  {
	  			  sprintf(uart_msg_rx,"Node1 Message Received    : ID = 0x%03X, DLC = 0x%02X, DATA1 = 0x%02X, DATA2 = 0x%02X\r\n",rx_id,rx_dlc_len,rx_msg[0],rx_msg[1]);
	  			  USART2_SendString(uart_msg_rx);
	  		  }
	  	  }

	  	  if(mcp2515_node2_int_pending)
	  	  {
	  		  mcp2515_node2_int_pending = 0;

	  		 //Node 2 Receives
	  		  MCP2515_SelectNode(MCP2515_NODE_2);
	  		  MCP2515_SelectSpiBus(MCP2515_SPI_BUS_1);

	  		  //Receiving frame
	  		  while(MCP2515_ReadMessage(&rx_id2,rx_msg2,&rx_dlc_len2))
	  		  {
	  			  sprintf(uart_msg_rx2,"Node2 Message Received    : ID = 0x%03X, DLC = 0x%02X, DATA1 = 0x%02X, DATA2 = 0x%02X\r\n",rx_id2,rx_dlc_len2,rx_msg2[0],rx_msg2[1]);
	  			  USART2_SendString(uart_msg_rx2);
	  		  }
	  	  }

	  	  if(can1_rx0_pending)
	  	  {
	  		can1_rx0_pending = 0;

			  while(CAN1_ReadMessage_FIFO0(&rx_id3, rx_msg3, &rx_dlc_len3))
			  {
				  sprintf(uart_msg_rx3,"bxCAN Message Received    : ID = 0x%03X, DLC = 0x%02X, DATA1 = 0x%02X, DATA2 = 0x%02X\r\n",rx_id3,rx_dlc_len3,rx_msg3[0],rx_msg3[1]);
				  USART2_SendString(uart_msg_rx3);
			  }

			  //Enable interrupt for next msg reception
			  CAN1->IER |= (CAN_IER_FMPIE0);

	  	  }

	  MAX7219_CharBlinking('A');*/

	  //MAX7219_StringBlinking("Hello Via Matrix");

	  //MAX7219_ScrollText_RtoL("HELLO", 100);

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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 64;
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
