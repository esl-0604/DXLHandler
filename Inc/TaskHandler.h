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
#define MAIN_HOMING_SET_VEL_MODE			0x22U
#define MAIN_HOMING_VEL								0x23U
#define MAIN_HOMING_CHK_PHOTO					0x24U
#define MAIN_HOMING_VEL_RE						0x25U
#define MAIN_HOMING_CHK_PHOTO_RE			0x26U
#define	MAIN_HOMING_SET_OFFSET				0x27U
#define MAIN_HOMING_SET_POS_MODE			0x28U
#define	MAIN_HOMING_SET_INIT_POS			0x29U

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


// PWM Interface
#define PWM_MAX			42000-1
#define PWM_MIN			0


// Photo Sensor Interface
#define FAST_SPEED			200
#define SLOW_SPEED			10

#include <stdint.h>
#include <stdbool.h>
#include "main.h"
#include "DXLHandler.h"

class DXLHandler;

class TaskHandler {
	// Variables
	private:
		DXLHandler* _pDXLHandler;					// 외부에서 이식받을 DXLHandler 객체
		uint8_t* _pucDXLIDList;						// DXL 전체 ID List
		uint8_t _ucMainState;							// Main Process State


	// Methods
	public:
		TaskHandler(DXLHandler* pDXLHandler);
		virtual ~TaskHandler();

		void MainProcess(volatile uint32_t* pnPowerPWM, volatile uint32_t* pnOperPWM);
};


#endif /* ENDO_INC_TASKHANDLER_H_ */
