/*
 * DXLProtocol.h
 *
 *  Created on: Mar 8, 2024
 *      Author: eslee
 */

#ifndef ENDO_INC_DXLPROTOCOL_H_
#define ENDO_INC_DXLPROTOCOL_H_

#pragma once
#include <stdint.h>
#include "Packet.h"
#include "DXLDefine.h"

class Packet;

class DXLProtocol {
	// Variables
	private:
		// Transmit
		uint8_t _ucTotalLength;						// Packet 전체 길이
		uint8_t _ucParamsLength;					// Packet의 Params 길이
		uint8_t* _pucHeader;							// #Header 	= 4
		uint8_t _ucID;										// #ID 			= 1
		uint8_t* _pucLength;	     		 		// #Length 	= 2, ( Length = TotalLength - 7 )
		uint8_t _ucInstruction;						// #Inst 		= 1
		uint8_t* _pucParams;							// #Params 	= X

		// Receive
		uint8_t _ucError;									// #Error 	= 1
		bool _bIsCRCValidate;							// 패킷의 유효성
		bool _bIsByteStuffing;						// Byte Stuffing 유무


	// Methods
	public:
		DXLProtocol();
		virtual ~DXLProtocol();

		// DXL Protocol Interface ---------------------------------------------------------------------------------------
		void SetHeader(); 																							// Header 세팅
		void SetID(uint8_t ucID);																				// ID 세팅
		void SetLength(uint8_t ucLength);																// Length 세팅
		void SetInstruction(uint8_t ucInstruction);											// Instruction 세팅
		void SetParams(uint8_t* pucParams, uint8_t ucParamsLength);			// Instruction 세팅
//		void SetByteStuffing(uint8_t* pucParams);												// ByteStuffing 세팅
		void SetCRC(uint8_t* pucPacket);																// CRC 세팅
		void SetClear();																								// 멤버변수 전부 초기화
		uint8_t* pucGetPacket();																				// 자신에게 세팅된 값들로 패킷을 만들어 반환하는 함수


		// Transmit Ping ------------------------------------------------------------------------------------------------
		Packet GetTxSyncPingPacket();
		Packet GetTxPingPacket(uint8_t ucID);


		// Transmit R/W -------------------------------------------------------------------------------------------------
		Packet GetTxReadPacket(uint8_t ucID, uint8_t ucTargetAddress, uint8_t ucTargetByte);
		Packet GetTxWritePacket(uint8_t ucID, int32_t ucTargetParams, uint8_t ucTargetAddress, uint8_t ucTargetByte);


		//Transmit sync R/W ---------------------------------------------------------------------------------------------
		Packet GetTxSyncReadPacket(uint8_t ucIdNum, uint8_t* pucIdList, uint8_t ucTargetAddress, uint8_t ucTargetByte);
		Packet GetTxSyncWritePacket(uint8_t ucIdNum, int32_t* pucTargetParams, uint8_t ucTargetAddress, uint8_t ucTargetByte);


		// Transmit optional --------------------------------------------------------------------------------------------
//		Packet GetTxRegWritePacket();
//		Packet GetTxActionPacket();
//		Packet GetTxFactoryResetPacket();
//		Packet GetTxRebootPacket();
//		Packet GetTxClearPacket();


		// Rx Validate Interface ----------------------------------------------------------------------------------------
		bool bIsCrcValidate(uint8_t* rawRxPacket, uint16_t usRxPacketLength);
		uint16_t CRCFunc(uint8_t* pucPacket, uint16_t usPacketLength);
};

#endif /* ENDO_INC_DXLPROTOCOL_H_ */
