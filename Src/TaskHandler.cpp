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
	this->_ucOperationState = OPERATION_START;
	this->_ucSystemFlag = FLAG_OFF;
	this->_ucHomingFlag = FLAG_OFF;
	this->_ucOperationFlag = FLAG_OFF;
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


// DXL Handler Interface -----------------------------------------------------------------------------------
void TaskHandler::DXLInit(){
	this->_DXLHandler.SetDXLInit();
	this->_DXLHandler.SetDXLMapInit(this->_pucDXLIDList);
}

void TaskHandler::DXLClear(){
	this->_DXLHandler.SetDXLMapClear();
}

void TaskHandler::DXLWrite_Read_Check(uint8_t ucWriteReadCheckValue){
	if(ucWriteReadCheckValue == 0){
		// 한번 더 보내라는 Flag
	}
}


// PSensor Handler Interface -------------------------------------------------------------------------------
void TaskHandler::PSensorInit(){
	this->_PSensorHandler.SetPSensorMapInit(this->_pucPSencorIDList);
}

void TaskHandler::PSensorClear(){
	this->_PSensorHandler.SetPSensorMapClear();
}


// Main Process Interface ----------------------------------------------------------------------------------
void TaskHandler::Main_Process(){
	switch(this->_ucMainState){
		/* Main Off -----------------------------------------------*/
		case MAIN_OFF: {
			if(this->_ucSystemFlag == SYSTEM_START_FLAG){
				this->DXLInit();
				this->PSensorInit();
				this->_ucMainState = MAIN_INIT;
			}
			break;
		}


		/* Main Init ----------------------------------------------*/
		case MAIN_INIT: {

			/* USER CODE BEGIN Init */

			// Write variables
			int32_t pnOperatingModeParams[8] = {DXL_1, 3, DXL_2, 3, DXL_3, 3, DXL_4, 3};
			int32_t pnHomingOffsetParams[8] = {DXL_1, 0, DXL_2, 0, DXL_3, 0, DXL_4, 0};
			int32_t pnCurrentLimitParams[8] = {DXL_1, 1000, DXL_2, 1000, DXL_3, 1000, DXL_4, 1000};
			int32_t pnVelocityLimitParams[8] = {DXL_1, 190, DXL_2, 190, DXL_3, 190, DXL_4, 190};
			int32_t pnMaxPositionLimitParams[8] = {DXL_1, 4000, DXL_2, 4000, DXL_3, 4000, DXL_4, 4000};
			int32_t pnMinPositionLimitParams[8] = {DXL_1, 0, DXL_2, 0, DXL_3, 0, DXL_4, 0};

			int32_t pnTorqueEnableOFFParams[8] = {DXL_1, 0, DXL_2, 0, DXL_3, 0, DXL_4, 0};
			int32_t pnTorqueEnableONParams[8] = {DXL_1, 1, DXL_2, 1, DXL_3, 1, DXL_4, 1};
			int32_t pnLEDONParams[8] = {DXL_1, 1, DXL_2, 1, DXL_3, 1, DXL_4, 1};
			int32_t pnStatusReturnLevelParams[8] = {DXL_1, 2, DXL_2, 2, DXL_3, 2, DXL_4, 2};


			// ERROM Write & Read
			this->DXLWrite_Read_Check( this->_DXLHandler.SyncWriteReadTorqueEnable(DXL_CNT, this->_pucDXLIDList, pnTorqueEnableOFFParams) );
			this->DXLWrite_Read_Check( this->_DXLHandler.SyncWriteReadOperatingMode(DXL_CNT, this->_pucDXLIDList, pnOperatingModeParams) );
			this->DXLWrite_Read_Check( this->_DXLHandler.SyncWriteReadHomingOffset(DXL_CNT, this->_pucDXLIDList, pnHomingOffsetParams) );
			this->DXLWrite_Read_Check( this->_DXLHandler.SyncWriteReadCurrentLimit(DXL_CNT, this->_pucDXLIDList, pnCurrentLimitParams) );
			this->DXLWrite_Read_Check( this->_DXLHandler.SyncWriteReadVelocityLimit(DXL_CNT, this->_pucDXLIDList, pnVelocityLimitParams) );
			this->DXLWrite_Read_Check( this->_DXLHandler.SyncWriteReadMaxPositionLimit(DXL_CNT, this->_pucDXLIDList, pnMaxPositionLimitParams) );
			this->DXLWrite_Read_Check( this->_DXLHandler.SyncWriteReadMinPositionLimit(DXL_CNT, this->_pucDXLIDList, pnMinPositionLimitParams) );

			// RAM Write & Read
			this->DXLWrite_Read_Check( this->_DXLHandler.SyncWriteReadTorqueEnable(DXL_CNT, this->_pucDXLIDList, pnTorqueEnableONParams) );
			this->DXLWrite_Read_Check( this->_DXLHandler.SyncWriteReadLED(DXL_CNT, this->_pucDXLIDList, pnLEDONParams) );
			this->DXLWrite_Read_Check( this->_DXLHandler.SyncWriteReadStatusReturnLevel(DXL_CNT, this->_pucDXLIDList, pnStatusReturnLevelParams) );

			if(this->_DXLHandler.GetDxlState() == DXL_IDLE){
				this->_ucMainState = MAIN_IDLE;
			}
			else{
				this->_ucMainState = MAIN_INIT_ERR;
			}

			break;

			/* USER CODE END Init */
		}



		/* Main Idle ----------------------------------------------*/
		case MAIN_IDLE: {

			/* USER CODE BEGIN Idle */

			// Homing Switch 클릭 시 --> Homing
			if(this->_ucHomingFlag == HOMING_START_FLAG){
				this->_ucMainState = MAIN_HOMING;
			}

			// Operating Switch 클릭 시 (Homing이 되어있다면) --> Operation / (Homing이 되어있지 않다면) --> Operation Flag 취소
			if(this->_ucOperationFlag == OPERATION_START_FLAG){
				if(this->_ucHomingFlag == HOMING_END_FLAG){
					this->_ucOperationState = OPERATION_START;
					this->_ucMainState = MAIN_OPERATION;
				}
				else{
					this->_ucOperationFlag = FLAG_OFF;
				}
			}

			// Operation 종료 시, Flag 전부 초기화
			if(this->_ucOperationFlag == OPERATION_END_FLAG){
				this->_ucHomingFlag = FLAG_OFF;
				this->_ucOperationFlag = FLAG_OFF;

			}
			/* USER CODE END Idle */

			break;
		}


		/* Main Homing --------------------------------------------*/
		case MAIN_HOMING: {

			/* USER CODE BEGIN Homing */
			uint8_t PSensorHomingCpltCnt = 0;

			// PSensor 1 Homing --> DXL_1
			PSensorHomingCpltCnt += this->PSensorHoming(PSENSOR_1, DXL_1);
			PSensorHomingCpltCnt += this->PSensorHoming(PSENSOR_2, DXL_2);
			/* USER CODE END Homing */

			// 모든 Photo Sensor가 Homing이 완료된 경우 Complete
			if((PSensorHomingCpltCnt == PSENSOR_CNT)){
				if(this->_DXLHandler.GetDxlState() == DXL_IDLE){
					this->_PSensorHandler.SetPSensorStatusHomingStateReset();
					this->_ucHomingFlag = HOMING_END_FLAG;
					this->_ucMainState = MAIN_IDLE;
				}
				else{
					this->_ucMainState = MAIN_HOMING_ERR;
				}
			}
			break;
		}


		/* Main Operation -----------------------------------------*/
		case MAIN_OPERATION: {

			/* USER CODE BEGIN Operation */
			bool DXLOperationCplt = false;

			DXLOperationCplt = this->DXLOperation();
			/* USER CODE END Operation */

			if(DXLOperationCplt == true){
				if(this->_DXLHandler.GetDxlState() == DXL_IDLE){
					this->_ucMainState = MAIN_IDLE;
				}
				else{
					this->_ucMainState = MAIN_OPERATION_ERR;
				}
			}

			break;
		}


		/* Main Exit ----------------------------------------------*/
		case MAIN_EXIT: {

			/* USER CODE BEGIN Exit */

			int32_t pnExitParams[8] = {1, 0, 2, 0, 3, 0, 4, 0};
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
		}


		/* Error State --------------------------------------------*/
		case MAIN_INIT_ERR: {
			break;
		}

		case MAIN_HOMING_ERR: {
			break;
		}

		case MAIN_IDLE_ERR: {
			break;
		}

		case MAIN_OPERATION_ERR: {
			break;
		}

		case MAIN_EXIT_ERR: {
			break;
		}

	}

	// Power Switch가 꺼지면, 프로세스를 중단
	if(this->_ucSystemFlag == SYSTEM_END_FLAG){
		if(this->_ucMainState > MAIN_OFF){
			this->_ucMainState = MAIN_EXIT;
		}
	}
}

void TaskHandler::GPIO_Input(){
	// POWER_SW Interface ----------------------------------------------------------------
	bool bIsPowerSWOn = HAL_GPIO_ReadPin(POWER_SW_GPIO_Port, POWER_SW_Pin) ? false : true;
	if(bIsPowerSWOn == true){
		this->_ucSystemFlag = SYSTEM_START_FLAG;
	}
	else{
		this->_ucSystemFlag = SYSTEM_END_FLAG;
	}

	// HOMING_SW Interface ---------------------------------------------------------------
	bool bIsHomingSWOn = HAL_GPIO_ReadPin(PULL_UP_IN1_GPIO_Port, PULL_UP_IN1_Pin) ? false : true;
	if(bIsHomingSWOn == true){
		this->_ucHomingFlag = HOMING_START_FLAG;
	}


	// OPERATING_SW Interface ------------------------------------------------------------
	bool bIsOperatingSWOn = HAL_GPIO_ReadPin(PULL_UP_IN3_GPIO_Port, PULL_UP_IN3_Pin) ? false : true;
	if(bIsOperatingSWOn == true){
		this->_ucOperationFlag = OPERATION_START_FLAG;
	}
	else{
		if(this->_ucOperationFlag == OPERATION_START_FLAG){
			this->_ucOperationFlag = OPERATION_END_FLAG;
		}
	}


	// PHOTO_SENSOR Interface ------------------------------------------------------------
	uint8_t ucPSensorStatus1 = HAL_GPIO_ReadPin(PULL_UP_IN5_GPIO_Port, PULL_UP_IN5_Pin) ? SENSOR_DETECTED : SENSOR_NOT_DETECTED;
	this->_PSensorHandler.SetPSensorStatusSensorFlag(PSENSOR_1, ucPSensorStatus1);

	uint8_t ucPSensorStatus2 = HAL_GPIO_ReadPin(PULL_UP_IN7_GPIO_Port, PULL_UP_IN7_Pin) ? SENSOR_DETECTED : SENSOR_NOT_DETECTED;
	this->_PSensorHandler.SetPSensorStatusSensorFlag(PSENSOR_2, ucPSensorStatus2);
}

void TaskHandler::GPIO_PWM(volatile uint32_t* pnPowerPWM, volatile uint32_t* pnHomingPWM, volatile uint32_t* pnOperPWM){
	// POWER_SW Interface ----------------------------------------------------------------
	if(this->_ucSystemFlag == SYSTEM_START_FLAG){
		*pnPowerPWM = PWM_MAX;
	}
	else{
		*pnPowerPWM = PWM_MIN;
		*pnHomingPWM = PWM_MIN;
		*pnOperPWM = PWM_MIN;
	}

	// HOMING_SW Interface ---------------------------------------------------------------
	if(this->_ucHomingFlag == HOMING_START_FLAG){
		*pnHomingPWM = PWM_MAX;
	}
	else{
		*pnHomingPWM = PWM_MIN;
	}

	// OPERATING_SW Interface ------------------------------------------------------------
	if(this->_ucOperationFlag == OPERATION_START_FLAG){
		*pnOperPWM = PWM_MAX;
	}
	else{
		*pnOperPWM = PWM_MIN;
	}
}


// Main Process Sub Interface -------------------------------------------------------------------------------
uint8_t TaskHandler::PSensorHoming(uint8_t ucPSensorID, uint8_t ucDXLID){
	switch(this->_PSensorHandler.GetPSensorStatusHomingState(ucPSensorID)){
		case HOMING_START: {
			this->DXLWrite_Read_Check( this->_DXLHandler.WriteReadTorqueEnableOff(ucDXLID) );
			this->DXLWrite_Read_Check( this->_DXLHandler.WriteReadOperatingMode(ucDXLID, 1) );
			this->DXLWrite_Read_Check( this->_DXLHandler.WriteReadTorqueEnableOn(ucDXLID) );

			if(this->_PSensorHandler.GetPSensorStatusSensorFlag(ucPSensorID) == SENSOR_DETECTED){
				this->_DXLHandler.WriteGoalVelocity(ucDXLID, HOMING_FAST_FAR_VELOCITY);
				this->_PSensorHandler.SetPSensorStatusHomingState(ucPSensorID, HOMING_FAST_FAR);
			}
			else{
				this->_DXLHandler.WriteGoalVelocity(ucDXLID, HOMING_FAST_CLOSE_VELOCITY);
				this->_PSensorHandler.SetPSensorStatusHomingState(ucPSensorID, HOMING_FAST_CLOSE);
			}

			break;
		}

		case HOMING_FAST_FAR: {
			if(_PSensorHandler.GetPSensorStatusSensorFlag(ucPSensorID) == SENSOR_NOT_DETECTED){
				this->_DXLHandler.WriteGoalVelocity(ucDXLID, HOMING_SLOW_CLOSE_VELOCITY);
				this->_PSensorHandler.SetPSensorStatusHomingState(ucPSensorID, HOMING_SLOW_CLOSE);
			}
			break;

		case HOMING_FAST_CLOSE:
			if(_PSensorHandler.GetPSensorStatusSensorFlag(ucPSensorID) == SENSOR_DETECTED){
				this->_DXLHandler.WriteGoalVelocity(ucDXLID, HOMING_SLOW_FAR_VELOCITY);
				this->_PSensorHandler.SetPSensorStatusHomingState(ucPSensorID, HOMING_SLOW_FAR);
			}
			break;
		}

		case HOMING_SLOW_FAR: {
			if(_PSensorHandler.GetPSensorStatusSensorFlag(ucPSensorID) == SENSOR_NOT_DETECTED){
				this->_DXLHandler.WriteGoalVelocity(ucDXLID, HOMING_SLOW_CLOSE_VELOCITY);
				this->_PSensorHandler.SetPSensorStatusHomingState(ucPSensorID, HOMING_SLOW_CLOSE);
			}
			break;
		}

		case HOMING_SLOW_CLOSE: {
			if(_PSensorHandler.GetPSensorStatusSensorFlag(ucPSensorID) == SENSOR_DETECTED){
				this->_DXLHandler.WriteGoalVelocity(ucDXLID, DXL_STOP_VELOCITY);
				this->_PSensorHandler.SetPSensorStatusHomingState(ucPSensorID, HOMING_OFFSET_SET);
			}
			break;
		}

		case HOMING_OFFSET_SET: {
			this->_DXLHandler.WriteTorqueEnableOff(ucDXLID);
			this->_DXLHandler.WriteHomingOffset(ucDXLID, 0);

			this->_DXLHandler.ReadPresentPosition(ucDXLID);
			this->_DXLHandler.WriteHomingOffset(ucDXLID, (-1) * this->_DXLHandler.GetDXLStatusPresentPosition(ucDXLID));
			this->_DXLHandler.ReadHomingOffset(ucDXLID);
			this->_DXLHandler.ReadPresentPosition(ucDXLID);

			this->_DXLHandler.WriteOperatingMode(ucDXLID, 3);
			this->_DXLHandler.WriteTorqueEnableOn(ucDXLID);

			// Offset 이 계속 변동되는 문제 발생 -> 극복하는 알고리즘 개발
			uint8_t ucPresentPosition = this->_DXLHandler.GetDXLStatusPresentPosition(ucDXLID);
			if(((-1) * DXL_POSITION_OFFSET <= ucPresentPosition) && (ucPresentPosition <= DXL_POSITION_OFFSET)){
				this->_PSensorHandler.SetPSensorStatusHomingState(ucPSensorID, HOMING_CPLT);
			}
			break;
		}

		case HOMING_CPLT: {
			return 1;
			break;
		}

		return 0;
	}
}

bool TaskHandler::DXLOperation(){
	if(this->_ucOperationFlag == OPERATION_END_FLAG){
		if(this->_ucOperationState <= OPERATION_BUSY){
			this->_ucOperationState = OPERATION_STOP;
		}
	}

	switch(this->_ucOperationState){
		case OPERATION_START: {
			int32_t pnTorqueEnalbeOffParams[8] = {DXL_1, 0, DXL_2, 0, DXL_3, 0, DXL_4, 0};
			int32_t pnTorqueEnalbeOnParams[8] = {DXL_1, 1, DXL_2, 1, DXL_3, 1, DXL_4, 1};
			int32_t pnOperatingModeParams[8] = {DXL_1, 1, DXL_2, 1, DXL_3, 1, DXL_4, 1};
			int32_t pnGoalVelocityParams[8] = {DXL_1, 200, DXL_2, 200, DXL_3, 200, DXL_4, 200};

			this->_DXLHandler.SyncWriteTorqueEnable(DXL_CNT, pnTorqueEnalbeOffParams);
			this->_DXLHandler.SyncWriteOperatingMode(DXL_CNT, pnOperatingModeParams);
			this->_DXLHandler.SyncWriteTorqueEnable(DXL_CNT, pnTorqueEnalbeOnParams);
			this->_DXLHandler.SyncWriteGoalVelocity(DXL_CNT, pnGoalVelocityParams);

			this->_ucOperationState = OPERATION_BUSY;
			break;
		}

		case OPERATION_BUSY: {
			this->_DXLHandler.SyncReadPresentVelocity(DXL_CNT, this->_pucDXLIDList);
			this->_DXLHandler.SyncReadPresentPosition(DXL_CNT, this->_pucDXLIDList);
			break;
		}

		case OPERATION_STOP: {
			int32_t pnGoalVelocityParams[8] = {DXL_1, 0, DXL_2, 0, DXL_3, 0, DXL_4, 0};
			int32_t pnTorqueEnalbeOffParams[8] = {DXL_1, 0, DXL_2, 0, DXL_3, 0, DXL_4, 0};
			int32_t pnTorqueEnalbeOnParams[8] = {DXL_1, 1, DXL_2, 1, DXL_3, 1, DXL_4, 1};
			int32_t pnOperatingModeParams[8] = {DXL_1, 3, DXL_2, 3, DXL_3, 3, DXL_4, 3};

			this->_DXLHandler.SyncWriteGoalVelocity(DXL_CNT, pnGoalVelocityParams);
			this->_DXLHandler.SyncWriteTorqueEnable(DXL_CNT, pnTorqueEnalbeOffParams);
			this->_DXLHandler.SyncWriteOperatingMode(DXL_CNT, pnOperatingModeParams);
			this->_DXLHandler.SyncWriteTorqueEnable(DXL_CNT, pnTorqueEnalbeOnParams);

			this->_ucOperationState = OPERATION_CPLT;
			break;
		}

		case OPERATION_CPLT: {
			return true;
			break;
		}
	}

	return false;
}


