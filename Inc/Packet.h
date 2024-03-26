/*
 * Packet.h
 *
 *  Created on: Mar 8, 2024
 *      Author: eslee
 */

#ifndef ENDO_INC_PACKET_H_
#define ENDO_INC_PACKET_H_

#pragma once
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include "DXLDefine.h"


class Packet {
	// Variables
	private:
		uint8_t* _pucPacket;
		uint16_t _usPacketLength;
		uint16_t _usPacketMaxLength;


	// Methods
	public:
		Packet(uint16_t usPacketMaxLength = DXL_PACKET_MAX_SIZE);
		virtual ~Packet();
		uint8_t& operator[] (const uint8_t ucIndex);									// 배열 연산자


		// Get ----------------------------------------------------------------------
		uint8_t* pucGetPacket();																			// 패킷 반환
		uint16_t usGetPacketLength();																	// 패킷 길이 반환


		// Set ----------------------------------------------------------------------
		void SetPacket(uint8_t* pucPacket, uint16_t usPacketLength);	// 패킷 세팅
		void SetPacketClear(uint8_t ucClearParam = 0);								// 패킷 초기화
};



#endif /* ENDO_INC_PACKET_H_ */
