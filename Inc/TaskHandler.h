/*
 * TaskHandler.h
 *
 *  Created on: Mar 13, 2024
 *      Author: eslee
 */

#ifndef ENDO_INC_TASKHANDLER_H_
#define ENDO_INC_TASKHANDLER_H_

#pragma once

// Main State
#define MAIN_OFF											0x00U

#define MAIN_INIT											0x10U
#define MAIN_INIT_ERR									0x11U

#define MAIN_HOMING										0x20U
#define MAIN_HOMING_ERR								0x21U


#define MAIN_IDLE											0x30U
#define MAIN_IDLE_ERR									0x31U

#define MAIN_OPERATION								0x40U
#define MAIN_OPERATION_ERR						0x41U

#define MAIN_RESET										0x50U
#define MAIN_RESET_ERR								0x51U

#define MAIN_EXIT											0x60U


// DXL Interface
#define DXL_CNT			4
#define DXL_1				1
#define DXL_2				2
#define DXL_3				3
#define DXL_4				4

#define HOMING_FAST_FAR_SPEED				-200
#define HOMING_FAST_CLOSE_SPEED			200
#define HOMING_SLOW_FAR_SPEED				-10
#define HOMING_SLOW_CLOSE_SPEED			10
#define DXL_STOP										0


// Photo Sensor Interface
#define PSENSOR_CNT		2
#define PSENSOR_1			1
#define PSENSOR_2			2



// PWM Interface
#define PWM_MAX			42000-1
#define PWM_MIN			0




#include <stdint.h>
#include <stdbool.h>
#include "main.h"
#include "DXLHandler.h"
#include "PSensorHandler.h"

class DXLHandler;
class PSensorHandler;

class TaskHandler {
	// Variables
	private:
		DXLHandler _DXLHandler;							// DXL Handler
		PSensorHandler _PSensorHandler;			// PhotoSensor Handler
		uint8_t* _pucDXLIDList;							// DXL 전체 ID List
		uint8_t* _pucPSencorIDList;					// PhotoSensor 전체 ID List

		uint8_t _ucMainState;								// Main Process State


	// Methods
	public:
		TaskHandler(UART_HandleTypeDef* huart);
		virtual ~TaskHandler();

		// Tx/Rx Interface ------------------------------------------------------------------
		void TransmitCplt();
		void RecieveCplt(uint16_t usPacketSize);


		// Child Handler Interface ----------------------------------------------------------
		void DXLInit();
		void DXLClear();
		void PSensorInit();
		void PSensorClear();


		// Main Process Interface -----------------------------------------------------------
		void MainProcess(volatile uint32_t* pnPowerPWM, volatile uint32_t* pnOperPWM);

};


#endif /* ENDO_INC_TASKHANDLER_H_ */
