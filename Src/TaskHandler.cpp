/*
 * TaskHandler.cpp
 *
 *  Created on: Mar 13, 2024
 *      Author: eslee
 */

// * 모터의 변위를 연속으로 제어하는 경우, State를 분할하여 직관적으로 표현하도록 한다.
// * USER CODE BEGIN / USER CODE END Interface를 최대한 활용하도록 한다.


#include "TaskHandler.h"



TaskHandler::TaskHandler(DXLHandler* pDXLHandler) : _pDXLHandler(pDXLHandler){
	this->_ucMainState = MAIN_OFF;
	this->_pucDXLIDList = new uint8_t[DXL_CNT];

	/* USER CODE BEGIN DXL_ID List */
	_pucDXLIDList[0] = DXL_1;
	_pucDXLIDList[1] = DXL_2;
	_pucDXLIDList[2] = DXL_3;
	_pucDXLIDList[3] = DXL_4;
	/* USER CODE END DXL_ID List */
}

TaskHandler::~TaskHandler(){

}

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
				this->_pDXLHandler->SetDXLMapInit(this->_pucDXLIDList);
				this->_ucMainState = MAIN_INIT;
			}
			break;


		/* Main Init ----------------------------------------------*/
		case MAIN_INIT:


			/* USER CODE BEGIN Init */

			// EX Code
			// ERROM Read
			this->_pDXLHandler->SyncPing();
			this->_pDXLHandler->SyncReadOperatingMode(DXL_CNT, this->_pucDXLIDList);
			this->_pDXLHandler->SyncReadHomingOffset(DXL_CNT, this->_pucDXLIDList);
			this->_pDXLHandler->SyncReadCurrentLimit(DXL_CNT, this->_pucDXLIDList);
			this->_pDXLHandler->SyncReadVelocityLimit(DXL_CNT, this->_pucDXLIDList);
			this->_pDXLHandler->SyncReadMaxPositionLimit(DXL_CNT, this->_pucDXLIDList);
			this->_pDXLHandler->SyncReadMinPositionLimit(DXL_CNT, this->_pucDXLIDList);

			// RAM Read
			this->_pDXLHandler->SyncReadTorqueEnable(DXL_CNT, this->_pucDXLIDList);
			this->_pDXLHandler->SyncReadLED(DXL_CNT, this->_pucDXLIDList);
			this->_pDXLHandler->SyncReadStatusReturnLevel(DXL_CNT, this->_pucDXLIDList);
			this->_pDXLHandler->SyncReadGoalPosition(DXL_CNT, this->_pucDXLIDList);
			this->_pDXLHandler->SyncReadMoving(DXL_CNT, this->_pucDXLIDList);
			this->_pDXLHandler->SyncReadPresentCurrent(DXL_CNT, this->_pucDXLIDList);
			this->_pDXLHandler->SyncReadPresentVelocity(DXL_CNT, this->_pucDXLIDList);
			this->_pDXLHandler->SyncReadPresentPosition(DXL_CNT, this->_pucDXLIDList);

			// Write
			static int32_t pnInitParams[8] = {1, 1, 2, 1, 3, 1, 4, 1};
			this->_pDXLHandler->SyncWriteLED(DXL_CNT, pnInitParams);
			this->_pDXLHandler->SyncWriteTorqueEnable(DXL_CNT, pnInitParams);


			/* USER CODE END Init */


			if(this->_pDXLHandler->GetDxlState() == DXL_IDLE){
				this->_ucMainState = MAIN_HOMING;
			}
			break;


		/* Main Homing --------------------------------------------*/
		case MAIN_HOMING:


			/* USER CODE BEGIN Homing */

			// EX Code
			static int32_t pnHomingParams[8] = {1, 0, 2, 0, 3, 0, 4, 0};
			this->_pDXLHandler->SyncWriteGoalPosition(DXL_CNT, pnHomingParams);


			/* USER CODE END Homing */


			if(this->_pDXLHandler->GetDxlState() == DXL_IDLE){
				this->_ucMainState = MAIN_IDLE;
			}
			break;


		/* Main Idle ----------------------------------------------*/
		case MAIN_IDLE:


			/* USER CODE BEGIN Idle */

			// EX Code
			if(bIsOPSwitchOn == true){
				this->_ucMainState = MAIN_OPERATION;
			}


			/* USER CODE END Idle */


			break;


		/* Main Operation -----------------------------------------*/
		case MAIN_OPERATION:


			/* USER CODE BEGIN Operation */

			// EX Code
			*pnOperPWM = PWM_MAX;
			static int32_t pnOperParams[8] = {1, 1500, 2, 1500, 3, 1500, 4, 1500};
			this->_pDXLHandler->SyncWriteGoalPosition(DXL_CNT, pnOperParams);
			static uint8_t ucCpltDxlCnt = 0;

			while(1){
				this->_pDXLHandler->SyncReadPresentPosition(DXL_CNT, this->_pucDXLIDList);

				ucCpltDxlCnt = 0;
				for(size_t i=0; i<DXL_CNT; ++i){
					if(this->_pDXLHandler->GetDXLStatusPresentPosition(i+1) >= 1499){
						ucCpltDxlCnt |= (1 << i);
					}
				}
				if(ucCpltDxlCnt == 15){
					break;
				}
			}

			static int32_t pnOperEndParams[8] = {1, 0, 2, 0, 3, 0, 4, 0};
			this->_pDXLHandler->SyncWriteGoalPosition(DXL_CNT, pnOperEndParams);

			while(1){
				this->_pDXLHandler->SyncReadPresentPosition(DXL_CNT, this->_pucDXLIDList);

				ucCpltDxlCnt = 0;
				for(size_t i=0; i<DXL_CNT; ++i){
					if(this->_pDXLHandler->GetDXLStatusPresentPosition(i+1) <= 1){
						ucCpltDxlCnt |= (1 << i);
					}
				}
				if(ucCpltDxlCnt == 15){
					break;
				}
			}
			/* USER CODE END Operation */


			if(this->_pDXLHandler->GetDxlState() == DXL_IDLE){
				*pnOperPWM = PWM_MIN;
				this->_ucMainState = MAIN_IDLE;
			}
			break;


		/* Main Reset ---------------------------------------------*/
		case MAIN_RESET:


			/* USER CODE BEGIN Reset */

			/* USER CODE END Reset */


			if(this->_pDXLHandler->GetDxlState() == DXL_IDLE){
				this->_ucMainState = MAIN_INIT;
			}
			break;


		/* Main Exit ----------------------------------------------*/
		case MAIN_EXIT:


			/* USER CODE BEGIN Exit */

			// EX Code
			static int32_t pnExitParams[8] = {1, 0, 2, 0, 3, 0, 4, 0};
			this->_pDXLHandler->SyncWriteLED(DXL_CNT, pnExitParams);
			this->_pDXLHandler->SyncWriteTorqueEnable(DXL_CNT, pnExitParams);


			/* USER CODE END Exit */


			if(this->_pDXLHandler->GetDxlState() == DXL_IDLE){
				this->_pDXLHandler->SetDXLMapClear();
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
