/*
 * TaskHandler.cpp
 *
 *  Created on: Mar 13, 2024
 *      Author: eslee
 */

// * 모터의 변위를 연속적으로 제어하는 경우, State를 분할하여 직관적으로 표현하도록 한다.
// * USER CODE BEGIN / USER CODE END Interface를 최대한 활용하도록 한다.


#include "TaskHandler.h"



TaskHandler::TaskHandler(UART_HandleTypeDef* huart) : _DXLHandler(huart, DXL_CNT), _PSensorHandler(PSENSOR_CNT){
	this->_ucMainState = MAIN_OFF;
	this->_pucDXLIDList = new uint8_t[DXL_CNT];
	this->_pucPSencorIDList = new uint8_t[PSENSOR_CNT];
	this->_ucSystemFlag = FLAG_OFF;
	this->_ucHomingFlag = FLAG_OFF;
	this->_ucOperationFlag = FLAG_OFF;

	/* USER CODE BEGIN DXL_ID List */
	_pucDXLIDList[0] = DXL_1;
	_pucDXLIDList[1] = DXL_2;
	_pucDXLIDList[2] = DXL_3;
	_pucDXLIDList[3] = DXL_4;
	/* USER CODE END DXL_ID List */

	/* USER CODE BEGIN PSensor_ID List */
	_pucPSencorIDList[0] = PSENSOR_1;
	_pucPSencorIDList[1] = PSENSOR_2;
	/* USER CODE END PSensor_ID List */
}

TaskHandler::~TaskHandler(){

}


// Tx/Rx Interface --------------------------------------------------------------------------------------
void TaskHandler::TransmitCplt(){
	this->_DXLHandler.TransmitPacketCplt();
}

void TaskHandler::RecieveCplt(uint16_t usPacketSize){
	this->_DXLHandler.RecievePacketCplt(usPacketSize);
}


// Child Handler Interface -------------------------------------------------------------------------------
void TaskHandler::DXLInit(){
	this->_DXLHandler.SetDXLMapInit(this->_pucDXLIDList);
}

void TaskHandler::DXLClear(){
	this->_DXLHandler.SetDXLMapClear();
}

void TaskHandler::PSensorInit(){
	this->_PSensorHandler.SetPSensorMapInit(this->_pucPSencorIDList);
}

void TaskHandler::PSensorClear(){
	this->_PSensorHandler.SetPSensorMapClear();
}


// Main Process Interface ----------------------------------------------------------------------------------
void TaskHandler::MainProcess(volatile uint32_t* pnPowerPWM, volatile uint32_t* pnOperPWM){
	switch(this->_ucMainState){
		/* Main Off -----------------------------------------------*/
		case MAIN_OFF:
			if(this->_ucSystemFlag == SYSTEM_START_FLAG){
				this->DXLInit();
				this->PSensorInit();
				this->_ucMainState = MAIN_INIT;
			}
			break;


		/* Main Init ----------------------------------------------*/
		case MAIN_INIT:

			/* USER CODE BEGIN Init */


			// ERROM Read
			this->_DXLHandler.SyncPing();
			this->_DXLHandler.SyncReadOperatingMode(DXL_CNT, this->_pucDXLIDList);
			this->_DXLHandler.SyncReadHomingOffset(DXL_CNT, this->_pucDXLIDList);
			this->_DXLHandler.SyncReadCurrentLimit(DXL_CNT, this->_pucDXLIDList);
			this->_DXLHandler.SyncReadVelocityLimit(DXL_CNT, this->_pucDXLIDList);
			this->_DXLHandler.SyncReadMaxPositionLimit(DXL_CNT, this->_pucDXLIDList);
			this->_DXLHandler.SyncReadMinPositionLimit(DXL_CNT, this->_pucDXLIDList);

			// RAM Read
			this->_DXLHandler.SyncReadTorqueEnable(DXL_CNT, this->_pucDXLIDList);
			this->_DXLHandler.SyncReadLED(DXL_CNT, this->_pucDXLIDList);
			this->_DXLHandler.SyncReadStatusReturnLevel(DXL_CNT, this->_pucDXLIDList);
			this->_DXLHandler.SyncReadGoalPosition(DXL_CNT, this->_pucDXLIDList);
			this->_DXLHandler.SyncReadMoving(DXL_CNT, this->_pucDXLIDList);
			this->_DXLHandler.SyncReadPresentCurrent(DXL_CNT, this->_pucDXLIDList);
			this->_DXLHandler.SyncReadPresentVelocity(DXL_CNT, this->_pucDXLIDList);
			this->_DXLHandler.SyncReadPresentPosition(DXL_CNT, this->_pucDXLIDList);

			// Write
			static int32_t pnInitParams[8] = {1, 1, 2, 1, 3, 1, 4, 1};
			this->_DXLHandler.SyncWriteLED(DXL_CNT, pnInitParams);
			this->_DXLHandler.SyncWriteTorqueEnable(DXL_CNT, pnInitParams);


			/* USER CODE END Init */

			if(this->_DXLHandler.GetDxlState() == DXL_IDLE){
				this->_ucMainState = MAIN_IDLE;
			}
			else{
				this->_ucMainState = MAIN_INIT_ERR;
			}
			break;


		/* Main Idle ----------------------------------------------*/
		case MAIN_IDLE:

			/* USER CODE BEGIN Idle */


			// Operation 종료 시, Flag 전부 초기화
			if(this->_ucOperationFlag == OPERATION_END_FLAG){
				this->_ucHomingFlag = FLAG_OFF;
				this->_ucOperationFlag = FLAG_OFF;
			}

			// Homing Switch 클릭 시 --> Homing
			if(this->_ucHomingFlag == HOMING_START_FLAG){
				this->_ucMainState = MAIN_HOMING;
			}

			// Operating Switch 클릭 시 (Homing이 되어있다면) --> Operation / (Homing이 되어있지 않다면) --> Operation Flag 취소
			if(this->_ucOperationFlag == OPERATION_START_FLAG){
				if(this->_ucHomingFlag == HOMING_END_FLAG){
					this->_ucMainState = MAIN_OPERATION;
				}
				else{
					this->_ucOperationFlag = FLAG_OFF;
				}
			}


			/* USER CODE END Idle */

			break;


		/* Main Homing --------------------------------------------*/
		case MAIN_HOMING:

			/* USER CODE BEGIN Homing */


			// PSensor 1 Homing --> DXL_1
			switch(this->_PSensorHandler.GetPSensorStatusHomingState(PSENSOR_1)){
				case HOMING_START:
					if(this->_PSensorHandler.GetPSensorStatusSensorFlag(PSENSOR_1) == SENSOR_DETECTED){
						this->_DXLHandler.WriteGoalVelocity(DXL_1, HOMING_FAST_FAR_SPEED);
						this->_PSensorHandler.SetPSensorStatusHomingState(PSENSOR_1, HOMING_FAST_FAR);
					}
					else{
						this->_DXLHandler.WriteGoalVelocity(DXL_1, HOMING_FAST_CLOSE_SPEED);
						this->_PSensorHandler.SetPSensorStatusHomingState(PSENSOR_1, HOMING_FAST_CLOSE);
					}
					break;

				case HOMING_FAST_FAR:
					if(_PSensorHandler.GetPSensorStatusSensorFlag(PSENSOR_1) == SENSOR_NOT_DETECTED){
						this->_DXLHandler.WriteGoalVelocity(DXL_1, HOMING_SLOW_CLOSE_SPEED);
						this->_PSensorHandler.SetPSensorStatusHomingState(PSENSOR_1, HOMING_SLOW_CLOSE);
					}
					break;

				case HOMING_FAST_CLOSE:
					if(_PSensorHandler.GetPSensorStatusSensorFlag(PSENSOR_1) == SENSOR_DETECTED){
						this->_DXLHandler.WriteGoalVelocity(DXL_1, HOMING_SLOW_FAR_SPEED);
						this->_PSensorHandler.SetPSensorStatusHomingState(PSENSOR_1, HOMING_SLOW_FAR);
					}
					break;

				case HOMING_SLOW_FAR:
					if(_PSensorHandler.GetPSensorStatusSensorFlag(PSENSOR_1) == SENSOR_NOT_DETECTED){
						this->_DXLHandler.WriteGoalVelocity(DXL_1, HOMING_SLOW_CLOSE_SPEED);
						this->_PSensorHandler.SetPSensorStatusHomingState(PSENSOR_1, HOMING_SLOW_CLOSE);
					}
					break;

				case HOMING_SLOW_CLOSE:
					if(_PSensorHandler.GetPSensorStatusSensorFlag(PSENSOR_1) == SENSOR_DETECTED){
						this->_DXLHandler.WriteGoalVelocity(DXL_1, DXL_STOP);
						this->_PSensorHandler.SetPSensorStatusHomingState(PSENSOR_1, HOMING_OFFSET_SET);
					}
					break;

				case HOMING_OFFSET_SET:
					this->_DXLHandler.ReadPresentPosition(DXL_1);
					static int32_t nHomingOffsetValue = this->_DXLHandler.GetDXLStatusPresentPosition(DXL_1);
					this->_DXLHandler.WriteHomingOffset(DXL_1, nHomingOffsetValue);
					this->_DXLHandler.ReadPresentPosition(DXL_1);
					static int32_t nPresentPosition = this->_DXLHandler.GetDXLStatusPresentPosition(DXL_1);
					if(nPresentPosition == 0){
						this->_PSensorHandler.SetPSensorStatusHomingState(PSENSOR_1, HOMING_CPLT);
					}
					break;

				case HOMING_CPLT:
					break;
			}

			// PSensor 2 Homing --> DXL_3
			switch(this->_PSensorHandler.GetPSensorStatusHomingState(PSENSOR_2)){
				case HOMING_START:
					if(this->_PSensorHandler.GetPSensorStatusSensorFlag(PSENSOR_2) == SENSOR_DETECTED){
						this->_DXLHandler.WriteGoalVelocity(DXL_3, HOMING_FAST_FAR_SPEED);
						this->_PSensorHandler.SetPSensorStatusHomingState(PSENSOR_2, HOMING_FAST_FAR);
					}
					else{
						this->_DXLHandler.WriteGoalVelocity(DXL_3, HOMING_FAST_CLOSE_SPEED);
						this->_PSensorHandler.SetPSensorStatusHomingState(PSENSOR_2, HOMING_FAST_CLOSE);
					}
					break;

				case HOMING_FAST_FAR:
					if(_PSensorHandler.GetPSensorStatusSensorFlag(PSENSOR_2) == SENSOR_NOT_DETECTED){
						this->_DXLHandler.WriteGoalVelocity(DXL_3, HOMING_SLOW_CLOSE_SPEED);
						this->_PSensorHandler.SetPSensorStatusHomingState(PSENSOR_2, HOMING_SLOW_CLOSE);
					}
					break;

				case HOMING_FAST_CLOSE:
					if(_PSensorHandler.GetPSensorStatusSensorFlag(PSENSOR_2) == SENSOR_DETECTED){
						this->_DXLHandler.WriteGoalVelocity(DXL_3, HOMING_SLOW_FAR_SPEED);
						this->_PSensorHandler.SetPSensorStatusHomingState(PSENSOR_2, HOMING_SLOW_FAR);
					}
					break;

				case HOMING_SLOW_FAR:
					if(_PSensorHandler.GetPSensorStatusSensorFlag(PSENSOR_2) == SENSOR_NOT_DETECTED){
						this->_DXLHandler.WriteGoalVelocity(DXL_3, HOMING_SLOW_CLOSE_SPEED);
						this->_PSensorHandler.SetPSensorStatusHomingState(PSENSOR_2, HOMING_SLOW_CLOSE);
					}
					break;

				case HOMING_SLOW_CLOSE:
					if(_PSensorHandler.GetPSensorStatusSensorFlag(PSENSOR_2) == SENSOR_DETECTED){
						this->_DXLHandler.WriteGoalVelocity(DXL_3, DXL_STOP);
						this->_PSensorHandler.SetPSensorStatusHomingState(PSENSOR_2, HOMING_OFFSET_SET);
					}
					break;

				case HOMING_OFFSET_SET:
					this->_DXLHandler.ReadPresentPosition(DXL_3);
					static int32_t nHomingOffsetValue = this->_DXLHandler.GetDXLStatusPresentPosition(DXL_3);
					this->_DXLHandler.WriteHomingOffset(DXL_3, nHomingOffsetValue);
					this->_DXLHandler.ReadPresentPosition(DXL_3);
					static int32_t nPresentPosition = this->_DXLHandler.GetDXLStatusPresentPosition(DXL_3);
					if(nPresentPosition == 0){
						this->_PSensorHandler.SetPSensorStatusHomingState(PSENSOR_2, HOMING_CPLT);
					}
					break;

				case HOMING_CPLT:
					break;
			}

			// 모든 Photo Sensor가 Homing이 완료된 경우 Complete
			if((this->_PSensorHandler.GetPSensorStatusHomingState(PSENSOR_1) == HOMING_CPLT) && (this->_PSensorHandler.GetPSensorStatusHomingState(PSENSOR_2) == HOMING_CPLT)){
				if(this->_DXLHandler.GetDxlState() == DXL_IDLE){
					this->_ucHomingFlag = HOMING_END_FLAG;
					this->_ucMainState = MAIN_IDLE;
				}
				else{
					this->_ucMainState = MAIN_HOMING_ERR;
				}
			}

			/* USER CODE END Homing */

			break;


		/* Main Operation -----------------------------------------*/
		case MAIN_OPERATION:

			/* USER CODE BEGIN Operation */

			// 예시
//			*pnOperPWM = PWM_MAX;
//			static int32_t pnOperParams[8] = {1, 1500, 2, 1500, 3, 1500, 4, 1500};
//			this->_DXLHandler.SyncWriteGoalPosition(DXL_CNT, pnOperParams);
//			static uint8_t ucCpltDxlCnt = 0;
//
//			while(1){
//				this->_DXLHandler.SyncReadPresentPosition(DXL_CNT, this->_pucDXLIDList);
//
//				ucCpltDxlCnt = 0;
//				for(size_t i=0; i<DXL_CNT; ++i){
//					if(this->_DXLHandler.GetDXLStatusPresentPosition(i+1) >= 1499){
//						ucCpltDxlCnt |= (1 << i);
//					}
//				}
//				if(ucCpltDxlCnt == 15){
//					break;
//				}
//			}
//
//			static int32_t pnOperEndParams[8] = {1, 0, 2, 0, 3, 0, 4, 0};
//			this->_DXLHandler.SyncWriteGoalPosition(DXL_CNT, pnOperEndParams);
//
//			while(1){
//				this->_DXLHandler.SyncReadPresentPosition(DXL_CNT, this->_pucDXLIDList);
//
//				ucCpltDxlCnt = 0;
//				for(size_t i=0; i<DXL_CNT; ++i){
//					if(this->_DXLHandler.GetDXLStatusPresentPosition(i+1) <= 1){
//						ucCpltDxlCnt |= (1 << i);
//					}
//				}
//				if(ucCpltDxlCnt == 15){
//					break;
//				}
//			}
//
//			*pnOperPWM = PWM_MIN;
			if(this->_DXLHandler.GetDxlState() == DXL_IDLE){
				this->_ucOperationFlag = OPERATION_END_FLAG;
				this->_ucMainState = MAIN_IDLE;
			}
			else{
				this->_ucMainState = MAIN_OPERATION_ERR;
			}

			/* USER CODE END Operation */

			break;


		/* Main Exit ----------------------------------------------*/
		case MAIN_EXIT:

			/* USER CODE BEGIN Exit */

			static int32_t pnExitParams[8] = {1, 0, 2, 0, 3, 0, 4, 0};
			this->_DXLHandler.SyncWriteLED(DXL_CNT, pnExitParams);
			this->_DXLHandler.SyncWriteTorqueEnable(DXL_CNT, pnExitParams);


			/* USER CODE END Exit */

			this->DXLClear();
			this->PSensorClear();
			this->_ucSystemFlag = FLAG_OFF;
			this->_ucHomingFlag = FLAG_OFF;
			this->_ucOperationFlag = FLAG_OFF;
			this->_ucMainState = MAIN_OFF;
			break;

		/* Error State --------------------------------------------*/
		case MAIN_INIT_ERR:
			break;

		case MAIN_HOMING_ERR:
			break;

		case MAIN_IDLE_ERR:
			break;

		case MAIN_OPERATION_ERR:
			break;

		case MAIN_EXIT_ERR:
			break;
	}

	// Power Switch가 꺼지면, 프로세스를 중단
	if(this->_ucSystemFlag == SYSTEM_END_FLAG){
		if(this->_ucMainState != MAIN_OFF){
			this->_ucMainState = MAIN_EXIT;
		}
	}
}

void TaskHandler::GPIOInput(){
	/* POWER_SW Interface ----------------------------------------------------------------*/
	bool bIsPowerSWOn = HAL_GPIO_ReadPin(POWER_SW_GPIO_Port, POWER_SW_Pin) ? false : true;
	if(bIsPowerSWOn == true){
		this->_ucSystemFlag = SYSTEM_START_FLAG;
	}
	else{
		this->_ucSystemFlag = SYSTEM_END_FLAG;
	}

	/* HOMING_SW Interface ---------------------------------------------------------------*/
	bool bIsHomingSWOn = HAL_GPIO_ReadPin(PULL_UP_IN1_GPIO_Port, PULL_UP_IN1_Pin) ? false : true;
	if(bIsHomingSWOn == true){
		this->_ucHomingFlag = HOMING_START_FLAG;
	}


	/* OPERATING_SW Interface -------------------------------------------------------------*/
	bool bIsOperatingSWOn = HAL_GPIO_ReadPin(PULL_UP_IN2_GPIO_Port, PULL_UP_IN2_Pin) ? false : true;
	if(bIsOperatingSWOn == true){
		this->_ucOperationFlag = OPERATION_START_FLAG;
	}


	/* PHOTO_SENSOR Interface ------------------------------------------------------------*/
	uint8_t ucPSensorStatus1 = HAL_GPIO_ReadPin(PULL_UP_IN3_GPIO_Port, PULL_UP_IN3_Pin) ? SENSOR_NOT_DETECTED : SENSOR_DETECTED;
	this->_PSensorHandler.SetPSensorStatusSensorFlag(PSENSOR_1, ucPSensorStatus1);

	uint8_t ucPSensorStatus2 = HAL_GPIO_ReadPin(PULL_UP_IN4_GPIO_Port, PULL_UP_IN4_Pin) ? SENSOR_NOT_DETECTED : SENSOR_DETECTED;
	this->_PSensorHandler.SetPSensorStatusSensorFlag(PSENSOR_2, ucPSensorStatus2);
}
