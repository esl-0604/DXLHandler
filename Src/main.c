/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#include "CrcCheck.h"
#include "Dynamixel.h"
#include "Dynamixel.h"
#include "DynamixelControl.h"
#include "DynamixelHandler.h"
#include "DynamixelPacket.h"
#include "DynamixelProtocol.h"
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
 TIM_HandleTypeDef htim11;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart1_rx;

/* USER CODE BEGIN PV */
uint32_t g_uiTim11Cnt = 0;

//Dynamixel
uint8_t g_ucSelectMotor = 0;
uint8_t g_ucSize = 25;
uint8_t g_pucRxBuffer[PACKET_SIZE] = {0, };

int8_t g_cMode = 3;

uint8_t g_ucMotorStateFlag = 0;

int32_t g_iParsingData =0;
int32_t g_iMotorTargetPos1 = -200;
int32_t g_iMotorTargetPos2 = 1400;
int32_t g_uiPosReadTick = 0;
int32_t g_uiPosReadDelay = 250;
int32_t g_iGoalVelUp = 150;
int32_t g_iGoalVelDown = -150;

bool g_bMotorBackFlag = false;
bool g_bPostionReadFlag = false;
bool g_bMotorCompFlag = false;


int32_t g_TargetVel = -50; 

int32_t g_iOffeset = 1800;

DynamixelHandler g_dynamixelHandler(&huart1);
Dynamixel g_mapDynamixel[2] = {Dynamixel(&g_dynamixelHandler,1),Dynamixel(&g_dynamixelHandler,6)};

//main Control Flag

int32_t g_uiDownPos = -220;

uint16_t g_usMainFlag = 21;

uint8_t MotorOpFlag = 0;
uint8_t MotorPreOpFlag = 0;
bool g_bWaitFlag = false;
bool g_bMotorInitFlag = false;
bool g_bMotorOPFlag = false;
bool g_bMotorStopFlag = false;

bool g_bMotorFlag1 = false;
bool g_bMotorFlag2 = false;

bool g_bResetFlag = false;
uint8_t g_ucMotorSpeedState =0;

uint32_t g_uiTick =0;
uint32_t g_uiTargetTick =100;
uint32_t g_uiSpeedTick =0;
uint32_t g_uiSpeedTargetTick =400;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM11_Init(void);
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
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_TIM11_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start_IT(&htim11);
  HAL_GPIO_WritePin(GPIOC,GPIO_PIN_10,GPIO_PIN_SET);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  RCC_OscInitStruct.PLL.PLLR = 2;
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
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM11 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM11_Init(void)
{

  /* USER CODE BEGIN TIM11_Init 0 */

  /* USER CODE END TIM11_Init 0 */

  /* USER CODE BEGIN TIM11_Init 1 */

  /* USER CODE END TIM11_Init 1 */
  htim11.Instance = TIM11;
  htim11.Init.Prescaler = 83;
  htim11.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim11.Init.Period = 99;
  htim11.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim11.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim11) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM11_Init 2 */

  /* USER CODE END TIM11_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 1000000;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */
//HAL_UARTEx_ReceiveToIdle_DMA(&huart1, g_pucRxBuffer, g_ucSize);
  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream2_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PB14 */
  GPIO_InitStruct.Pin = GPIO_PIN_14;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PC7 PC8 */
  GPIO_InitStruct.Pin = GPIO_PIN_7|GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PC10 */
  GPIO_InitStruct.Pin = GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
    if(htim->Instance == TIM11){
      g_uiTim11Cnt++;
    if(g_dynamixelHandler.bGetParsingFlag()){
      if(g_dynamixelHandler.bParsingFunction(g_mapDynamixel,g_ucSelectMotor)){
        g_dynamixelHandler.xSetParsingFlag(false);
      }
    }
      if(HAL_GetTick()-g_uiTick > g_uiTargetTick){        
        if(HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_8) == GPIO_PIN_RESET){
          if(g_bMotorFlag1 == false){
          g_bMotorOPFlag = true;
          }
        g_uiTick = HAL_GetTick();
        }
      if(HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_9) == GPIO_PIN_RESET){
        if(g_bMotorFlag2 == false){
          g_bMotorStopFlag = true;
        }
        g_uiTick = HAL_GetTick();
        }
      }
      if(g_bPostionReadFlag){
        if(HAL_GetTick() - g_uiPosReadTick > g_uiPosReadDelay){
          g_mapDynamixel[0].xTransmitDxlRPacket(PRESENT_POSITION,4);
          g_uiPosReadTick = HAL_GetTick();
        }
      }
    
      switch(g_usMainFlag){
      case 0:
//        if(HAL_GetTick()-g_uiSpeedTick > 500){
//          g_usMainFlag = 7;
//          g_uiSpeedTick = HAL_GetTick();
//        }
        break;
      case 21:
         if(g_bMotorStopFlag == true){
          g_mapDynamixel[0].xTransmitDxlWPacket(TORQUE_ADDR,0,1);
          g_bMotorFlag2 = false;
          g_bMotorStopFlag = false;
        }
        switch(g_ucMotorStateFlag){
        case 0:
//          g_mapDynamixel[0].xTransmitFacReset(0x02);
//          g_uiSpeedTick = HAL_GetTick();
          g_ucMotorStateFlag = 1;
          break;
        case 1:
////          HAL_Delay(5000);
//          if(HAL_GetTick()- g_uiSpeedTick >  5000000){
//            g_ucMotorStateFlag = 2;
//          }
        case 2:
          g_mapDynamixel[0].xInitDxl(4,1);
          HAL_Delay(2);                 
          g_mapDynamixel[0].xTransmitDxlWPacket(LED,1,1);
          HAL_Delay(2);
          HAL_GPIO_WritePin(GPIOC,GPIO_PIN_10,GPIO_PIN_RESET);
          g_bMotorInitFlag = true;
          g_bPostionReadFlag = true;
          g_ucMotorStateFlag = 3;
          break;
        case 3:
          if(g_bMotorOPFlag == true){
          g_iMotorTargetPos1 = -200; //g_mapDynamixel[0].iGetCurrentPosition()-g_iOffeset;
          g_mapDynamixel[0].xTransmitDxlWPacket(GOAL_POSITION,g_iMotorTargetPos1,4);
          g_bMotorOPFlag = false;
          g_bMotorBackFlag = true;
        }
          if(g_bMotorBackFlag == true && (abs(g_mapDynamixel[0].iGetCurrentPosition() - g_iMotorTargetPos1) <21) )
        {
//          g_iMotorTargetPos2 = 1400;//g_iMotorTargetPos1+g_iOffeset;
          g_mapDynamixel[0].xTransmitDxlWPacket(GOAL_POSITION,g_iMotorTargetPos2,4);
          g_bMotorBackFlag = false;
          g_bMotorCompFlag = true;
        }
        if(g_mapDynamixel[0].iGetCurrentPosition() == g_iMotorTargetPos2){
          g_bMotorCompFlag = false;
          g_bMotorFlag1 = false;
        }
        break;
        case 4:
          
          break;
        default:
          break;
        }
        break;
      case 1:
        g_mapDynamixel[g_ucSelectMotor].Dxl_Ping();   
        g_usMainFlag = 0;
        break;
      case 2:
//        HAL_UART_Transmit_IT(&huart1, l_ucTempPackets, 10);
//        g_usMainFlag = 0;
        break;
      case 3:
        break;
      case 4:
        HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_SET);
        g_mapDynamixel[g_ucSelectMotor].xTransmitDxlWPacket(LED,1,1);
        g_usMainFlag = 0;
        break;
      case 5:
        g_mapDynamixel[g_ucSelectMotor].xTransmitDxlWPacket(LED,0,1);
        g_usMainFlag = 0;
        break;
      case 6:
        if(g_bMotorInitFlag == false){
          g_mapDynamixel[0].xInitDxl(4,1);
          HAL_Delay(2);
          g_mapDynamixel[0].xTransmitDxlWPacket(POSITION_I_GAIN,10,4);
          HAL_Delay(2);
          g_mapDynamixel[0].xTransmitDxlWPacket(POSITION_D_GAIN,10,4);
          HAL_Delay(2);                 
          g_mapDynamixel[0].xTransmitDxlWPacket(LED,1,1);
          HAL_Delay(2);
          HAL_GPIO_WritePin(GPIOC,GPIO_PIN_10,GPIO_PIN_RESET);
          g_bMotorInitFlag = true;
          g_bPostionReadFlag = true;
        }
        if(g_bMotorOPFlag == true){
          g_iMotorTargetPos1 = g_uiDownPos; //g_mapDynamixel[0].iGetCurrentPosition()-g_iOffeset;
          g_mapDynamixel[0].xTransmitDxlWPacket(GOAL_POSITION,g_iMotorTargetPos1,4);
          g_bMotorOPFlag = false;
          g_bMotorBackFlag = true;
        }
        if(g_bMotorStopFlag == true){
          g_mapDynamixel[0].xTransmitDxlWPacket(TORQUE_ADDR,0,1);
          g_bMotorFlag2 = false;
          g_bMotorStopFlag = false;
        }
        
        if(g_bMotorBackFlag == true && ((g_mapDynamixel[0].iGetCurrentPosition() < -190) ))
        {
          g_iMotorTargetPos2 = 1400;//g_iMotorTargetPos1+g_iOffeset;
          g_mapDynamixel[0].xTransmitDxlWPacket(GOAL_POSITION,1600,4);
          g_bMotorBackFlag = false;
          g_bMotorCompFlag = true;
        }
        if(g_mapDynamixel[0].iGetCurrentPosition() == g_iMotorTargetPos2){
          g_bMotorCompFlag = false;
          g_bMotorFlag1 = false;
        }
          
        break;
      case 7:
        if(g_bResetFlag == false){
        HAL_Delay(30);
        g_mapDynamixel[0].xTransmitFacReset(0x02);
        HAL_Delay(5);
        g_bResetFlag = true;
        g_uiSpeedTick = HAL_GetTick();
        }
        if(g_bResetFlag == true &&HAL_GetTick() - g_uiSpeedTick > 5000){
          g_bResetFlag = false;
          g_usMainFlag =6;
          g_uiSpeedTick = HAL_GetTick();
        }
        break;
      case 8:
          g_mapDynamixel[0].xInitDxl(1,1);
          HAL_Delay(5);
          g_mapDynamixel[0].xTransmitDxlWPacket(LED,1,1);
          HAL_Delay(5);
          g_mapDynamixel[1].xInitDxl(1,1);
          HAL_Delay(5);
          g_mapDynamixel[1].xTransmitDxlWPacket(LED,1,1);
          g_usMainFlag =0;
          break;
      case 9:
        g_mapDynamixel[0].xTransmitDxlWPacket(LED,1,1);
        g_usMainFlag = 0;
        break;
      case 10:
        g_mapDynamixel[0].xTransmitDxlWPacket(LED,0,1);
        g_usMainFlag = 0;
        break;
      case 11:
        HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1,GPIO_PIN_SET);
        g_usMainFlag =0;
        break;
      case 12:
        HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1,GPIO_PIN_RESET);
        g_usMainFlag =0;
        break;
      case 13:
                g_mapDynamixel[0].xTransmitFacReset(0x02);
                g_usMainFlag = 0;
                break;
      case 14:
        g_mapDynamixel[0].Dxl_Ping();
        g_usMainFlag = 0;
      case 15:
//        g_mapDynamixel[0].xTransmitDxlWPacket(
        break;
      case 16:
        g_mapDynamixel[0].xTransmitDxlWPacket(GOAL_VELOCITY,g_TargetVel,4);
        g_usMainFlag = 0;
      case 17:
        g_mapDynamixel[0].xInitDxl(1,1);
        g_usMainFlag = 0;
      break;
      case 18:
         g_mapDynamixel[0].xInitDxl(4,1);
         g_usMainFlag = 0;
         break;
      case 19:
        g_mapDynamixel[0].xTransmitDxlWPacket(GOAL_POSITION,g_uiDownPos,4);
        g_usMainFlag = 0;
        break;
      case 20:
        g_mapDynamixel[0].xTransmitDxlWPacket(GOAL_POSITION,1600&0xFFFF,4);
        g_usMainFlag = 0;
        break;
    }
  }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){
  if(huart->Instance == USART1){
    g_mapDynamixel[g_ucSelectMotor].xSetTxPacketZero();
    g_dynamixelHandler.xSetTxFlag(true);
    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_RESET);
//    HAL_HalfDuplex_EnableReceiver(&huart1);
    HAL_UARTEx_ReceiveToIdle_DMA(&huart1,g_pucRxBuffer,PACKET_SIZE);
  }
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t ucSize) {
  g_ucSize = ucSize;
  if(huart->Instance == USART1) {
    g_dynamixelHandler.xSetRxTempPackets(g_pucRxBuffer,ucSize);
    g_dynamixelHandler.xSetParsingFlag(true);
//    HAL_UARTEx_ReceiveToIdle_DMA(huart, g_pucRxBuffer, ucSize);
  }
}
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

#ifdef  USE_FULL_ASSERT
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
