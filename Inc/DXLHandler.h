/*
 * DXLHandler.h
 *
 *  Created on: Mar 13, 2024
 *      Author: eslee
 */

#ifndef ENDO_INC_DXLHANDLER_H_
#define ENDO_INC_DXLHANDLER_H_

#pragma once
#include <map>
#include <main.h>
#include <stdint.h>
#include <stdbool.h>
#include "Packet.h"
#include "DXLProtocol.h"
#include "UARTHandler.h"
#include "DXLDefine.h"

using namespace std;

class Packet;
class UARTHandler;
class DXLProtocol;

struct tDXLStatus {

	// EEPROM Status
	volatile uint8_t ucOperatingMode = 3;
	volatile int32_t nHomingOffset = 0;
	volatile int32_t nMaxPositionLimit = 0 ;
	volatile int32_t nMinPositionLimit = 0;

	// RAM Status
	volatile bool bIsTorqueEnable = false;
	volatile bool bIsLEDOn = false;
	volatile bool bIsMoving = false;
	volatile uint8_t ucStatusReturnLevel = 1;

	volatile int32_t nGoalCurrent = 0;
	volatile int32_t nGoalVelocity = 0;
	volatile int32_t nGoalPosition = 0;

	volatile int32_t nPresentCurrent = 0;
	volatile int32_t nPresentVelocity = 0;
	volatile int32_t nPresentPosition = 0;
};


class DXLHandler {
	// Variables
	private:
		UARTHandler _UARTHandler;
		DXLProtocol _DXLProtocol;
		map<uint8_t, tDXLStatus*> _mDXLStatusList;

		uint8_t _ucTotalDXLCnt;
		uint8_t _ucDxlState; 								// DXL 현재 상태
		uint8_t _ucParsingType;							// 현재 받은 Status Packet의 Type
		uint8_t _ucTxIdCnt;									// Sync Tx의 경우, 전송한 ID 개수
		uint8_t _ucRxIdCnt;									// Sync Rx의 경우, 응답한 ID 개수


	// Methods
	public:
		DXLHandler(UART_HandleTypeDef* huart, uint8_t ucTotalDXLCnt);
		virtual ~DXLHandler();

		// DXL Handler Interface --------------------------------------------------
		void SetDXLMapInit(uint8_t* pucIdList);
		void SetDXLMapClear();
		void SetDxlState(uint8_t ucDxlState);
		void ParsingRxData();
		uint8_t GetDxlState();
		int32_t CalculateParams(uint8_t* pucTargetParams, uint8_t ucTargetByte);


		// UART Interface ---------------------------------------------------------
		void TransmitPacketCplt();
		void RecievePacketCplt(uint16_t Size);
		void WaitUntilCplt();
		void TransmitAndWaitUntilCplt(Packet TxPacket);


		// DXL Ping Call ----------------------------------------------------------
		void SyncPing();
		void Ping(uint8_t ucID);


		// DXL Single Read Call ---------------------------------------------------
		void ReadOperatingMode				(uint8_t ucID);
		void ReadHomingOffset					(uint8_t ucID);
		void ReadTorqueEnable					(uint8_t ucID);
		void ReadLED									(uint8_t ucID);
		void ReadGoalCurrent					(uint8_t ucID);
		void ReadGoalVelocity					(uint8_t ucID);
		void ReadGoalPosition					(uint8_t ucID);
		void ReadMoving								(uint8_t ucID);
		void ReadPresentCurrent				(uint8_t ucID);
		void ReadPresentVelocity			(uint8_t ucID);
		void ReadPresentPosition			(uint8_t ucID);


		// DXL Single Write Call --------------------------------------------------
		void WriteOperatingMode				(uint8_t ucID, int32_t nOperatingMode);
		void WriteHomingOffset				(uint8_t ucID, int32_t nHomingOffset);
		void WriteTorqueEnableON			(uint8_t ucID);
		void WriteTorqueEnableOFF			(uint8_t ucID);
		void WriteLEDOn								(uint8_t ucID);
		void WriteLEDOff							(uint8_t ucID);
		void WriteGoalCurrent					(uint8_t ucID, int32_t nGoalCurrent);
		void WriteGoalVelocity				(uint8_t ucID, int32_t nGoalVelocity);
		void WriteGoalPosition				(uint8_t ucID, int32_t nGoalPosition);


		// DXL Sync Read Call -----------------------------------------------------
		void SyncReadOperatingMode		(uint8_t ucIdNum, uint8_t* pucIdList);
		void SyncReadHomingOffset			(uint8_t ucIdNum, uint8_t* pucIdList);
		void SyncReadTorqueEnable			(uint8_t ucIdNum, uint8_t* pucIdList);
		void SyncReadLED							(uint8_t ucIdNum, uint8_t* pucIdList);
		void SyncReadGoalPosition			(uint8_t ucIdNum, uint8_t* pucIdList);
		void SyncReadMoving						(uint8_t ucIdNum, uint8_t* pucIdList);
		void SyncReadPresentCurrent		(uint8_t ucIdNum, uint8_t* pucIdList);
		void SyncReadPresentVelocity	(uint8_t ucIdNum, uint8_t* pucIdList);
		void SyncReadPresentPosition	(uint8_t ucIdNum, uint8_t* pucIdList);


		// DXL Sync Write Call ----------------------------------------------------
		void WriteOperatingMode				(uint8_t ucIdNum, int32_t* pnTargetParams);
		void SyncWriteHomingOffset		(uint8_t ucIdNum, int32_t* pnTargetParams);
		void SyncWriteTorqueEnable		(uint8_t ucIdNum, int32_t* pnTargetParams);
		void SyncWriteLED							(uint8_t ucIdNum, int32_t* pnTargetParams);
		void SyncWriteGoalCurrent			(uint8_t ucIdNum, int32_t* pnTargetParams);
		void SyncWriteGoalVelocity		(uint8_t ucIdNum, int32_t* pnTargetParams);
		void SyncWriteGoalPosition		(uint8_t ucIdNum, int32_t* pnTargetParams);
};



#endif /* ENDO_INC_DXLHANDLER_H_ */
