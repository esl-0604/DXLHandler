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

	bool bIsPWSwitchOn = HAL_GPIO_ReadPin(POWER_SW_GPIO_Port, POWER_SW_Pin) ? false : true;
	bool bIsOPSwitchOn = HAL_GPIO_ReadPin(PULL_UP_IN1_GPIO_Port, PULL_UP_IN1_Pin) ? false : true;

	/* POWER_SW가 활성화되면, 동작을 시작한다. -----------------------------------------------*/
	if(bIsPWSwitchOn == true){
		*pnPowerPWM = PWM_MAX;
	}
	else{
		if(this->_ucMainState != MAIN_OFF){
			this->_ucMainState = MAIN_EXIT;
		}
		*pnPowerPWM = PWM_MIN;
	}


	/* Main Process State ---------------------------------------------------------------*/

	switch(this->_ucMainState){
		/* Main Off -----------------------------------------------*/
		case MAIN_OFF:
			if(bIsPWSwitchOn == true){
				this->DXLInit();
				this->PSensorInit();
				this->_ucMainState = MAIN_INIT;
			}
			break;


		/* Main Init ----------------------------------------------*/
		case MAIN_INIT:
			static bool ucInitCplt = false;

			/* USER CODE BEGIN Init */

			// EX Code
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


			ucInitCplt = true;
			/* USER CODE END Init */


			if((ucInitCplt == true) && (this->_DXLHandler.GetDxlState() == DXL_IDLE)){
				this->_ucMainState = MAIN_HOMING;
			}
			break;


		/* Main Homing --------------------------------------------*/
		case MAIN_HOMING:
			static bool ucHomingCplt = false;


			/* USER CODE BEGIN Homing */

			// EX Code
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
				ucHomingCplt = true;
			}


			/* USER CODE END Homing */


			if((ucHomingCplt == true) && (this->_DXLHandler.GetDxlState() == DXL_IDLE)){
				this->_ucMainState = MAIN_IDLE;
			}
			break;


		/* Main Idle ----------------------------------------------*/
		case MAIN_IDLE:
			static bool ucIdleCplt = false;


			/* USER CODE BEGIN Idle */

			// EX Code
			if(bIsOPSwitchOn == true){
				this->_ucMainState = MAIN_OPERATION;
			}


			/* USER CODE END Idle */

			if((ucIdleCplt == true) && (this->_DXLHandler.GetDxlState() == DXL_IDLE)){
				this->_ucMainState = MAIN_OPERATION;
			}
			break;


		/* Main Operation -----------------------------------------*/
		case MAIN_OPERATION:
			static bool ucOperCplt = false;


			/* USER CODE BEGIN Operation */

			// EX Code
			*pnOperPWM = PWM_MAX;
			static int32_t pnOperParams[8] = {1, 1500, 2, 1500, 3, 1500, 4, 1500};
			this->_DXLHandler.SyncWriteGoalPosition(DXL_CNT, pnOperParams);
			static uint8_t ucCpltDxlCnt = 0;

			while(1){
				this->_DXLHandler.SyncReadPresentPosition(DXL_CNT, this->_pucDXLIDList);

				ucCpltDxlCnt = 0;
				for(size_t i=0; i<DXL_CNT; ++i){
					if(this->_DXLHandler.GetDXLStatusPresentPosition(i+1) >= 1499){
						ucCpltDxlCnt |= (1 << i);
					}
				}
				if(ucCpltDxlCnt == 15){
					break;
				}
			}

			static int32_t pnOperEndParams[8] = {1, 0, 2, 0, 3, 0, 4, 0};
			this->_DXLHandler.SyncWriteGoalPosition(DXL_CNT, pnOperEndParams);

			while(1){
				this->_DXLHandler.SyncReadPresentPosition(DXL_CNT, this->_pucDXLIDList);

				ucCpltDxlCnt = 0;
				for(size_t i=0; i<DXL_CNT; ++i){
					if(this->_DXLHandler.GetDXLStatusPresentPosition(i+1) <= 1){
						ucCpltDxlCnt |= (1 << i);
					}
				}
				if(ucCpltDxlCnt == 15){
					break;
				}
			}

			*pnOperPWM = PWM_MIN;
			/* USER CODE END Operation */


			if((ucOperCplt == true) && (this->_DXLHandler.GetDxlState() == DXL_IDLE)){
				this->_ucMainState = MAIN_IDLE;
			}
			break;


		/* Main Reset ---------------------------------------------*/
		case MAIN_RESET:
			static bool ucResetCplt = false;


			/* USER CODE BEGIN Reset */

			/* USER CODE END Reset */


			if((ucResetCplt == true) && (this->_DXLHandler.GetDxlState() == DXL_IDLE)){
				this->_ucMainState = MAIN_IDLE;
			}
			break;


		/* Main Exit ----------------------------------------------*/
		case MAIN_EXIT:
			static bool ucExitCplt = false;

			/* USER CODE BEGIN Exit */

			// EX Code
			static int32_t pnExitParams[8] = {1, 0, 2, 0, 3, 0, 4, 0};
			this->_DXLHandler.SyncWriteLED(DXL_CNT, pnExitParams);
			this->_DXLHandler.SyncWriteTorqueEnable(DXL_CNT, pnExitParams);


			ucExitCplt = true;

			/* USER CODE END Exit */


			if((ucExitCplt == true) && (this->_DXLHandler.GetDxlState() == DXL_IDLE)){
				this->DXLClear();
				this->PSensorClear();
				this->_ucMainState = MAIN_OFF;
			}
			break;

		/* Error State --------------------------------------------*/
//		case MAIN_INIT_ERR:
//			break;
//
//		case MAIN_HOMING_ERR:
//			break;
//
//		case MAIN_IDLE_ERR:
//			break;
//
//		case MAIN_OPERATION_ERR:
//			break;
//
//		case MAIN_RESET_ERR:
//			break;
	}
}
