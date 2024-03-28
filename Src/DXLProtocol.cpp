/*
 * DXLProtocol.cpp
 *
 *  Created on: Mar 8, 2024
 *      Author: eslee
 */


#include "DXLProtocol.h"

DXLProtocol::DXLProtocol(){
	this->_ucID = 0;
	this->_ucTotalLength = 0;
	this->_ucParamsLength = 0;
	this->_pucHeader = nullptr;
	this->_pucLength = nullptr;
	this->_ucInstruction = 0;
	this->_pucParams = nullptr;
	this->_ucError = 0;
	this->_bIsCRCValidate = false;
	this->_bIsByteStuffing = false;
}

DXLProtocol::~DXLProtocol(){
}


// DXL Protocol Interface ---------------------------------------------------------------------------------------
void DXLProtocol::SetHeader(){
	// Header Size == 4
	this->_pucHeader = new uint8_t[DXL_PACKET_HEADER_SIZE];
	_pucHeader[0] = 0xFF;
	_pucHeader[1] = 0xFF;
	_pucHeader[2] = 0xFD;
	_pucHeader[3] = 0x00;
}

void DXLProtocol::SetID(uint8_t ucID){
	this->_ucID = ucID;
}

void DXLProtocol::SetLength(uint8_t ucLength){
	// Length Size == 2
	this->_pucLength = new uint8_t[DXL_PACKET_LENGTH_SIZE];
	_pucLength[0] = ucLength & 0xFF;
	_pucLength[1] = (ucLength >> DXL_SHIFT_8_BIT) & 0xFF;
}

void DXLProtocol::SetInstruction(uint8_t ucInstruction){
	this->_ucInstruction = ucInstruction;
}

void DXLProtocol::SetParams(uint8_t* pucParams, uint8_t ucParamsLength){
	// Total Length == Header(4) + Id(1) + Length(2) + Inst(1) + Params(X) + CRC(2)
	this->_ucTotalLength = DXL_PACKET_HEADER_SIZE + DXL_PACKET_ID_SIZE + DXL_PACKET_LENGTH_SIZE + DXL_PACKET_INST_SIZE + ucParamsLength + DXL_PACKET_CRC_SIZE;
	this->_ucParamsLength = ucParamsLength;
	if(ucParamsLength > 0){
		this->_pucParams = new uint8_t[ucParamsLength];
		memcpy(_pucParams, pucParams, ucParamsLength);
	}

	delete[] pucParams;
}

// 추후 수정 필요
//void DXLProtocol::SetByteStuffing(uint8_t* pucPacket,uint8_t ucLength){
//	vector<uint8_t> vucStuffedPacket;
//
//	for (int i = 0; i < ucLength; ++i) {
//		stuffedPacket.push_back(pucPacket[i]);
//	}
//
//  for (int i = 7; i < ucLength - 3; ++i) {
//		// "0xFF 0xFF 0xFD" 패턴 발견 시
//		if (i < ucLength - 4 && stuffedPacket[i] == 0xFF && stuffedPacket[i + 1] == 0xFF && stuffedPacket[i + 2] == 0xFD) {
//			// "0xFD" 추가
//			stuffedPacket.insert(stuffedPacket.begin() + i + 3, 0xFD);
//			i += 3; // 패턴 다음으로 인덱스 이동
//		}
//  }
//
//  return vucStuffedPacket;
//}

void DXLProtocol::SetCRC(uint8_t* pucPacket){
	// CRC는 항상 패킷 후미의 2칸을 차지 (Fast Sync Read 제외)
 	uint16_t usCRC = this->CRCFunc(pucPacket, this->_ucTotalLength - DXL_PACKET_CRC_SIZE);
	pucPacket[this->_ucTotalLength - 2] = (usCRC) & 0xFF;
	pucPacket[this->_ucTotalLength - 1] = (usCRC >> DXL_SHIFT_8_BIT) & 0xFF;
}

void DXLProtocol::SetClear(){
	this->_ucTotalLength = 0;
	this->_ucParamsLength = 0;
	this->_ucID = 0;
	this->_ucInstruction = 0;
	this->_ucError = 0;
	this->_bIsCRCValidate = false;
	this->_bIsByteStuffing = false;
	delete[] this->_pucHeader; this->_pucHeader = nullptr;
	delete[] this->_pucLength; this->_pucLength = nullptr;
	delete[] this->_pucParams; this->_pucParams = nullptr;
}

uint8_t* DXLProtocol::pucGetPacket(){
	uint8_t* pucDXLPacket = new uint8_t[this->_ucTotalLength];

	for(size_t i=0; i<DXL_PACKET_HEADER_SIZE; ++i){ 						// Header 복사
		pucDXLPacket[i] = this->_pucHeader[i];
	}
	pucDXLPacket[DXL_PACKET_ID] = this->_ucID;									// Id 복사
	for(size_t i=0; i<DXL_PACKET_LENGTH_SIZE; ++i){							// Length 복사
		pucDXLPacket[DXL_PACKET_LEN+i] = this->_pucLength[i];
	}
	pucDXLPacket[DXL_PACKET_INST] = _ucInstruction;							// Instruction 복사
	for(size_t i=0; i<this->_ucParamsLength; ++i){							// Params 복사
		pucDXLPacket[DXL_PACKET_PARAMS+i] = this->_pucParams[i];
	}
	this->SetCRC(pucDXLPacket);																	// CRC 삽입

	return pucDXLPacket;
}


// Transmit Ping ------------------------------------------------------------------------------------------------
Packet DXLProtocol::GetTxSyncPingPacket(){
	this->SetHeader();
	this->SetID(BROADCAST_ID);
	this->SetLength(DXL_PING_PACKET_LENGTH);
	this->SetInstruction(PING);
	this->SetParams(nullptr, 0);
	uint8_t* pucPingPacket = this->pucGetPacket();

	Packet TxPingPacket(this->_ucTotalLength);
	TxPingPacket.SetPacket(pucPingPacket, this->_ucTotalLength);
	this->SetClear();
	return TxPingPacket;
}

Packet DXLProtocol::GetTxPingPacket(uint8_t ucID){
	this->SetHeader();
	this->SetID(ucID);
	this->SetLength(DXL_PING_PACKET_LENGTH);
	this->SetInstruction(PING);
	this->SetParams(nullptr, 0);
	uint8_t* pucPingPacket = this->pucGetPacket();

	Packet TxPingPacket(this->_ucTotalLength);
	TxPingPacket.SetPacket(pucPingPacket, this->_ucTotalLength);
	this->SetClear();
	return TxPingPacket;
}


// Transmit R/W -------------------------------------------------------------------------------------------------
Packet DXLProtocol::GetTxReadPacket(uint8_t ucID, uint8_t ucTargetAddress, uint8_t ucTargetByte){
	// Read Packet의 경우, 항상 [Address 2Byte + Size 2Byte = 총 4Byte] 크기의 Parameter를 가진다.
	uint8_t* pucParams = new uint8_t[DXL_PACKET_TARGET_ADDR_SIZE + DXL_PACKET_TARGET_BYTE_SIZE];
	pucParams[0] = ucTargetAddress & 0xFF;
	pucParams[1] = (ucTargetAddress >> DXL_SHIFT_8_BIT) & 0xFF;
	pucParams[2] = ucTargetByte & 0xFF;
	pucParams[3] = (ucTargetByte >> DXL_SHIFT_8_BIT) & 0xFF;

	this->SetHeader();
	this->SetID(ucID);
	this->SetLength(DXL_READ_PACKET_LENGTH);
	this->SetInstruction(READ);
	this->SetParams(pucParams, DXL_PACKET_TARGET_ADDR_SIZE + DXL_PACKET_TARGET_BYTE_SIZE);
	uint8_t* pucReadPacket = this->pucGetPacket();

	Packet TxReadPacket(this->_ucTotalLength);
	TxReadPacket.SetPacket(pucReadPacket, this->_ucTotalLength);
	this->SetClear();
	return TxReadPacket;
}

Packet DXLProtocol::GetTxWritePacket(uint8_t ucID, int32_t ucTargetParams, uint8_t ucTargetAddress, uint8_t ucTargetByte){
	// Write Packet의 경우, [Address 2Byte + Size만큼의 Byte] 크기의 Parameter를 가진다.
	uint8_t* pucParams = new uint8_t[DXL_PACKET_TARGET_ADDR_SIZE + ucTargetByte];
	pucParams[0] = ucTargetAddress & 0xFF;
	pucParams[1] = (ucTargetAddress >> DXL_SHIFT_8_BIT) & 0xFF;
	for(size_t i=0; i<ucTargetByte; ++i) {
		pucParams[DXL_PACKET_TARGET_ADDR_SIZE + i] = (ucTargetParams >> (DXL_SHIFT_8_BIT * i)) & 0xFF;
	}

	this->SetHeader();
	this->SetID(ucID);
	this->SetLength(DXL_PACKET_INST_SIZE + DXL_PACKET_TARGET_ADDR_SIZE + ucTargetByte + DXL_PACKET_CRC_SIZE);
	this->SetInstruction(WRITE);
	this->SetParams(pucParams, DXL_PACKET_TARGET_ADDR_SIZE + ucTargetByte);
	uint8_t* pucWritePacket = this->pucGetPacket();

	Packet TxWritePacket(this->_ucTotalLength);
	TxWritePacket.SetPacket(pucWritePacket, this->_ucTotalLength);
	this->SetClear();
	return TxWritePacket;
}


//Transmit sync R/W ---------------------------------------------------------------------------------------------
Packet DXLProtocol::GetTxSyncReadPacket(uint8_t ucIdNum, uint8_t* pucIdList, uint8_t ucTargetAddress, uint8_t ucTargetByte){
	// Sync Read Packet의 경우, [Address 2Byte + Size 2Byte + ID 개수] 크기의 Parameter를 가진다.
	uint8_t* pucParams = new uint8_t[DXL_PACKET_TARGET_ADDR_SIZE + DXL_PACKET_TARGET_BYTE_SIZE + ucIdNum];
	pucParams[0] = ucTargetAddress & 0xFF;
	pucParams[1] = (ucTargetAddress >> DXL_SHIFT_8_BIT) & 0xFF;
	pucParams[2] = ucTargetByte & 0xFF;
	pucParams[3] = (ucTargetByte >> DXL_SHIFT_8_BIT) & 0xFF;
	memcpy(pucParams + DXL_PACKET_TARGET_ADDR_SIZE + DXL_PACKET_TARGET_BYTE_SIZE, pucIdList, ucIdNum);

	this->SetHeader();
	this->SetID(BROADCAST_ID);
	this->SetLength(DXL_READ_PACKET_LENGTH + ucIdNum);
	this->SetInstruction(SYNC_READ);
	this->SetParams(pucParams, DXL_PACKET_TARGET_ADDR_SIZE + DXL_PACKET_TARGET_BYTE_SIZE + ucIdNum);
	uint8_t* pucSyncReadPacket = this->pucGetPacket();

	Packet TxSyncReadPacket(this->_ucTotalLength);
	TxSyncReadPacket.SetPacket(pucSyncReadPacket, this->_ucTotalLength);
	this->SetClear();
	return TxSyncReadPacket;
}

Packet DXLProtocol::GetTxSyncWritePacket(uint8_t ucIdNum, int32_t* pucTargetParams, uint8_t ucTargetAddress, uint8_t ucTargetByte){
	// Sync Write Packet의 경우, [Address 2Byte + Size 2Byte + {ID 1Byte + Size 만큼의 Byte} * ID 개수] 크기의 Parameter를 가진다.
	uint8_t* pucParams = new uint8_t[DXL_PACKET_TARGET_ADDR_SIZE + DXL_PACKET_TARGET_BYTE_SIZE + ((DXL_PACKET_ID_SIZE + ucTargetByte) * ucIdNum)];
	pucParams[0] = ucTargetAddress & 0xFF;
	pucParams[1] = (ucTargetAddress >> DXL_SHIFT_8_BIT) & 0xFF;
	pucParams[2] = ucTargetByte & 0xFF;
	pucParams[3] = (ucTargetByte >> DXL_SHIFT_8_BIT) & 0xFF;
	for(size_t i=0; i<ucIdNum; ++i){
		pucParams[DXL_PACKET_TARGET_ADDR_SIZE + DXL_PACKET_TARGET_BYTE_SIZE + (i * (ucTargetByte + DXL_PACKET_ID_SIZE))] = pucTargetParams[2 * i];
		for(size_t j=0; j<ucTargetByte; ++j){
			pucParams[DXL_PACKET_TARGET_ADDR_SIZE + DXL_PACKET_TARGET_BYTE_SIZE + (i * (ucTargetByte + DXL_PACKET_ID_SIZE)) + 1 + j] = (pucTargetParams[(2 * i) + 1] >> (DXL_SHIFT_8_BIT * j)) & 0xFF;
		}
	}

	this->SetHeader();
	this->SetID(BROADCAST_ID);
	this->SetLength(DXL_PACKET_INST_SIZE + DXL_PACKET_TARGET_ADDR_SIZE + DXL_PACKET_TARGET_BYTE_SIZE + (ucTargetByte + 1) * ucIdNum + DXL_PACKET_CRC_SIZE);
	this->SetInstruction(SYNC_WRITE);
	this->SetParams(pucParams, DXL_PACKET_TARGET_ADDR_SIZE + DXL_PACKET_TARGET_BYTE_SIZE + ((ucTargetByte + DXL_PACKET_ID_SIZE) * ucIdNum));
	uint8_t* pucSyncWritePacket = this->pucGetPacket();

	Packet TxSyncWritePacket(this->_ucTotalLength);
	TxSyncWritePacket.SetPacket(pucSyncWritePacket, this->_ucTotalLength);
	this->SetClear();
	return TxSyncWritePacket;
}


// Transmit optional --------------------------------------------------------------------------------------------
//Packet DXLProtocol::GetTxRegWritePacket(){}
//Packet DXLProtocol::GetTxActionPacket(){}
//Packet DXLProtocol::GetTxFactoryResetPacket(){}
//Packet DXLProtocol::GetTxRebootPacket(){}
//Packet DXLProtocol::GetTxClearPacket(){}


// Rx Validate Interface ----------------------------------------------------------------------------------------
bool DXLProtocol::bIsCrcValidate(uint8_t* rawRxPacket, uint16_t usRxPacketLength){
	bool bIsCrcValidate = true;

	// Header Check
	if(rawRxPacket[0] != 0xFF) 	bIsCrcValidate = false;
	if(rawRxPacket[1] != 0xFF) 	bIsCrcValidate = false;
	if(rawRxPacket[2] != 0xFD) 	bIsCrcValidate = false;
	if(rawRxPacket[3] != 0x00) 	bIsCrcValidate = false;

	// Crc Check
	uint16_t usCRC = this->CRCFunc(rawRxPacket, usRxPacketLength - 2);
	if(rawRxPacket[usRxPacketLength - 2] != ((usCRC) & 0xFF)) 										bIsCrcValidate = false;
	if(rawRxPacket[usRxPacketLength - 1] != ((usCRC >> DXL_SHIFT_8_BIT) & 0xFF)) 	bIsCrcValidate = false;

	return bIsCrcValidate;
}

uint16_t DXLProtocol::CRCFunc(uint8_t *pucPacket, uint16_t ulPacketLength) {
	uint16_t ulCRCVal = 0;
	uint16_t i, j;
	uint16_t pulCRCTable[256] = {
			0x0000, 0x8005, 0x800F, 0x000A, 0x801B, 0x001E, 0x0014, 0x8011,
			0x8033, 0x0036, 0x003C, 0x8039, 0x0028, 0x802D, 0x8027, 0x0022,
			0x8063, 0x0066, 0x006C, 0x8069, 0x0078, 0x807D, 0x8077, 0x0072,
			0x0050, 0x8055, 0x805F, 0x005A, 0x804B, 0x004E, 0x0044, 0x8041,
			0x80C3, 0x00C6, 0x00CC, 0x80C9, 0x00D8, 0x80DD, 0x80D7, 0x00D2,
			0x00F0, 0x80F5, 0x80FF, 0x00FA, 0x80EB, 0x00EE, 0x00E4, 0x80E1,
			0x00A0, 0x80A5, 0x80AF, 0x00AA, 0x80BB, 0x00BE, 0x00B4, 0x80B1,
			0x8093, 0x0096, 0x009C, 0x8099, 0x0088, 0x808D, 0x8087, 0x0082,
			0x8183, 0x0186, 0x018C, 0x8189, 0x0198, 0x819D, 0x8197, 0x0192,
			0x01B0, 0x81B5, 0x81BF, 0x01BA, 0x81AB, 0x01AE, 0x01A4, 0x81A1,
			0x01E0, 0x81E5, 0x81EF, 0x01EA, 0x81FB, 0x01FE, 0x01F4, 0x81F1,
			0x81D3, 0x01D6, 0x01DC, 0x81D9, 0x01C8, 0x81CD, 0x81C7, 0x01C2,
			0x0140, 0x8145, 0x814F, 0x014A, 0x815B, 0x015E, 0x0154, 0x8151,
			0x8173, 0x0176, 0x017C, 0x8179, 0x0168, 0x816D, 0x8167, 0x0162,
			0x8123, 0x0126, 0x012C, 0x8129, 0x0138, 0x813D, 0x8137, 0x0132,
			0x0110, 0x8115, 0x811F, 0x011A, 0x810B, 0x010E, 0x0104, 0x8101,
			0x8303, 0x0306, 0x030C, 0x8309, 0x0318, 0x831D, 0x8317, 0x0312,
			0x0330, 0x8335, 0x833F, 0x033A, 0x832B, 0x032E, 0x0324, 0x8321,
			0x0360, 0x8365, 0x836F, 0x036A, 0x837B, 0x037E, 0x0374, 0x8371,
			0x8353, 0x0356, 0x035C, 0x8359, 0x0348, 0x834D, 0x8347, 0x0342,
			0x03C0, 0x83C5, 0x83CF, 0x03CA, 0x83DB, 0x03DE, 0x03D4, 0x83D1,
			0x83F3, 0x03F6, 0x03FC, 0x83F9, 0x03E8, 0x83ED, 0x83E7, 0x03E2,
			0x83A3, 0x03A6, 0x03AC, 0x83A9, 0x03B8, 0x83BD, 0x83B7, 0x03B2,
			0x0390, 0x8395, 0x839F, 0x039A, 0x838B, 0x038E, 0x0384, 0x8381,
			0x0280, 0x8285, 0x828F, 0x028A, 0x829B, 0x029E, 0x0294, 0x8291,
			0x82B3, 0x02B6, 0x02BC, 0x82B9, 0x02A8, 0x82AD, 0x82A7, 0x02A2,
			0x82E3, 0x02E6, 0x02EC, 0x82E9, 0x02F8, 0x82FD, 0x82F7, 0x02F2,
			0x02D0, 0x82D5, 0x82DF, 0x02DA, 0x82CB, 0x02CE, 0x02C4, 0x82C1,
			0x8243, 0x0246, 0x024C, 0x8249, 0x0258, 0x825D, 0x8257, 0x0252,
			0x0270, 0x8275, 0x827F, 0x027A, 0x826B, 0x026E, 0x0264, 0x8261,
			0x0220, 0x8225, 0x822F, 0x022A, 0x823B, 0x023E, 0x0234, 0x8231,
			0x8213, 0x0216, 0x021C, 0x8219, 0x0208, 0x820D, 0x8207, 0x0202
};

	for (j = 0; j < ulPacketLength; j++) {
		i = ((uint16_t) (ulCRCVal >> 8) ^ pucPacket[j]) & 0xFF;
		ulCRCVal = (ulCRCVal << 8) ^ pulCRCTable[i];
	}

	return ulCRCVal;
}
