/*
 * DXLHandlerV2.h
 *
 *  Created on: Apr 8, 2024
 *      Author: eslee
 */

#ifndef ENDO_INC_DXLHANDLERV2_H_
#define ENDO_INC_DXLHANDLERV2_H_

#pragma once

#include <map>
#include <vector>
#include <main.h>
#include <stdint.h>
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
	volatile int32_t nCurrentLimit = 1193;
	volatile int32_t nVelocityLimit = 200;
	volatile int32_t nMaxPositionLimit = 4095 ;
	volatile int32_t nMinPositionLimit = 0;

	// RAM Status
	volatile uint8_t ucTorqueEnable = 0;
	volatile uint8_t ucLED = 0;
	volatile uint8_t ucMoving = 0;
	volatile uint8_t ucStatusReturnLevel = 1;

	volatile int32_t nGoalCurrent = 0;
	volatile int32_t nGoalVelocity = 0;
	volatile int32_t nGoalPosition = 0;

	volatile int32_t nPresentCurrent = 0;
	volatile int32_t nPresentVelocity = 0;
	volatile int32_t nPresentPosition = 0;
};


class DXLHandlerV2 {
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
		DXLHandlerV2(UART_HandleTypeDef* huart, uint8_t ucTotalDXLCnt);
		virtual ~DXLHandlerV2();

		// DXL Handler Interface --------------------------------------------------
		void SetDXLInit();
		void SetDXLMapInit(uint8_t* pucDXLIdList);
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


		// DXL Status Interface ---------------------------------------------------
		int32_t GetDXLStatusByTargetAddress		(uint8_t ucID, uint8_t ucTargetAddress);


		// DXL Ping Call ----------------------------------------------------------
		void SyncPing();
		void Ping(uint8_t ucID);


		// DXL Single Read Call ---------------------------------------------------
		int32_t ReadCMD_1Byte	(uint8_t ucID, uint8_t ucTargetAddress);
		int32_t ReadCMD_2Byte	(uint8_t ucID, uint8_t ucTargetAddress);
		int32_t ReadCMD_4Byte	(uint8_t ucID, uint8_t ucTargetAddress);


		// DXL Single Write Call --------------------------------------------------
		void WriteCMD_1Byte 	(uint8_t ucID, uint8_t ucTargetAddress, int32_t nTargetParams);
		void WriteCMD_2Byte		(uint8_t ucID, uint8_t ucTargetAddress, int32_t nTargetParams);
		void WriteCMD_4Byte		(uint8_t ucID, uint8_t ucTargetAddress, int32_t nTargetParams);


		// DXL Sync Read Call -----------------------------------------------------
		vector<int32_t> SyncReadCMD_1Byte		(uint8_t ucIdNum, uint8_t* pucIdList, uint8_t ucTargetAddress);
		vector<int32_t> SyncReadCMD_2Byte		(uint8_t ucIdNum, uint8_t* pucIdList, uint8_t ucTargetAddress);
		vector<int32_t> SyncReadCMD_4Byte		(uint8_t ucIdNum, uint8_t* pucIdList, uint8_t ucTargetAddress);


		// DXL Sync Write Call ----------------------------------------------------
		void SyncWriteCMD_1Byte (uint8_t ucIdNum, int32_t* pnTargetParams, uint8_t ucTargetAddress);
		void SyncWriteCMD_2Byte	(uint8_t ucIdNum, int32_t* pnTargetParams, uint8_t ucTargetAddress);
		void SyncWriteCMD_4Byte	(uint8_t ucIdNum, int32_t* pnTargetParams, uint8_t ucTargetAddress);


		// DXL Write_Read Call ----------------------------------------------------
		uint8_t WriteReadCMD_1Byte	(uint8_t ucID, uint8_t ucTargetAddress, int32_t nTargetParams, uint8_t ucRetry);
		uint8_t WriteReadCMD_2Byte	(uint8_t ucID, uint8_t ucTargetAddress, int32_t nTargetParams, uint8_t ucRetry);
		uint8_t WriteReadCMD_4Byte	(uint8_t ucID, uint8_t ucTargetAddress, int32_t nTargetParams, uint8_t ucRetry);


		// DXL Sync Write_Read Call ------------------------------------------------
		uint8_t SyncWriteReadCMD_1Byte	(uint8_t ucIdNum, uint8_t* pucIdList, int32_t* pnTargetParams, uint8_t ucTargetAddress, uint8_t ucRetry);
		uint8_t SyncWriteReadCMD_2Byte	(uint8_t ucIdNum, uint8_t* pucIdList, int32_t* pnTargetParams, uint8_t ucTargetAddress, uint8_t ucRetry);
		uint8_t SyncWriteReadCMD_4Byte	(uint8_t ucIdNum, uint8_t* pucIdList, int32_t* pnTargetParams, uint8_t ucTargetAddress, uint8_t ucRetry);
};



#endif /* ENDO_INC_DXLHANDLERV2_H_ */
