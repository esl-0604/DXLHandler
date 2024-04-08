/*
 * DXLHandlerV2.cpp
 *
 *  Created on: Apr 8, 2024
 *      Author: eslee
 */



#include "DXLHandlerV2.h"


tDXLStatus g_tDebugStatusV2[5];


DXLHandlerV2::DXLHandlerV2(UART_HandleTypeDef* huart, uint8_t ucTotalDXLCnt) : _UARTHandler(huart), _DXLProtocol(), _ucTotalDXLCnt(ucTotalDXLCnt){
	this->_ucDxlState = DXL_IDLE;
	this->_ucParsingType = NO_INST;
	this->_ucTxIdCnt = 0;
	this->_ucRxIdCnt = 0;
}

DXLHandlerV2::~DXLHandlerV2(){
	this->SetDXLMapClear();
}


// DXL Handler Interface -----------------------------------------------------------------------------------
void DXLHandlerV2::SetDXLInit(){
	this->_ucDxlState = DXL_IDLE;
	this->_ucParsingType = NO_INST;
	this->_ucTxIdCnt = 0;
	this->_ucRxIdCnt = 0;
	this->_UARTHandler.SetRxBufferClear();
	this->_UARTHandler.SetRecieveMode();
	this->_DXLProtocol.SetClear();
}

void DXLHandlerV2::SetDXLMapInit(uint8_t* pucDXLIdList){
	for(size_t i=0; i<this->_ucTotalDXLCnt; ++i){
		this->_mDXLStatusList[pucDXLIdList[i]] = new tDXLStatus;
	}
}

void DXLHandlerV2::SetDXLMapClear(){
	for (auto& pair : this->_mDXLStatusList) {
		delete pair.second;
	}
	this->_mDXLStatusList.clear();
}

void DXLHandlerV2::SetDxlState(uint8_t ucDxlState){
	this->_ucDxlState = ucDxlState;
}

void DXLHandlerV2::ParsingRxData(){
	uint8_t* pucRxPacket = this->_UARTHandler.GetRxBuffer();
	uint8_t ucID = pucRxPacket[DXL_PACKET_ID];
	uint8_t ucERR = pucRxPacket[DXL_STATUS_PACKET_ERR];
	uint8_t* pucTargetParams = pucRxPacket + DXL_STATUS_PACKET_PARAMS;
	int32_t nDataValue = 0;


	// ERR 값 체크
	if(ucERR){
		this->_ucDxlState = DXL_RECIEVE_ERR;
		return;
	}

	// ID 존재 확인
	bool bIsIDExist = false;
	for(auto &iter : this->_mDXLStatusList) {
		if(iter.first == ucID){
			bIsIDExist = true;
		}
	}

	// ID가 등록되어 있지 않은 경우 --> DXL_REGISTER_ERR
	if(bIsIDExist == false) {
			this->_ucDxlState = DXL_REGISTER_ERR;
			return;
	}

	// ID가 등록되어 있는 경우 --> Params 값 계산 & DXLStatus 업데이트
	switch(this->_ucParsingType){
		case NO_INST:
			this->_ucDxlState = DXL_RECIEVE_ERR; return;
			break;

		case WRITE:
			break;

		case SYNC_WRITE:
			break;

		case PING:
			break;

		case OPERATING_MODE_ADDR:
			nDataValue = this->CalculateParams(pucTargetParams, DXL_DATA_1_BYTE);
			this->_mDXLStatusList[ucID]->ucOperatingMode = nDataValue;
			break;

		case HOMING_OFFSET_ADDR:
			nDataValue = this->CalculateParams(pucTargetParams, DXL_DATA_4_BYTE);
			this->_mDXLStatusList[ucID]->nHomingOffset = nDataValue;
			break;

		case CURRENT_LIMIT_ADDR:
			nDataValue = this->CalculateParams(pucTargetParams, DXL_DATA_2_BYTE);
			this->_mDXLStatusList[ucID]->nCurrentLimit = nDataValue;
			break;

		case VELOCITY_LIMIT_ADDR:
			nDataValue = this->CalculateParams(pucTargetParams, DXL_DATA_4_BYTE);
			this->_mDXLStatusList[ucID]->nVelocityLimit = nDataValue;
			break;

		case MAX_POSITION_LIMIT_ADDR:
			nDataValue = this->CalculateParams(pucTargetParams, DXL_DATA_4_BYTE);
			this->_mDXLStatusList[ucID]->nMaxPositionLimit = nDataValue;
			break;

		case MIN_POSITION_LIMIT_ADDR:
			nDataValue = this->CalculateParams(pucTargetParams, DXL_DATA_4_BYTE);
			this->_mDXLStatusList[ucID]->nMinPositionLimit = nDataValue;
			break;

		case TORQUE_ENABLE_ADDR:
			nDataValue = this->CalculateParams(pucTargetParams, DXL_DATA_1_BYTE);
			this->_mDXLStatusList[ucID]->ucTorqueEnable = nDataValue;
			break;

		case LED_ADDR:
			nDataValue = this->CalculateParams(pucTargetParams, DXL_DATA_1_BYTE);
			this->_mDXLStatusList[ucID]->ucLED = nDataValue;
			break;

		case STATUS_RETURN_LEVEL_ADDR:
			nDataValue = this->CalculateParams(pucTargetParams, DXL_DATA_1_BYTE);
			this->_mDXLStatusList[ucID]->ucStatusReturnLevel = nDataValue;
			break;

		case GOAL_CURRENT_ADDR:
			nDataValue = this->CalculateParams(pucTargetParams, DXL_DATA_2_BYTE);
			this->_mDXLStatusList[ucID]->nGoalCurrent = nDataValue;
			break;

		case GOAL_VELOCITY_ADDR:
			nDataValue = this->CalculateParams(pucTargetParams, DXL_DATA_4_BYTE);
			this->_mDXLStatusList[ucID]->nGoalVelocity = nDataValue;
			break;

		case GOAL_POSITION_ADDR:
			nDataValue = this->CalculateParams(pucTargetParams, DXL_DATA_4_BYTE);
			this->_mDXLStatusList[ucID]->nGoalPosition = nDataValue;
			break;

		case MOVING_ADDR:
			nDataValue = this->CalculateParams(pucTargetParams, DXL_DATA_1_BYTE);
			this->_mDXLStatusList[ucID]->ucMoving =  nDataValue;
			break;

		case PRESENT_CURRENT_ADDR:
			nDataValue = this->CalculateParams(pucTargetParams, DXL_DATA_2_BYTE);
			this->_mDXLStatusList[ucID]->nPresentCurrent = nDataValue;
			break;

		case PRESENT_VELOCITY_ADDR:
			nDataValue = this->CalculateParams(pucTargetParams, DXL_DATA_4_BYTE);
			this->_mDXLStatusList[ucID]->nPresentVelocity = nDataValue;
			break;

		case PRESENT_POSITION_ADDR:
			nDataValue = this->CalculateParams(pucTargetParams, DXL_DATA_4_BYTE);
			this->_mDXLStatusList[ucID]->nPresentPosition = nDataValue;
			break;
	}


	// 파싱 및 업데이트가 완료된 경우, 패킷 처리 카운트를 +1 하고, 다시 RecieveMode 활성화
	this->_ucRxIdCnt++;
	this->_UARTHandler.SetRecieveMode();



	// Debuging 코드 ---------------------------------------
	g_tDebugStatusV2[ucID] = *(this->_mDXLStatusList[ucID]);
	// Debuging 코드 ---------------------------------------



	// 전달받아야할 Status Packet을 전부 전달받은 경우
	if(this->_ucTxIdCnt == this->_ucRxIdCnt){
		this->_ucParsingType = NO_INST;
		this->_ucDxlState = DXL_CPLT;
		this->_ucTxIdCnt = 0;
		this->_ucRxIdCnt = 0;
	}

	// 전달받아야할 Status Packet이 아직 전부 오지 않은 경우
	else{
		this->_ucDxlState = DXL_RECIEVE_WAIT;
	}
}

uint8_t DXLHandlerV2::GetDxlState(){
	return this->_ucDxlState;
}

int32_t DXLHandlerV2::CalculateParams(uint8_t* pucTargetParams, uint8_t ucTargetByte){
	int32_t nParamsValue = 0;
	for(size_t i = 0; i < ucTargetByte; ++i) {
		nParamsValue |= static_cast<int32_t>(pucTargetParams[i]) << (DXL_SHIFT_8_BIT * i);
	}
	return nParamsValue;
}


// UART Interface ------------------------------------------------------------------------------------------
void DXLHandlerV2::TransmitPacketCplt(){
	this->_UARTHandler.SetRecieveMode();
	this->_ucDxlState = DXL_RECIEVE_WAIT;
}

void DXLHandlerV2::RecievePacketCplt(uint16_t usRxPacketLength){
	bool bIsValidate = this->_DXLProtocol.bIsCrcValidate( this->_UARTHandler.GetRxBuffer(), usRxPacketLength );

	// CRC 체크를 통과했다면, Body 영역 추출 & Parsing
	if(bIsValidate == true){
		this->_ucDxlState = DXL_RECIEVE_PARSING;
		this->ParsingRxData();
	}

	// CRC 체크를 통과하지 못했다면, 에러 Flag On
	else{
		this->_ucDxlState = DXL_RECIEVE_ERR;
	}
}

void DXLHandlerV2::WaitUntilCplt(){
	uint32_t nStartTick = HAL_GetTick();
	while(!(this->_ucDxlState == DXL_IDLE || this->_ucDxlState > DXL_ERR_CODE)){
		// Sync Write
		if(this->_ucParsingType == SYNC_WRITE && this->_ucDxlState == DXL_RECIEVE_WAIT){
			this->_ucParsingType = NO_INST;
			this->_ucDxlState = DXL_CPLT;
		}

		// Complete
		if(this->_ucDxlState == DXL_CPLT)	this->_ucDxlState = DXL_IDLE;

		// Timeout
		if(HAL_GetTick() - nStartTick >= DXL_TIMEOUT){
			if(this->_ucDxlState == DXL_TRANSMIT_WAIT) 	this->_ucDxlState = DXL_TRANSMIT_TIMEOUT_ERR;
			if(this->_ucDxlState == DXL_RECIEVE_WAIT) 	this->_ucDxlState = DXL_RECIEVE_TIMEOUT_ERR;
		}
	}
}

void DXLHandlerV2::TransmitAndWaitUntilCplt(Packet TxPacket){
	this->_ucDxlState = DXL_TRANSMIT_WAIT;
	this->_UARTHandler.TransmitPacket( TxPacket );
	this->WaitUntilCplt();
}


// DXL Status Interface -----------------------------------------------------------------------------------
int32_t DXLHandlerV2::GetDXLStatusByTargetAddress(uint8_t ucID, uint8_t ucTargetAddress){
	switch(ucTargetAddress){
		case OPERATING_MODE_ADDR:
			return(this->_mDXLStatusList[ucID]->ucOperatingMode);
			break;

		case HOMING_OFFSET_ADDR:
			return(this->_mDXLStatusList[ucID]->nHomingOffset);
			break;

		case CURRENT_LIMIT_ADDR:
			return(this->_mDXLStatusList[ucID]->nCurrentLimit);
			break;

		case VELOCITY_LIMIT_ADDR:
			return(this->_mDXLStatusList[ucID]->nVelocityLimit);
			break;

		case MAX_POSITION_LIMIT_ADDR:
			return(this->_mDXLStatusList[ucID]->nMaxPositionLimit);
			break;

		case MIN_POSITION_LIMIT_ADDR:
			return(this->_mDXLStatusList[ucID]->nMinPositionLimit);
			break;

		case TORQUE_ENABLE_ADDR:
			return(this->_mDXLStatusList[ucID]->ucTorqueEnable);
			break;

		case LED_ADDR:
			return(this->_mDXLStatusList[ucID]->ucLED);
			break;

		case MOVING_ADDR:
			return(this->_mDXLStatusList[ucID]->ucMoving);
			break;

		case STATUS_RETURN_LEVEL_ADDR:
			return(this->_mDXLStatusList[ucID]->ucStatusReturnLevel);
			break;

		case GOAL_CURRENT_ADDR:
			return(this->_mDXLStatusList[ucID]->nGoalCurrent);
			break;

		case GOAL_VELOCITY_ADDR:
			return(this->_mDXLStatusList[ucID]->nGoalVelocity);
			break;

		case GOAL_POSITION_ADDR:
			return(this->_mDXLStatusList[ucID]->nGoalPosition);
			break;

		case PRESENT_CURRENT_ADDR:
			return(this->_mDXLStatusList[ucID]->nPresentCurrent);
			break;

		case PRESENT_VELOCITY_ADDR:
			return(this->_mDXLStatusList[ucID]->nPresentVelocity);
			break;

		case PRESENT_POSITION_ADDR:
			return(this->_mDXLStatusList[ucID]->nPresentPosition);
			break;

		default:
			return 0;
	}
}


// DXL Ping Call -------------------------------------------------------------------------------------------
void DXLHandlerV2::SyncPing(){
	this->_ucParsingType = PING;
	this->_ucTxIdCnt = this->_ucTotalDXLCnt;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxSyncPingPacket() );
}

void DXLHandlerV2::Ping(uint8_t ucID){
	this->_ucParsingType = PING;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxPingPacket(ucID) );
}


// DXL Single Read Call ------------------------------------------------------------------------------------
int32_t DXLHandlerV2::ReadCMD_1Byte(uint8_t ucID, uint8_t ucTargetAddress){
	this->_ucParsingType = ucTargetAddress;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxReadPacket(ucID, ucTargetAddress, DXL_DATA_1_BYTE) );
	return( this->GetDXLStatusByTargetAddress(ucID, ucTargetAddress) );
}

int32_t DXLHandlerV2::ReadCMD_2Byte(uint8_t ucID, uint8_t ucTargetAddress){
	this->_ucParsingType = ucTargetAddress;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxReadPacket(ucID, ucTargetAddress, DXL_DATA_2_BYTE) );
	return( this->GetDXLStatusByTargetAddress(ucID, ucTargetAddress) );
}

int32_t DXLHandlerV2::ReadCMD_4Byte(uint8_t ucID, uint8_t ucTargetAddress){
	this->_ucParsingType = ucTargetAddress;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxReadPacket(ucID, ucTargetAddress, DXL_DATA_4_BYTE) );
	return( this->GetDXLStatusByTargetAddress(ucID, ucTargetAddress) );
}


// DXL Single Write Call ------------------------------------------------------------------------------------
void DXLHandlerV2::WriteCMD_1Byte(uint8_t ucID, uint8_t ucTargetAddress, int32_t nTargetParams){
	this->_ucParsingType = WRITE;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxWritePacket(ucID, nTargetParams, ucTargetAddress, DXL_DATA_1_BYTE) );
}

void DXLHandlerV2::WriteCMD_2Byte(uint8_t ucID, uint8_t ucTargetAddress, int32_t nTargetParams){
	this->_ucParsingType = WRITE;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxWritePacket(ucID, nTargetParams, ucTargetAddress, DXL_DATA_2_BYTE) );
}

void DXLHandlerV2::WriteCMD_4Byte(uint8_t ucID, uint8_t ucTargetAddress, int32_t nTargetParams){
	this->_ucParsingType = WRITE;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxWritePacket(ucID, nTargetParams, ucTargetAddress, DXL_DATA_4_BYTE) );
}


// DXL Sync Read Call --------------------------------------------------------------------------------------
vector<int32_t> DXLHandlerV2::SyncReadCMD_1Byte(uint8_t ucIdNum, uint8_t* pucIdList, uint8_t ucTargetAddress){
	this->_ucParsingType = ucTargetAddress;
	this->_ucTxIdCnt = ucIdNum;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxSyncReadPacket(ucIdNum, pucIdList, ucTargetAddress, DXL_DATA_1_BYTE) );

	vector<int32_t> vucReadValues(ucIdNum*2);
	for(size_t i=0; i<ucIdNum; ++i){
		vucReadValues[2*i] = pucIdList[i];
		vucReadValues[2*i + 1] = this->GetDXLStatusByTargetAddress(pucIdList[i], ucTargetAddress);
	}
	return vucReadValues;
}

vector<int32_t> DXLHandlerV2::SyncReadCMD_2Byte(uint8_t ucIdNum, uint8_t* pucIdList, uint8_t ucTargetAddress){
	this->_ucParsingType = ucTargetAddress;
	this->_ucTxIdCnt = ucIdNum;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxSyncReadPacket(ucIdNum, pucIdList, ucTargetAddress, DXL_DATA_2_BYTE) );

	vector<int32_t> vucReadValues(ucIdNum*2);
	for(size_t i=0; i<ucIdNum; ++i){
		vucReadValues[2*i] = pucIdList[i];
		vucReadValues[2*i + 1] = this->GetDXLStatusByTargetAddress(pucIdList[i], ucTargetAddress);
	}
	return vucReadValues;
}

vector<int32_t> DXLHandlerV2::SyncReadCMD_4Byte(uint8_t ucIdNum, uint8_t* pucIdList, uint8_t ucTargetAddress){
	this->_ucParsingType = ucTargetAddress;
	this->_ucTxIdCnt = ucIdNum;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxSyncReadPacket(ucIdNum, pucIdList, ucTargetAddress, DXL_DATA_4_BYTE) );

	vector<int32_t> vucReadValues(ucIdNum*2);
	for(size_t i=0; i<ucIdNum; ++i){
		vucReadValues[2*i] = pucIdList[i];
		vucReadValues[2*i + 1] = this->GetDXLStatusByTargetAddress(pucIdList[i], ucTargetAddress);
	}
	return vucReadValues;
}


// DXL Sync Write Call -------------------------------------------------------------------------------------
void DXLHandlerV2::SyncWriteCMD_1Byte (uint8_t ucIdNum, int32_t* pnTargetParams, uint8_t ucTargetAddress){
	this->_ucParsingType = SYNC_WRITE;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxSyncWritePacket(ucIdNum, pnTargetParams, ucTargetAddress, DXL_DATA_1_BYTE) );
}

void DXLHandlerV2::SyncWriteCMD_2Byte (uint8_t ucIdNum, int32_t* pnTargetParams, uint8_t ucTargetAddress){
	this->_ucParsingType = SYNC_WRITE;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxSyncWritePacket(ucIdNum, pnTargetParams, ucTargetAddress, DXL_DATA_2_BYTE) );
}

void DXLHandlerV2::SyncWriteCMD_4Byte (uint8_t ucIdNum, int32_t* pnTargetParams, uint8_t ucTargetAddress){
	this->_ucParsingType = SYNC_WRITE;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxSyncWritePacket(ucIdNum, pnTargetParams, ucTargetAddress, DXL_DATA_4_BYTE) );
}


// DXL Write_Read Call -------------------------------------------------------------------------------------
uint8_t DXLHandlerV2::WriteReadCMD_1Byte(uint8_t ucID, uint8_t ucTargetAddress, int32_t nTargetParams, uint8_t ucRetry){
	uint32_t nStartTick = HAL_GetTick();
	while(ucRetry--) {
		WriteCMD_1Byte(ucID, ucTargetAddress, nTargetParams);

		// 설정값 == 실제값
		if(nTargetParams == ReadCMD_1Byte(ucID, ucTargetAddress)){
			return DXL_WRITE_OK;
		}

		// Timeout
		if(HAL_GetTick() - nStartTick >= DXL_TIMEOUT){
			return DXL_TIMEOUT_ERR;
		}
	}
	return DXL_WRITE_ERR;
}

uint8_t DXLHandlerV2::WriteReadCMD_2Byte(uint8_t ucID, uint8_t ucTargetAddress, int32_t nTargetParams, uint8_t ucRetry){
	uint32_t nStartTick = HAL_GetTick();
	while(ucRetry--) {
		WriteCMD_2Byte(ucID, ucTargetAddress, nTargetParams);

		// 설정값 == 실제값
		if(nTargetParams == ReadCMD_2Byte(ucID, ucTargetAddress)){
			return DXL_WRITE_OK;
		}

		// Timeout
		if(HAL_GetTick() - nStartTick >= DXL_TIMEOUT){
			return DXL_TIMEOUT_ERR;
		}
	}
	return DXL_WRITE_ERR;
}

uint8_t DXLHandlerV2::WriteReadCMD_4Byte(uint8_t ucID, uint8_t ucTargetAddress, int32_t nTargetParams, uint8_t ucRetry){
	uint32_t nStartTick = HAL_GetTick();
	while(ucRetry--) {
		WriteCMD_4Byte(ucID, ucTargetAddress, nTargetParams);

		// 설정값 == 실제값
		if(nTargetParams == ReadCMD_4Byte(ucID, ucTargetAddress)){
			return DXL_WRITE_OK;
		}

		// Timeout
		if(HAL_GetTick() - nStartTick >= DXL_TIMEOUT){
			return DXL_TIMEOUT_ERR;
		}
	}
	return DXL_WRITE_ERR;
}


// DXL Sync Write_Read Call --------------------------------------------------------------------------------
uint8_t DXLHandlerV2::SyncWriteReadCMD_1Byte(uint8_t ucIdNum, uint8_t* pucIdList, int32_t* pnTargetParams, uint8_t ucTargetAddress, uint8_t ucRetry){
	uint32_t nStartTick = HAL_GetTick();
		while(ucRetry--) {
			SyncWriteCMD_1Byte(ucIdNum, pnTargetParams, ucTargetAddress);

			uint8_t ucCheckIdCnt = 0;
			for(size_t i = 0; i < ucIdNum; ++i) {
					if(pnTargetParams[2*i + 1] == SyncReadCMD_1Byte(ucIdNum, pucIdList, ucTargetAddress)[2*i + 1]) {
							ucCheckIdCnt++;
					}
			}
			// 설정값 == 실제값
			if(ucCheckIdCnt == ucIdNum) {
					return DXL_WRITE_OK;
			}

			// Timeout
			if(HAL_GetTick() - nStartTick >= DXL_TIMEOUT){
				return DXL_TIMEOUT_ERR;
			}
		}
		return DXL_WRITE_ERR;
}

uint8_t DXLHandlerV2::SyncWriteReadCMD_2Byte(uint8_t ucIdNum, uint8_t* pucIdList, int32_t* pnTargetParams, uint8_t ucTargetAddress, uint8_t ucRetry){
	uint32_t nStartTick = HAL_GetTick();
		while(ucRetry--) {
			SyncWriteCMD_2Byte(ucIdNum, pnTargetParams, ucTargetAddress);

			uint8_t ucCheckIdCnt = 0;
			for(size_t i = 0; i < ucIdNum; ++i) {
					if(pnTargetParams[2*i + 1] == SyncReadCMD_2Byte(ucIdNum, pucIdList, ucTargetAddress)[2*i + 1]) {
							ucCheckIdCnt++;
					}
			}
			// 설정값 == 실제값
			if(ucCheckIdCnt == ucIdNum) {
					return DXL_WRITE_OK;
			}

			// Timeout
			if(HAL_GetTick() - nStartTick >= DXL_TIMEOUT){
				return DXL_TIMEOUT_ERR;
			}
		}
		return DXL_WRITE_ERR;
}

uint8_t DXLHandlerV2::SyncWriteReadCMD_4Byte(uint8_t ucIdNum, uint8_t* pucIdList, int32_t* pnTargetParams, uint8_t ucTargetAddress, uint8_t ucRetry){
	uint32_t nStartTick = HAL_GetTick();
		while(ucRetry--) {
			SyncWriteCMD_4Byte(ucIdNum, pnTargetParams, ucTargetAddress);

			uint8_t ucCheckIdCnt = 0;
			for(size_t i = 0; i < ucIdNum; ++i) {
					if(pnTargetParams[2*i + 1] == SyncReadCMD_4Byte(ucIdNum, pucIdList, ucTargetAddress)[2*i + 1]) {
							ucCheckIdCnt++;
					}
			}
			// 설정값 == 실제값
			if(ucCheckIdCnt == ucIdNum) {
					return DXL_WRITE_OK;
			}

			// Timeout
			if(HAL_GetTick() - nStartTick >= DXL_TIMEOUT){
				return DXL_TIMEOUT_ERR;
			}
		}
		return DXL_WRITE_ERR;
}

