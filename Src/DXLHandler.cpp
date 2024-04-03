/*
 * DXLHandler.cpp
 *
 *  Created on: Mar 13, 2024
 *      Author: eslee
 */


#include "DXLHandler.h"


tDXLStatus g_tDebugStatus[5];


DXLHandler::DXLHandler(UART_HandleTypeDef* huart, uint8_t ucTotalDXLCnt) : _UARTHandler(huart), _DXLProtocol(), _ucTotalDXLCnt(ucTotalDXLCnt){
	this->_ucDxlState = DXL_IDLE;
	this->_ucParsingType = NO_INST;
	this->_ucTxIdCnt = 0;
	this->_ucRxIdCnt = 0;
}

DXLHandler::~DXLHandler(){
	this->SetDXLMapClear();
}


// DXL Handler Interface -----------------------------------------------------------------------------------
void DXLHandler::SetDXLInit(){
	this->_ucDxlState = DXL_IDLE;
	this->_ucParsingType = NO_INST;
	this->_ucTxIdCnt = 0;
	this->_ucRxIdCnt = 0;
	this->_UARTHandler.SetRxBufferClear();
	this->_UARTHandler.SetRecieveMode();
	this->_DXLProtocol.SetClear();
}

void DXLHandler::SetDXLMapInit(uint8_t* pucDXLIdList){
	for(size_t i=0; i<this->_ucTotalDXLCnt; ++i){
		this->_mDXLStatusList[pucDXLIdList[i]] = new tDXLStatus;
	}
}

void DXLHandler::SetDXLMapClear(){
	for (auto& pair : this->_mDXLStatusList) {
		delete pair.second;
	}
	this->_mDXLStatusList.clear();
}

void DXLHandler::SetDxlState(uint8_t ucDxlState){
	this->_ucDxlState = ucDxlState;
}

void DXLHandler::ParsingRxData(){
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
	g_tDebugStatus[ucID] = *(this->_mDXLStatusList[ucID]);
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

uint8_t DXLHandler::GetDxlState(){
	return this->_ucDxlState;
}

int32_t DXLHandler::CalculateParams(uint8_t* pucTargetParams, uint8_t ucTargetByte){
	int32_t nParamsValue = 0;
	for(size_t i = 0; i < ucTargetByte; ++i) {
		nParamsValue |= static_cast<int32_t>(pucTargetParams[i]) << (DXL_SHIFT_8_BIT * i);
	}
	return nParamsValue;
}


// UART Interface ------------------------------------------------------------------------------------------
void DXLHandler::TransmitPacketCplt(){
	this->_UARTHandler.SetRecieveMode();
	this->_ucDxlState = DXL_RECIEVE_WAIT;
}

void DXLHandler::RecievePacketCplt(uint16_t usRxPacketLength){
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

void DXLHandler::WaitUntilCplt(){
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

void DXLHandler::TransmitAndWaitUntilCplt(Packet TxPacket){
	this->_ucDxlState = DXL_TRANSMIT_WAIT;
	this->_UARTHandler.TransmitPacket( TxPacket );
	this->WaitUntilCplt();
}


// DXL Status Interface -----------------------------------------------------------------------------------
uint8_t DXLHandler::GetDXLStatusOperatingMode(uint8_t ucID){
	return(this->_mDXLStatusList[ucID]->ucOperatingMode);
}

int32_t DXLHandler::GetDXLStatusHomingOffset(uint8_t ucID){
	return(this->_mDXLStatusList[ucID]->nHomingOffset);
}

int32_t DXLHandler::GetDXLStatusCurrentLimit(uint8_t ucID){
	return(this->_mDXLStatusList[ucID]->nCurrentLimit);
}

int32_t DXLHandler::GetDXLStatusVelocityLimit(uint8_t ucID){
	return(this->_mDXLStatusList[ucID]->nVelocityLimit);
}

int32_t DXLHandler::GetDXLStatusMaxPositionLimit(uint8_t ucID){
	return(this->_mDXLStatusList[ucID]->nMaxPositionLimit);
}

int32_t DXLHandler::GetDXLStatusMinPositionLimit(uint8_t ucID){
	return(this->_mDXLStatusList[ucID]->nMinPositionLimit);
}

uint8_t DXLHandler::GetDXLStatusTorqueEnable(uint8_t ucID){
	return(this->_mDXLStatusList[ucID]->ucTorqueEnable);
}

uint8_t DXLHandler::GetDXLStatusLED(uint8_t ucID){
	return(this->_mDXLStatusList[ucID]->ucLED);
}

uint8_t DXLHandler::GetDXLStatusMoving(uint8_t ucID){
	return(this->_mDXLStatusList[ucID]->ucMoving);
}

uint8_t DXLHandler::GetDXLStatusStatusReturnLevel(uint8_t ucID){
	return(this->_mDXLStatusList[ucID]->ucStatusReturnLevel);
}

int32_t DXLHandler::GetDXLStatusGoalCurrent(uint8_t ucID){
	return(this->_mDXLStatusList[ucID]->nGoalCurrent);
}

int32_t DXLHandler::GetDXLStatusGoalVelocity(uint8_t ucID){
	return(this->_mDXLStatusList[ucID]->nGoalVelocity);
}

int32_t DXLHandler::GetDXLStatusGoalPosition(uint8_t ucID){
	return(this->_mDXLStatusList[ucID]->nGoalPosition);
}

int32_t DXLHandler::GetDXLStatusPresentCurrent(uint8_t ucID){
	return(this->_mDXLStatusList[ucID]->nPresentCurrent);
}

int32_t DXLHandler::GetDXLStatusPresentVelocity(uint8_t ucID){
	return(this->_mDXLStatusList[ucID]->nPresentVelocity);
}

int32_t DXLHandler::GetDXLStatusPresentPosition(uint8_t ucID){
	return(this->_mDXLStatusList[ucID]->nPresentPosition);
}


// DXL Ping Call -------------------------------------------------------------------------------------------
void DXLHandler::SyncPing(){
	this->_ucParsingType = PING;
	this->_ucTxIdCnt = this->_ucTotalDXLCnt;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxSyncPingPacket() );
}

void DXLHandler::Ping(uint8_t ucID){
	this->_ucParsingType = PING;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxPingPacket(ucID) );
}


// DXL Single Read Call ------------------------------------------------------------------------------------
int32_t DXLHandler::ReadOperatingMode(uint8_t ucID){
	this->_ucParsingType = OPERATING_MODE_ADDR;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxReadPacket(ucID, OPERATING_MODE_ADDR, DXL_DATA_1_BYTE) );
	return( this->GetDXLStatusOperatingMode(ucID) );
}

int32_t DXLHandler::ReadHomingOffset(uint8_t ucID){
	this->_ucParsingType = HOMING_OFFSET_ADDR;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxReadPacket(ucID, HOMING_OFFSET_ADDR, DXL_DATA_4_BYTE) );
	return( this->GetDXLStatusHomingOffset(ucID) );
}

int32_t DXLHandler::ReadCurrentLimit(uint8_t ucID){
	this->_ucParsingType = CURRENT_LIMIT_ADDR;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxReadPacket(ucID, CURRENT_LIMIT_ADDR, DXL_DATA_2_BYTE) );
	return( this->GetDXLStatusCurrentLimit(ucID) );
}

int32_t DXLHandler::ReadVelocityLimit(uint8_t ucID){
	this->_ucParsingType = VELOCITY_LIMIT_ADDR;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxReadPacket(ucID, VELOCITY_LIMIT_ADDR, DXL_DATA_4_BYTE) );
	return( this->GetDXLStatusVelocityLimit(ucID) );
}

int32_t DXLHandler::ReadMaxPositionLimit(uint8_t ucID){
	this->_ucParsingType = MAX_POSITION_LIMIT_ADDR;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxReadPacket(ucID, MAX_POSITION_LIMIT_ADDR, DXL_DATA_4_BYTE) );
	return( this->GetDXLStatusMaxPositionLimit(ucID) );
}

int32_t DXLHandler::ReadMinPositionLimit(uint8_t ucID){
	this->_ucParsingType = MIN_POSITION_LIMIT_ADDR;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxReadPacket(ucID, MIN_POSITION_LIMIT_ADDR, DXL_DATA_4_BYTE) );
	return( this->GetDXLStatusMinPositionLimit(ucID) );
}

int32_t DXLHandler::ReadTorqueEnable(uint8_t ucID){
	this->_ucParsingType = TORQUE_ENABLE_ADDR;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxReadPacket(ucID, TORQUE_ENABLE_ADDR, DXL_DATA_1_BYTE) );
	return( this->GetDXLStatusTorqueEnable(ucID) );
}

int32_t DXLHandler::ReadLED(uint8_t ucID){
	this->_ucParsingType = LED_ADDR;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxReadPacket(ucID, LED_ADDR, DXL_DATA_1_BYTE) );
	return( this->GetDXLStatusLED(ucID) );
}

int32_t DXLHandler::ReadStatusReturnLevel(uint8_t ucID){
	this->_ucParsingType = STATUS_RETURN_LEVEL_ADDR;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxReadPacket(ucID, STATUS_RETURN_LEVEL_ADDR, DXL_DATA_1_BYTE) );
	return( this->GetDXLStatusStatusReturnLevel(ucID) );
}

int32_t DXLHandler::ReadGoalCurrent(uint8_t ucID){
	this->_ucParsingType = GOAL_CURRENT_ADDR;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxReadPacket(ucID, GOAL_CURRENT_ADDR, DXL_DATA_2_BYTE) );
	return( this->GetDXLStatusGoalCurrent(ucID) );
}

int32_t DXLHandler::ReadGoalVelocity(uint8_t ucID){
	this->_ucParsingType = GOAL_VELOCITY_ADDR;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxReadPacket(ucID, GOAL_VELOCITY_ADDR, DXL_DATA_4_BYTE) );
	return( this->GetDXLStatusGoalVelocity(ucID) );
}

int32_t DXLHandler::ReadGoalPosition(uint8_t ucID){
	this->_ucParsingType = GOAL_POSITION_ADDR;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxReadPacket(ucID, GOAL_POSITION_ADDR, DXL_DATA_4_BYTE) );
	return( this->GetDXLStatusGoalPosition(ucID) );
}

int32_t DXLHandler::ReadMoving(uint8_t ucID){
	this->_ucParsingType = MOVING_ADDR;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxReadPacket(ucID, MOVING_ADDR, DXL_DATA_1_BYTE) );
	return( this->GetDXLStatusMoving(ucID) );
}

int32_t DXLHandler::ReadPresentCurrent(uint8_t ucID){
	this->_ucParsingType = PRESENT_CURRENT_ADDR;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxReadPacket(ucID, PRESENT_CURRENT_ADDR, DXL_DATA_2_BYTE) );
	return( this->GetDXLStatusPresentCurrent(ucID) );
}

int32_t DXLHandler::ReadPresentVelocity(uint8_t ucID){
	this->_ucParsingType = PRESENT_VELOCITY_ADDR;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxReadPacket(ucID, PRESENT_VELOCITY_ADDR, DXL_DATA_4_BYTE) );
	return( this->GetDXLStatusPresentVelocity(ucID) );
}

int32_t DXLHandler::ReadPresentPosition(uint8_t ucID){
	this->_ucParsingType = PRESENT_POSITION_ADDR;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxReadPacket(ucID, PRESENT_POSITION_ADDR, DXL_DATA_4_BYTE) );
	return( this->GetDXLStatusPresentPosition(ucID) );
}


// DXL Single Write Call ------------------------------------------------------------------------------------
void DXLHandler::WriteOperatingMode(uint8_t ucID, int32_t nOperatingMode){
	this->_ucParsingType = WRITE;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxWritePacket(ucID, nOperatingMode, OPERATING_MODE_ADDR, DXL_DATA_1_BYTE) );
}

void DXLHandler::WriteHomingOffset(uint8_t ucID, int32_t nHomingOffset){
	this->_ucParsingType = WRITE;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxWritePacket(ucID, nHomingOffset, HOMING_OFFSET_ADDR, DXL_DATA_4_BYTE) );
}

void DXLHandler::WriteCurrentLimit(uint8_t ucID, int32_t nCurrentLimit){
	this->_ucParsingType = WRITE;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxWritePacket(ucID, nCurrentLimit, CURRENT_LIMIT_ADDR, DXL_DATA_2_BYTE) );
}

void DXLHandler::WriteVelocityLimit(uint8_t ucID, int32_t nVelocityLimit){
	this->_ucParsingType = WRITE;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxWritePacket(ucID, nVelocityLimit, VELOCITY_LIMIT_ADDR, DXL_DATA_4_BYTE) );
}

void DXLHandler::WriteMaxPositionLimit(uint8_t ucID, int32_t nMaxPositionLimit){
	this->_ucParsingType = WRITE;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxWritePacket(ucID, nMaxPositionLimit, MAX_POSITION_LIMIT_ADDR, DXL_DATA_4_BYTE) );
}

void DXLHandler::WriteMinPositionLimit(uint8_t ucID, int32_t nMinPositionLimit){
	this->_ucParsingType = WRITE;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxWritePacket(ucID, nMinPositionLimit, MIN_POSITION_LIMIT_ADDR, DXL_DATA_4_BYTE) );
}

void DXLHandler::WriteTorqueEnable(uint8_t ucID, int32_t nTorqueEnable){
	this->_ucParsingType = WRITE;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxWritePacket(ucID, nTorqueEnable, TORQUE_ENABLE_ADDR, DXL_DATA_1_BYTE) );
}

void DXLHandler::WriteLED(uint8_t ucID, int32_t nLED){
	this->_ucParsingType = WRITE;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxWritePacket(ucID, nLED, LED_ADDR, DXL_DATA_1_BYTE) );
}

void DXLHandler::WriteStatusReturnLevel(uint8_t ucID, int32_t nStatusReturnLevel){
	this->_ucParsingType = WRITE;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxWritePacket(ucID, nStatusReturnLevel, STATUS_RETURN_LEVEL_ADDR, DXL_DATA_1_BYTE) );
}

void DXLHandler::WriteGoalCurrent(uint8_t ucID, int32_t nGoalCurrent){
	this->_ucParsingType = WRITE;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxWritePacket(ucID, nGoalCurrent, GOAL_CURRENT_ADDR, DXL_DATA_2_BYTE) );
}

void DXLHandler::WriteGoalVelocity(uint8_t ucID, int32_t nGoalVelocity){
	this->_ucParsingType = WRITE;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxWritePacket(ucID, nGoalVelocity, GOAL_VELOCITY_ADDR, DXL_DATA_4_BYTE) );
}

void DXLHandler::WriteGoalPosition(uint8_t ucID, int32_t nGoalPosition){
	this->_ucParsingType = WRITE;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxWritePacket(ucID, nGoalPosition, GOAL_POSITION_ADDR, DXL_DATA_4_BYTE) );
}


// DXL Sync Read Call --------------------------------------------------------------------------------------
vector<int32_t> DXLHandler::SyncReadOperatingMode(uint8_t ucIdNum, uint8_t* pucIdList){
	this->_ucParsingType = OPERATING_MODE_ADDR;
	this->_ucTxIdCnt = ucIdNum;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxSyncReadPacket(ucIdNum, pucIdList, OPERATING_MODE_ADDR, DXL_DATA_1_BYTE) );

	vector<int32_t> vucReadValues(ucIdNum*2);
	for(size_t i=0; i<ucIdNum; ++i){
		vucReadValues[2*i] = pucIdList[i];
		vucReadValues[2*i + 1] = this->GetDXLStatusOperatingMode(pucIdList[i]);
	}
	return vucReadValues;
}

vector<int32_t> DXLHandler::SyncReadHomingOffset(uint8_t ucIdNum, uint8_t* pucIdList){
	this->_ucParsingType = HOMING_OFFSET_ADDR;
	this->_ucTxIdCnt = ucIdNum;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxSyncReadPacket(ucIdNum, pucIdList, HOMING_OFFSET_ADDR, DXL_DATA_4_BYTE) );

	vector<int32_t> vucReadValues(ucIdNum*2);
	for(size_t i=0; i<ucIdNum; ++i){
		vucReadValues[2*i] = pucIdList[i];
		vucReadValues[2*i + 1] = this->GetDXLStatusHomingOffset(pucIdList[i]);
	}
	return vucReadValues;
}

vector<int32_t> DXLHandler::SyncReadCurrentLimit(uint8_t ucIdNum, uint8_t* pucIdList){
	this->_ucParsingType = CURRENT_LIMIT_ADDR;
	this->_ucTxIdCnt = ucIdNum;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxSyncReadPacket(ucIdNum, pucIdList, CURRENT_LIMIT_ADDR, DXL_DATA_2_BYTE) );

	vector<int32_t> vucReadValues(ucIdNum*2);
	for(size_t i=0; i<ucIdNum; ++i){
		vucReadValues[2*i] = pucIdList[i];
		vucReadValues[2*i + 1] = this->GetDXLStatusCurrentLimit(pucIdList[i]);
	}
	return vucReadValues;
}

vector<int32_t> DXLHandler::SyncReadVelocityLimit(uint8_t ucIdNum, uint8_t* pucIdList){
	this->_ucParsingType = VELOCITY_LIMIT_ADDR;
	this->_ucTxIdCnt = ucIdNum;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxSyncReadPacket(ucIdNum, pucIdList, VELOCITY_LIMIT_ADDR, DXL_DATA_4_BYTE) );

	vector<int32_t> vucReadValues(ucIdNum*2);
	for(size_t i=0; i<ucIdNum; ++i){
		vucReadValues[2*i] = pucIdList[i];
		vucReadValues[2*i + 1] = this->GetDXLStatusVelocityLimit(pucIdList[i]);
	}
	return vucReadValues;
}

vector<int32_t> DXLHandler::SyncReadMaxPositionLimit(uint8_t ucIdNum, uint8_t* pucIdList){
	this->_ucParsingType = MAX_POSITION_LIMIT_ADDR;
	this->_ucTxIdCnt = ucIdNum;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxSyncReadPacket(ucIdNum, pucIdList, MAX_POSITION_LIMIT_ADDR, DXL_DATA_4_BYTE) );

	vector<int32_t> vucReadValues(ucIdNum*2);
	for(size_t i=0; i<ucIdNum; ++i){
		vucReadValues[2*i] = pucIdList[i];
		vucReadValues[2*i + 1] = this->GetDXLStatusMaxPositionLimit(pucIdList[i]);
	}
	return vucReadValues;
}

vector<int32_t> DXLHandler::SyncReadMinPositionLimit(uint8_t ucIdNum, uint8_t* pucIdList){
	this->_ucParsingType = MIN_POSITION_LIMIT_ADDR;
	this->_ucTxIdCnt = ucIdNum;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxSyncReadPacket(ucIdNum, pucIdList, MIN_POSITION_LIMIT_ADDR, DXL_DATA_4_BYTE) );

	vector<int32_t> vucReadValues(ucIdNum*2);
	for(size_t i=0; i<ucIdNum; ++i){
		vucReadValues[2*i] = pucIdList[i];
		vucReadValues[2*i + 1] = this->GetDXLStatusMinPositionLimit(pucIdList[i]);
	}
	return vucReadValues;
}

vector<int32_t> DXLHandler::SyncReadTorqueEnable(uint8_t ucIdNum, uint8_t* pucIdList){
	this->_ucParsingType = TORQUE_ENABLE_ADDR;
	this->_ucTxIdCnt = ucIdNum;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxSyncReadPacket(ucIdNum, pucIdList, TORQUE_ENABLE_ADDR, DXL_DATA_1_BYTE) );

	vector<int32_t> vucReadValues(ucIdNum*2);
	for(size_t i=0; i<ucIdNum; ++i){
		vucReadValues[2*i] = pucIdList[i];
		vucReadValues[2*i + 1] = this->GetDXLStatusTorqueEnable(pucIdList[i]);
	}
	return vucReadValues;
}

vector<int32_t> DXLHandler::SyncReadLED(uint8_t ucIdNum, uint8_t* pucIdList){
	this->_ucParsingType = LED_ADDR;
	this->_ucTxIdCnt = ucIdNum;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxSyncReadPacket(ucIdNum, pucIdList, LED_ADDR, DXL_DATA_1_BYTE) );

	vector<int32_t> vucReadValues(ucIdNum*2);
	for(size_t i=0; i<ucIdNum; ++i){
		vucReadValues[2*i] = pucIdList[i];
		vucReadValues[2*i + 1] = this->GetDXLStatusLED(pucIdList[i]);
	}
	return vucReadValues;
}

vector<int32_t> DXLHandler::SyncReadStatusReturnLevel(uint8_t ucIdNum, uint8_t* pucIdList){
	this->_ucParsingType = STATUS_RETURN_LEVEL_ADDR;
	this->_ucTxIdCnt = ucIdNum;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxSyncReadPacket(ucIdNum, pucIdList, STATUS_RETURN_LEVEL_ADDR, DXL_DATA_1_BYTE) );

	vector<int32_t> vucReadValues(ucIdNum*2);
	for(size_t i=0; i<ucIdNum; ++i){
		vucReadValues[2*i] = pucIdList[i];
		vucReadValues[2*i + 1] = this->GetDXLStatusStatusReturnLevel(pucIdList[i]);
	}
	return vucReadValues;
}

vector<int32_t> DXLHandler::SyncReadGoalCurrent(uint8_t ucIdNum, uint8_t* pucIdList){
	this->_ucParsingType = GOAL_CURRENT_ADDR;
	this->_ucTxIdCnt = ucIdNum;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxSyncReadPacket(ucIdNum, pucIdList, GOAL_CURRENT_ADDR, DXL_DATA_2_BYTE) );

	vector<int32_t> vucReadValues(ucIdNum*2);
	for(size_t i=0; i<ucIdNum; ++i){
		vucReadValues[2*i] = pucIdList[i];
		vucReadValues[2*i + 1] = this->GetDXLStatusGoalCurrent(pucIdList[i]);
	}
	return vucReadValues;
}

vector<int32_t> DXLHandler::SyncReadGoalVelocity(uint8_t ucIdNum, uint8_t* pucIdList){
	this->_ucParsingType = GOAL_VELOCITY_ADDR;
	this->_ucTxIdCnt = ucIdNum;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxSyncReadPacket(ucIdNum, pucIdList, GOAL_VELOCITY_ADDR, DXL_DATA_4_BYTE) );

	vector<int32_t> vucReadValues(ucIdNum*2);
	for(size_t i=0; i<ucIdNum; ++i){
		vucReadValues[2*i] = pucIdList[i];
		vucReadValues[2*i + 1] = this->GetDXLStatusGoalVelocity(pucIdList[i]);
	}
	return vucReadValues;
}

vector<int32_t> DXLHandler::SyncReadGoalPosition(uint8_t ucIdNum, uint8_t* pucIdList){
	this->_ucParsingType = GOAL_POSITION_ADDR;
	this->_ucTxIdCnt = ucIdNum;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxSyncReadPacket(ucIdNum, pucIdList, GOAL_POSITION_ADDR, DXL_DATA_4_BYTE) );

	vector<int32_t> vucReadValues(ucIdNum*2);
	for(size_t i=0; i<ucIdNum; ++i){
		vucReadValues[2*i] = pucIdList[i];
		vucReadValues[2*i + 1] = this->GetDXLStatusGoalPosition(pucIdList[i]);
	}
	return vucReadValues;
}

vector<int32_t> DXLHandler::SyncReadMoving(uint8_t ucIdNum, uint8_t* pucIdList){
	this->_ucParsingType = MOVING_ADDR;
	this->_ucTxIdCnt = ucIdNum;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxSyncReadPacket(ucIdNum, pucIdList, MOVING_ADDR, DXL_DATA_1_BYTE) );

	vector<int32_t> vucReadValues(ucIdNum*2);
	for(size_t i=0; i<ucIdNum; ++i){
		vucReadValues[2*i] = pucIdList[i];
		vucReadValues[2*i + 1] = this->GetDXLStatusMoving(pucIdList[i]);
	}
	return vucReadValues;
}

vector<int32_t> DXLHandler::SyncReadPresentCurrent(uint8_t ucIdNum, uint8_t* pucIdList){
	this->_ucParsingType = PRESENT_CURRENT_ADDR;
	this->_ucTxIdCnt = ucIdNum;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxSyncReadPacket(ucIdNum, pucIdList, PRESENT_CURRENT_ADDR, DXL_DATA_2_BYTE) );

	vector<int32_t> vucReadValues(ucIdNum*2);
	for(size_t i=0; i<ucIdNum; ++i){
		vucReadValues[2*i] = pucIdList[i];
		vucReadValues[2*i + 1] = this->GetDXLStatusPresentCurrent(pucIdList[i]);
	}
	return vucReadValues;
}

vector<int32_t> DXLHandler::SyncReadPresentVelocity(uint8_t ucIdNum, uint8_t* pucIdList){
	this->_ucParsingType = PRESENT_VELOCITY_ADDR;
	this->_ucTxIdCnt = ucIdNum;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxSyncReadPacket(ucIdNum, pucIdList, PRESENT_VELOCITY_ADDR, DXL_DATA_4_BYTE) );

	vector<int32_t> vucReadValues(ucIdNum*2);
	for(size_t i=0; i<ucIdNum; ++i){
		vucReadValues[2*i] = pucIdList[i];
		vucReadValues[2*i + 1] = this->GetDXLStatusPresentVelocity(pucIdList[i]);
	}
	return vucReadValues;
}

vector<int32_t> DXLHandler::SyncReadPresentPosition(uint8_t ucIdNum, uint8_t* pucIdList){
	this->_ucParsingType = PRESENT_POSITION_ADDR;
	this->_ucTxIdCnt = ucIdNum;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxSyncReadPacket(ucIdNum, pucIdList, PRESENT_POSITION_ADDR, DXL_DATA_4_BYTE) );

	vector<int32_t> vucReadValues(ucIdNum*2);
	for(size_t i=0; i<ucIdNum; ++i){
		vucReadValues[2*i] = pucIdList[i];
		vucReadValues[2*i + 1] = this->GetDXLStatusPresentPosition(pucIdList[i]);
	}
	return vucReadValues;
}


// DXL Sync Write Call -------------------------------------------------------------------------------------
void DXLHandler::SyncWriteOperatingMode(uint8_t ucIdNum, int32_t* pnTargetParams){
	this->_ucParsingType = SYNC_WRITE;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxSyncWritePacket(ucIdNum, pnTargetParams, OPERATING_MODE_ADDR, DXL_DATA_1_BYTE) );
}

void DXLHandler::SyncWriteHomingOffset(uint8_t ucIdNum, int32_t* pnTargetParams){
	this->_ucParsingType = SYNC_WRITE;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxSyncWritePacket(ucIdNum, pnTargetParams, HOMING_OFFSET_ADDR, DXL_DATA_4_BYTE) );
}

void DXLHandler::SyncWriteCurrentLimit(uint8_t ucIdNum, int32_t* pnTargetParams){
	this->_ucParsingType = SYNC_WRITE;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxSyncWritePacket(ucIdNum, pnTargetParams, CURRENT_LIMIT_ADDR, DXL_DATA_2_BYTE) );
}

void DXLHandler::SyncWriteVelocityLimit(uint8_t ucIdNum, int32_t* pnTargetParams){
	this->_ucParsingType = SYNC_WRITE;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxSyncWritePacket(ucIdNum, pnTargetParams, VELOCITY_LIMIT_ADDR, DXL_DATA_4_BYTE) );
}

void DXLHandler::SyncWriteMaxPositionLimit(uint8_t ucIdNum, int32_t* pnTargetParams){
	this->_ucParsingType = SYNC_WRITE;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxSyncWritePacket(ucIdNum, pnTargetParams, MAX_POSITION_LIMIT_ADDR, DXL_DATA_4_BYTE) );
}

void DXLHandler::SyncWriteMinPositionLimit(uint8_t ucIdNum, int32_t* pnTargetParams){
	this->_ucParsingType = SYNC_WRITE;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxSyncWritePacket(ucIdNum, pnTargetParams, MIN_POSITION_LIMIT_ADDR, DXL_DATA_4_BYTE) );
}

void DXLHandler::SyncWriteTorqueEnable(uint8_t ucIdNum, int32_t* pnTargetParams){
	this->_ucParsingType = SYNC_WRITE;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxSyncWritePacket(ucIdNum, pnTargetParams, TORQUE_ENABLE_ADDR, DXL_DATA_1_BYTE) );
}

void DXLHandler::SyncWriteLED(uint8_t ucIdNum, int32_t* pnTargetParams){
	this->_ucParsingType = SYNC_WRITE;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxSyncWritePacket(ucIdNum, pnTargetParams, LED_ADDR, DXL_DATA_1_BYTE) );
}

void DXLHandler::SyncWriteStatusReturnLevel(uint8_t ucIdNum, int32_t* pnTargetParams){
	this->_ucParsingType = SYNC_WRITE;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxSyncWritePacket(ucIdNum, pnTargetParams, STATUS_RETURN_LEVEL_ADDR, DXL_DATA_1_BYTE) );
}

void DXLHandler::SyncWriteGoalCurrent(uint8_t ucIdNum, int32_t* pnTargetParams){
	this->_ucParsingType = SYNC_WRITE;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxSyncWritePacket(ucIdNum, pnTargetParams, GOAL_CURRENT_ADDR, DXL_DATA_2_BYTE) );
}

void DXLHandler::SyncWriteGoalVelocity(uint8_t ucIdNum, int32_t* pnTargetParams){
	this->_ucParsingType = SYNC_WRITE;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxSyncWritePacket(ucIdNum, pnTargetParams, GOAL_VELOCITY_ADDR, DXL_DATA_4_BYTE) );
}

void DXLHandler::SyncWriteGoalPosition(uint8_t ucIdNum, int32_t* pnTargetParams){
	this->_ucParsingType = SYNC_WRITE;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxSyncWritePacket(ucIdNum, pnTargetParams, GOAL_POSITION_ADDR, DXL_DATA_4_BYTE) );
}


// DXL Write_Read Call -------------------------------------------------------------------------------------
uint8_t DXLHandler::WriteReadOperatingMode(uint8_t ucID, int32_t nOperatingMode, uint8_t ucRetry){
	return this->WriteReadTemplete(ucID, nOperatingMode, ucRetry,
			[this](uint8_t ucID, int32_t nOperatingMode){ this->WriteOperatingMode(ucID, nOperatingMode); },
			[this](uint8_t ucID) -> int32_t { return this->ReadOperatingMode(ucID); }
			);
}

uint8_t DXLHandler::WriteReadHomingOffset(uint8_t ucID, int32_t nHomingOffset, uint8_t ucRetry){
	return this->WriteReadTemplete(ucID, nHomingOffset, ucRetry,
			[this](uint8_t ucID, int32_t nHomingOffset){ this->WriteHomingOffset(ucID, nHomingOffset); },
			[this](uint8_t ucID) -> int32_t { return this->ReadHomingOffset(ucID); }
			);
}

uint8_t DXLHandler::WriteReadCurrentLimit(uint8_t ucID, int32_t nCurrentLimit, uint8_t ucRetry){
	return this->WriteReadTemplete(ucID, nCurrentLimit, ucRetry,
			[this](uint8_t ucID, int32_t nCurrentLimit){ this->WriteCurrentLimit(ucID, nCurrentLimit); },
			[this](uint8_t ucID) -> int32_t { return this->ReadCurrentLimit(ucID); }
			);
}

uint8_t DXLHandler::WriteReadVelocityLimit(uint8_t ucID, int32_t nVelocityLimit, uint8_t ucRetry){
	return this->WriteReadTemplete(ucID, nVelocityLimit, ucRetry,
			[this](uint8_t ucID, int32_t nVelocityLimit){ this->WriteVelocityLimit(ucID, nVelocityLimit); },
			[this](uint8_t ucID) -> int32_t { return this->ReadVelocityLimit(ucID); }
			);
}

uint8_t DXLHandler::WriteReadMaxPositionLimit(uint8_t ucID, int32_t nMaxPositionLimit, uint8_t ucRetry){
	return this->WriteReadTemplete(ucID, nMaxPositionLimit, ucRetry,
			[this](uint8_t ucID, int32_t nMaxPositionLimit){ this->WriteMaxPositionLimit(ucID, nMaxPositionLimit); },
			[this](uint8_t ucID) -> int32_t { return this->ReadMaxPositionLimit(ucID); }
			);
}

uint8_t DXLHandler::WriteReadMinPositionLimit(uint8_t ucID, int32_t nMinPositionLimit, uint8_t ucRetry){
	return this->WriteReadTemplete(ucID, nMinPositionLimit, ucRetry,
			[this](uint8_t ucID, int32_t nMinPositionLimit){ this->WriteMinPositionLimit(ucID, nMinPositionLimit); },
			[this](uint8_t ucID) -> int32_t { return this->ReadMinPositionLimit(ucID); }
			);
}

uint8_t DXLHandler::WriteReadTorqueEnable(uint8_t ucID, int32_t nTorqueEnable, uint8_t ucRetry){
	return this->WriteReadTemplete(ucID, nTorqueEnable, ucRetry,
			[this](uint8_t ucID, int32_t nTorqueEnable){ this->WriteTorqueEnable(ucID, nTorqueEnable); },
			[this](uint8_t ucID) -> int32_t { return this->ReadTorqueEnable(ucID); }
			);
}

uint8_t DXLHandler::WriteReadLED(uint8_t ucID, int32_t nLED, uint8_t ucRetry){
	return this->WriteReadTemplete(ucID, nLED, ucRetry,
			[this](uint8_t ucID, int32_t nLED){ this->WriteLED(ucID, nLED); },
			[this](uint8_t ucID) -> int32_t { return this->ReadLED(ucID); }
			);
}

uint8_t DXLHandler::WriteReadStatusReturnLevel(uint8_t ucID, int32_t nStatusReturnLevel, uint8_t ucRetry){
	return this->WriteReadTemplete(ucID, nStatusReturnLevel, ucRetry,
			[this](uint8_t ucID, int32_t nStatusReturnLevel){ this->WriteStatusReturnLevel(ucID, nStatusReturnLevel); },
			[this](uint8_t ucID) -> int32_t { return this->ReadStatusReturnLevel(ucID); }
			);
}

uint8_t DXLHandler::WriteReadGoalCurrent(uint8_t ucID, int32_t nGoalCurrent, uint8_t ucRetry){
	return this->WriteReadTemplete(ucID, nGoalCurrent, ucRetry,
			[this](uint8_t ucID, int32_t nGoalCurrent){ this->WriteGoalCurrent(ucID, nGoalCurrent); },
			[this](uint8_t ucID) -> int32_t { return this->ReadGoalCurrent(ucID); }
			);
}

uint8_t DXLHandler::WriteReadGoalVelocity(uint8_t ucID, int32_t nGoalVelocity, uint8_t ucRetry){
	return this->WriteReadTemplete(ucID, nGoalVelocity, ucRetry,
			[this](uint8_t ucID, int32_t nGoalVelocity){ this->WriteGoalVelocity(ucID, nGoalVelocity); },
			[this](uint8_t ucID) -> int32_t { return this->ReadGoalVelocity(ucID); }
			);
}

uint8_t DXLHandler::WriteReadGoalPosition(uint8_t ucID, int32_t nGoalPosition, uint8_t ucRetry){
	return this->WriteReadTemplete(ucID, nGoalPosition, ucRetry,
			[this](uint8_t ucID, int32_t nGoalPosition){ this->WriteGoalPosition(ucID, nGoalPosition); },
			[this](uint8_t ucID) -> int32_t { return this->ReadGoalPosition(ucID); }
			);
}


// DXL Sync Write_Read Call --------------------------------------------------------------------------------
uint8_t DXLHandler::SyncWriteReadOperatingMode(uint8_t ucIdNum, uint8_t* pucIdList, int32_t* pnTargetParams, uint8_t ucRetry){
	return this->SyncWriteReadTemplete(ucIdNum, pucIdList, pnTargetParams, ucRetry,
			[this](uint8_t ucIdNum, int32_t* pnTargetParams){ this->SyncWriteOperatingMode(ucIdNum, pnTargetParams); },
			[this](uint8_t ucIdNum, uint8_t* pucIdList) -> vector<int32_t> { return this->SyncReadOperatingMode(ucIdNum, pucIdList); }
			);
}

uint8_t DXLHandler::SyncWriteReadHomingOffset(uint8_t ucIdNum, uint8_t* pucIdList, int32_t* pnTargetParams, uint8_t ucRetry){
	return this->SyncWriteReadTemplete(ucIdNum, pucIdList, pnTargetParams, ucRetry,
			[this](uint8_t ucIdNum, int32_t* pnTargetParams){ this->SyncWriteHomingOffset(ucIdNum, pnTargetParams); },
			[this](uint8_t ucIdNum, uint8_t* pucIdList) -> vector<int32_t> { return this->SyncReadHomingOffset(ucIdNum, pucIdList); }
			);
}

uint8_t DXLHandler::SyncWriteReadCurrentLimit(uint8_t ucIdNum, uint8_t* pucIdList, int32_t* pnTargetParams, uint8_t ucRetry){
	return this->SyncWriteReadTemplete(ucIdNum, pucIdList, pnTargetParams, ucRetry,
			[this](uint8_t ucIdNum, int32_t* pnTargetParams){ this->SyncWriteCurrentLimit(ucIdNum, pnTargetParams); },
			[this](uint8_t ucIdNum, uint8_t* pucIdList) -> vector<int32_t> { return this->SyncReadCurrentLimit(ucIdNum, pucIdList); }
			);
}

uint8_t DXLHandler::SyncWriteReadVelocityLimit(uint8_t ucIdNum, uint8_t* pucIdList, int32_t* pnTargetParams, uint8_t ucRetry){
	return this->SyncWriteReadTemplete(ucIdNum, pucIdList, pnTargetParams, ucRetry,
			[this](uint8_t ucIdNum, int32_t* pnTargetParams){ this->SyncWriteVelocityLimit(ucIdNum, pnTargetParams); },
			[this](uint8_t ucIdNum, uint8_t* pucIdList) -> vector<int32_t> { return this->SyncReadVelocityLimit(ucIdNum, pucIdList); }
			);
}

uint8_t DXLHandler::SyncWriteReadMaxPositionLimit(uint8_t ucIdNum, uint8_t* pucIdList, int32_t* pnTargetParams, uint8_t ucRetry){
	return this->SyncWriteReadTemplete(ucIdNum, pucIdList, pnTargetParams, ucRetry,
			[this](uint8_t ucIdNum, int32_t* pnTargetParams){ this->SyncWriteMaxPositionLimit(ucIdNum, pnTargetParams); },
			[this](uint8_t ucIdNum, uint8_t* pucIdList) -> vector<int32_t> { return this->SyncReadMaxPositionLimit(ucIdNum, pucIdList); }
			);
}

uint8_t DXLHandler::SyncWriteReadMinPositionLimit(uint8_t ucIdNum, uint8_t* pucIdList, int32_t* pnTargetParams, uint8_t ucRetry){
	return this->SyncWriteReadTemplete(ucIdNum, pucIdList, pnTargetParams, ucRetry,
			[this](uint8_t ucIdNum, int32_t* pnTargetParams){ this->SyncWriteMinPositionLimit(ucIdNum, pnTargetParams); },
			[this](uint8_t ucIdNum, uint8_t* pucIdList) -> vector<int32_t> { return this->SyncReadMinPositionLimit(ucIdNum, pucIdList); }
			);
}

uint8_t DXLHandler::SyncWriteReadTorqueEnable(uint8_t ucIdNum, uint8_t* pucIdList, int32_t* pnTargetParams, uint8_t ucRetry){
	return this->SyncWriteReadTemplete(ucIdNum, pucIdList, pnTargetParams, ucRetry,
			[this](uint8_t ucIdNum, int32_t* pnTargetParams){ this->SyncWriteTorqueEnable(ucIdNum, pnTargetParams); },
			[this](uint8_t ucIdNum, uint8_t* pucIdList) -> vector<int32_t> { return this->SyncReadTorqueEnable(ucIdNum, pucIdList); }
			);
}

uint8_t DXLHandler::SyncWriteReadLED(uint8_t ucIdNum, uint8_t* pucIdList, int32_t* pnTargetParams, uint8_t ucRetry){
	return this->SyncWriteReadTemplete(ucIdNum, pucIdList, pnTargetParams, ucRetry,
			[this](uint8_t ucIdNum, int32_t* pnTargetParams){ this->SyncWriteLED(ucIdNum, pnTargetParams); },
			[this](uint8_t ucIdNum, uint8_t* pucIdList) -> vector<int32_t> { return this->SyncReadLED(ucIdNum, pucIdList); }
			);
}

uint8_t DXLHandler::SyncWriteReadStatusReturnLevel(uint8_t ucIdNum, uint8_t* pucIdList, int32_t* pnTargetParams, uint8_t ucRetry){
	return this->SyncWriteReadTemplete(ucIdNum, pucIdList, pnTargetParams, ucRetry,
			[this](uint8_t ucIdNum, int32_t* pnTargetParams){ this->SyncWriteStatusReturnLevel(ucIdNum, pnTargetParams); },
			[this](uint8_t ucIdNum, uint8_t* pucIdList) -> vector<int32_t> { return this->SyncReadStatusReturnLevel(ucIdNum, pucIdList); }
			);
}

uint8_t DXLHandler::SyncWriteReadGoalCurrent(uint8_t ucIdNum, uint8_t* pucIdList, int32_t* pnTargetParams, uint8_t ucRetry){
	return this->SyncWriteReadTemplete(ucIdNum, pucIdList, pnTargetParams, ucRetry,
			[this](uint8_t ucIdNum, int32_t* pnTargetParams){ this->SyncWriteGoalCurrent(ucIdNum, pnTargetParams); },
			[this](uint8_t ucIdNum, uint8_t* pucIdList) -> vector<int32_t> { return this->SyncReadGoalCurrent(ucIdNum, pucIdList); }
			);
}

uint8_t DXLHandler::SyncWriteReadGoalVelocity(uint8_t ucIdNum, uint8_t* pucIdList, int32_t* pnTargetParams, uint8_t ucRetry){
	return this->SyncWriteReadTemplete(ucIdNum, pucIdList, pnTargetParams, ucRetry,
			[this](uint8_t ucIdNum, int32_t* pnTargetParams){ this->SyncWriteGoalVelocity(ucIdNum, pnTargetParams); },
			[this](uint8_t ucIdNum, uint8_t* pucIdList) -> vector<int32_t> { return this->SyncReadGoalVelocity(ucIdNum, pucIdList); }
			);
}

uint8_t DXLHandler::SyncWriteReadGoalPosition(uint8_t ucIdNum, uint8_t* pucIdList, int32_t* pnTargetParams, uint8_t ucRetry){
	return this->SyncWriteReadTemplete(ucIdNum, pucIdList, pnTargetParams, ucRetry,
			[this](uint8_t ucIdNum, int32_t* pnTargetParams){ this->SyncWriteGoalPosition(ucIdNum, pnTargetParams); },
			[this](uint8_t ucIdNum, uint8_t* pucIdList) -> vector<int32_t> { return this->SyncReadGoalPosition(ucIdNum, pucIdList); }
			);
}


// DXL Call Templete ------------------------------------------------------------------------------------
template<typename WriteFunction, typename ReadFunction>
uint8_t DXLHandler::WriteReadTemplete (uint8_t ucID, int32_t nTargetParams, uint8_t ucRetry, WriteFunction WriteFunc, ReadFunction ReadFunc){

	uint32_t nStartTick = HAL_GetTick();
	while(ucRetry--) {
		WriteFunc(ucID, nTargetParams);

		// 설정값 == 실제값
		if(nTargetParams == ReadFunc(ucID)){
			return DXL_WRITE_OK;
		}

		// Timeout
		if(HAL_GetTick() - nStartTick >= DXL_TIMEOUT){
			return DXL_TIMEOUT_ERR;
		}
	}
	return DXL_WRITE_ERR;
}

template<typename SyncWriteFunction, typename SyncReadFunction>
uint8_t DXLHandler::SyncWriteReadTemplete (uint8_t ucIdNum, uint8_t* pucIdList, int32_t* pnTargetParams, uint8_t ucRetry, SyncWriteFunction SyncWriteFunc, SyncReadFunction SyncReadFunc){

	uint32_t nStartTick = HAL_GetTick();
	while(ucRetry--) {
		SyncWriteFunc(ucIdNum, pnTargetParams);

		uint8_t ucCheckIdCnt = 0;
		for(size_t i = 0; i < ucIdNum; ++i) {
				if(pnTargetParams[2*i + 1] == SyncReadFunc(ucIdNum, pucIdList)[2*i + 1]) {
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
