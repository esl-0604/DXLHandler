/*
 * DXLHandler.cpp
 *
 *  Created on: Mar 13, 2024
 *      Author: eslee
 */


#include "DXLHandler.h"


//	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
//	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
tDXLStatus g_tDebugStatus[5];
uint32_t g_StartTick = 0;
uint32_t g_EndTick = 0;

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
uint8_t DXLHandler::GetDXLStatusOperatingMode(uint8_t ucID){
	return(this->_mDXLStatusList[ucID]->ucOperatingMode);
}

int32_t DXLHandler::GetDXLStatusHomingOffset(uint8_t ucID){
	return(this->_mDXLStatusList[ucID]->nHomingOffset);
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


// DXL Handler Interface -----------------------------------------------------------------------------------
void DXLHandler::SetDXLMapInit(uint8_t* pucIdList){
	for(size_t i=0; i<this->_ucTotalDXLCnt; ++i){
		this->_mDXLStatusList[pucIdList[i]] = new tDXLStatus;
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
	uint8_t* ucRxPacket = this->_UARTHandler.GetRxBuffer();
	uint8_t ucID = ucRxPacket[DXL_PACKET_ID];
	uint8_t ucERR = ucRxPacket[DXL_STATUS_PACKET_ERR];
	uint8_t* pucTargetParams = ucRxPacket + DXL_STATUS_PACKET_PARAMS;
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
	else{
		switch(this->_ucParsingType){
			case NO_INST:
				this->_ucDxlState = DXL_RECIEVE_ERR; return;
				break;

			case P_WRITE:
				break;

			case P_PING:
				break;

			case P_OPERATING_MODE:
				nDataValue = this->CalculateParams(pucTargetParams, DXL_DATA_1_BYTE);
				this->_mDXLStatusList[ucID]->ucOperatingMode = nDataValue;
				break;

			case P_HOMING_OFFSET:
				nDataValue = this->CalculateParams(pucTargetParams, DXL_DATA_4_BYTE);
				this->_mDXLStatusList[ucID]->nHomingOffset = nDataValue;
				break;

			case P_CURRENT_LIMIT:
				nDataValue = this->CalculateParams(pucTargetParams, DXL_DATA_2_BYTE);
				this->_mDXLStatusList[ucID]->nCurrentLimit = nDataValue;
				break;

			case P_VELOCITY_LIMIT:
				nDataValue = this->CalculateParams(pucTargetParams, DXL_DATA_4_BYTE);
				this->_mDXLStatusList[ucID]->nVelocityLimit = nDataValue;
				break;

			case P_MAX_POSITION_LIMIT:
				nDataValue = this->CalculateParams(pucTargetParams, DXL_DATA_4_BYTE);
				this->_mDXLStatusList[ucID]->nMaxPositionLimit = nDataValue;
				break;

			case P_MIN_POSITION_LIMIT:
				nDataValue = this->CalculateParams(pucTargetParams, DXL_DATA_4_BYTE);
				this->_mDXLStatusList[ucID]->nMinPositionLimit = nDataValue;
				break;

			case P_TORQUE_ENABLE:
				nDataValue = this->CalculateParams(pucTargetParams, DXL_DATA_1_BYTE);
				this->_mDXLStatusList[ucID]->ucTorqueEnable = nDataValue;
				break;

			case P_LED:
				nDataValue = this->CalculateParams(pucTargetParams, DXL_DATA_1_BYTE);
				this->_mDXLStatusList[ucID]->ucLED = nDataValue;
				break;

			case P_GOAL_CURRENT:
				nDataValue = this->CalculateParams(pucTargetParams, DXL_DATA_2_BYTE);
				this->_mDXLStatusList[ucID]->nGoalCurrent = nDataValue;
				break;

			case P_GOAL_VELOCITY:
				nDataValue = this->CalculateParams(pucTargetParams, DXL_DATA_4_BYTE);
				this->_mDXLStatusList[ucID]->nGoalVelocity = nDataValue;
				break;

			case P_GOAL_POSITION:
				nDataValue = this->CalculateParams(pucTargetParams, DXL_DATA_4_BYTE);
				this->_mDXLStatusList[ucID]->nGoalPosition = nDataValue;
				break;

			case P_MOVING:
				nDataValue = this->CalculateParams(pucTargetParams, DXL_DATA_1_BYTE);
				this->_mDXLStatusList[ucID]->ucMoving =  nDataValue;
				break;

			case P_PRESENT_CURRENT:
				nDataValue = this->CalculateParams(pucTargetParams, DXL_DATA_2_BYTE);
				this->_mDXLStatusList[ucID]->nPresentCurrent = nDataValue;
				break;

			case P_PRESENT_VELOCITY:
				nDataValue = this->CalculateParams(pucTargetParams, DXL_DATA_4_BYTE);
				this->_mDXLStatusList[ucID]->nPresentVelocity = nDataValue;
				break;

			case P_PRESENT_POSITION:
				nDataValue = this->CalculateParams(pucTargetParams, DXL_DATA_4_BYTE);
				this->_mDXLStatusList[ucID]->nPresentPosition = nDataValue;
				break;
		}
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
	while(!(this->_ucDxlState == DXL_IDLE || this->_ucDxlState == DXL_TRANSMIT_ERR || this->_ucDxlState == DXL_RECIEVE_ERR || this->_ucDxlState == DXL_REGISTER_ERR)){
		// Sync Write
		if(this->_ucParsingType == P_SYNC_WRITE && this->_ucDxlState == DXL_RECIEVE_WAIT){
			this->_ucParsingType = NO_INST;
			this->_ucDxlState = DXL_CPLT;
		}

		// Cplt
		if(this->_ucDxlState == DXL_CPLT)	this->_ucDxlState = DXL_IDLE;

		// Timeout
		if(HAL_GetTick() - nStartTick >= DXL_TIMEOUT){
			if(this->_ucDxlState == DXL_TRANSMIT_WAIT) 	this->_ucDxlState = DXL_TRANSMIT_ERR;
			if(this->_ucDxlState == DXL_RECIEVE_WAIT) 	this->_ucDxlState = DXL_RECIEVE_ERR;
		}
	}
}

void DXLHandler::TransmitAndWaitUntilCplt(Packet TxPacket){
	this->_ucDxlState = DXL_TRANSMIT_WAIT;
	this->_UARTHandler.TransmitPacket( TxPacket );
	this->WaitUntilCplt();
}


// DXL Ping Call -------------------------------------------------------------------------------------------
void DXLHandler::SyncPing(){
	this->_ucParsingType = P_PING;
	this->_ucTxIdCnt = this->_ucTotalDXLCnt;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxSyncPingPacket() );
}

void DXLHandler::Ping(uint8_t ucID){
	this->_ucParsingType = P_PING;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxPingPacket(ucID) );
}


// DXL Single Read Call ------------------------------------------------------------------------------------
void DXLHandler::ReadOperatingMode(uint8_t ucID){
	this->_ucParsingType = P_OPERATING_MODE;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxReadPacket(ucID, OPERATING_MODE_ADDR, DXL_DATA_1_BYTE) );
}

void DXLHandler::ReadHomingOffset(uint8_t ucID){
	this->_ucParsingType = P_HOMING_OFFSET;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxReadPacket(ucID, HOMING_OFFSET_ADDR, DXL_DATA_4_BYTE) );
}

void DXLHandler::ReadCurrentLimit(uint8_t ucID){
	this->_ucParsingType = P_CURRENT_LIMIT;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxReadPacket(ucID, CURRENT_LIMIT_ADDR, DXL_DATA_2_BYTE) );
}

void DXLHandler::ReadVelocityLimit(uint8_t ucID){
	this->_ucParsingType = P_VELOCITY_LIMIT;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxReadPacket(ucID, VELOCITY_LIMIT_ADDR, DXL_DATA_4_BYTE) );
}

void DXLHandler::ReadMaxPositionLimit(uint8_t ucID){
	this->_ucParsingType = P_MAX_POSITION_LIMIT;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxReadPacket(ucID, MAX_POSITION_LIMIT_ADDR, DXL_DATA_4_BYTE) );
}

void DXLHandler::ReadMinPositionLimit(uint8_t ucID){
	this->_ucParsingType = P_MIN_POSITION_LIMIT;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxReadPacket(ucID, MIN_POSITION_LIMIT_ADDR, DXL_DATA_4_BYTE) );
}

void DXLHandler::ReadTorqueEnable(uint8_t ucID){
	this->_ucParsingType = P_TORQUE_ENABLE;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxReadPacket(ucID, TORQUE_ENABLE_ADDR, DXL_DATA_1_BYTE) );
}

void DXLHandler::ReadLED(uint8_t ucID){
	this->_ucParsingType = P_LED;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxReadPacket(ucID, LED_ADDR, DXL_DATA_1_BYTE) );
}

void DXLHandler::ReadGoalCurrent(uint8_t ucID){
	this->_ucParsingType = P_GOAL_CURRENT;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxReadPacket(ucID, GOAL_CURRENT_ADDR, DXL_DATA_2_BYTE) );
}

void DXLHandler::ReadGoalVelocity(uint8_t ucID){
	this->_ucParsingType = P_GOAL_VELOCITY;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxReadPacket(ucID, GOAL_VELOCITY_ADDR, DXL_DATA_4_BYTE) );
}

void DXLHandler::ReadGoalPosition(uint8_t ucID){
	this->_ucParsingType = P_GOAL_POSITION;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxReadPacket(ucID, GOAL_POSITION_ADDR, DXL_DATA_4_BYTE) );
}

void DXLHandler::ReadMoving(uint8_t ucID){
	this->_ucParsingType = P_MOVING;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxReadPacket(ucID, MOVING_ADDR, DXL_DATA_1_BYTE) );
}

void DXLHandler::ReadPresentCurrent(uint8_t ucID){
	this->_ucParsingType = P_PRESENT_CURRENT;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxReadPacket(ucID, PRESENT_CURRENT_ADDR, DXL_DATA_2_BYTE) );
}

void DXLHandler::ReadPresentVelocity(uint8_t ucID){
	this->_ucParsingType = P_PRESENT_VELOCITY;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxReadPacket(ucID, PRESENT_VELOCITY_ADDR, DXL_DATA_4_BYTE) );
}
void DXLHandler::ReadPresentPosition(uint8_t ucID){
	this->_ucParsingType = P_PRESENT_POSITION;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxReadPacket(ucID, PRESENT_POSITION_ADDR, DXL_DATA_4_BYTE) );
}


// DXL Single Write Call ------------------------------------------------------------------------------------
void DXLHandler::WriteOperatingMode(uint8_t ucID, int32_t nOperatingMode){
	this->_ucParsingType = P_WRITE;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxWritePacket(ucID, nOperatingMode, OPERATING_MODE_ADDR, DXL_DATA_1_BYTE) );
}

void DXLHandler::WriteHomingOffset(uint8_t ucID, int32_t nHomingOffset){
	this->_ucParsingType = P_WRITE;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxWritePacket(ucID, nHomingOffset, HOMING_OFFSET_ADDR, DXL_DATA_4_BYTE) );
}

void DXLHandler::WriteCurrentLimit(uint8_t ucID, int32_t nCurrentLimit){
	this->_ucParsingType = P_WRITE;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxWritePacket(ucID, nCurrentLimit, CURRENT_LIMIT_ADDR, DXL_DATA_2_BYTE) );
}

void DXLHandler::WriteVelocityLimit(uint8_t ucID, int32_t nVelocityLimit){
	this->_ucParsingType = P_WRITE;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxWritePacket(ucID, nVelocityLimit, VELOCITY_LIMIT_ADDR, DXL_DATA_4_BYTE) );
}

void DXLHandler::WriteMaxPositionLimit(uint8_t ucID, int32_t nMaxPositionLimit){
	this->_ucParsingType = P_WRITE;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxWritePacket(ucID, nMaxPositionLimit, MAX_POSITION_LIMIT_ADDR, DXL_DATA_4_BYTE) );
}

void DXLHandler::WriteMinPositionLimit(uint8_t ucID, int32_t nMinPositionLimit){
	this->_ucParsingType = P_WRITE;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxWritePacket(ucID, nMinPositionLimit, MIN_POSITION_LIMIT_ADDR, DXL_DATA_4_BYTE) );
}

void DXLHandler::WriteTorqueEnableON(uint8_t ucID){
	this->_ucParsingType = P_WRITE;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxWritePacket(ucID, DXL_ENABLE, TORQUE_ENABLE_ADDR, DXL_DATA_1_BYTE) );
}

void DXLHandler::WriteTorqueEnableOFF(uint8_t ucID){
	this->_ucParsingType = P_WRITE;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxWritePacket(ucID, DXL_DISABLE, TORQUE_ENABLE_ADDR, DXL_DATA_1_BYTE) );
}

void DXLHandler::WriteLEDOn(uint8_t ucID){
	this->_ucParsingType = P_WRITE;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxWritePacket(ucID, DXL_ENABLE, LED_ADDR, DXL_DATA_1_BYTE) );
}

void DXLHandler::WriteLEDOff(uint8_t ucID){
	this->_ucParsingType = P_WRITE;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxWritePacket(ucID, DXL_DISABLE, LED_ADDR, DXL_DATA_1_BYTE) );
}

void DXLHandler::WriteGoalCurrent(uint8_t ucID, int32_t nGoalCurrent){
	this->_ucParsingType = P_WRITE;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxWritePacket(ucID, nGoalCurrent, GOAL_CURRENT_ADDR, DXL_DATA_2_BYTE) );
}

void DXLHandler::WriteGoalVelocity(uint8_t ucID, int32_t nGoalVelocity){
	this->_ucParsingType = P_WRITE;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxWritePacket(ucID, nGoalVelocity, GOAL_VELOCITY_ADDR, DXL_DATA_4_BYTE) );
}

void DXLHandler::WriteGoalPosition(uint8_t ucID, int32_t nGoalPosition){
	this->_ucParsingType = P_WRITE;
	this->_ucTxIdCnt = 1;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxWritePacket(ucID, nGoalPosition, GOAL_POSITION_ADDR, DXL_DATA_4_BYTE) );
}


// DXL Sync Read Call --------------------------------------------------------------------------------------
void DXLHandler::SyncReadOperatingMode(uint8_t ucIdNum, uint8_t* pucIdList){
	this->_ucParsingType = P_OPERATING_MODE;
	this->_ucTxIdCnt = ucIdNum;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxSyncReadPacket(ucIdNum, pucIdList, OPERATING_MODE_ADDR, DXL_DATA_1_BYTE) );
}

void DXLHandler::SyncReadHomingOffset(uint8_t ucIdNum, uint8_t* pucIdList){
	this->_ucParsingType = P_HOMING_OFFSET;
	this->_ucTxIdCnt = ucIdNum;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxSyncReadPacket(ucIdNum, pucIdList, HOMING_OFFSET_ADDR, DXL_DATA_4_BYTE) );
}

void DXLHandler::SyncReadCurrentLimit(uint8_t ucIdNum, uint8_t* pucIdList){
	this->_ucParsingType = P_CURRENT_LIMIT;
	this->_ucTxIdCnt = ucIdNum;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxSyncReadPacket(ucIdNum, pucIdList, CURRENT_LIMIT_ADDR, DXL_DATA_2_BYTE) );
}

void DXLHandler::SyncReadVelocityLimit(uint8_t ucIdNum, uint8_t* pucIdList){
	this->_ucParsingType = P_VELOCITY_LIMIT;
	this->_ucTxIdCnt = ucIdNum;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxSyncReadPacket(ucIdNum, pucIdList, VELOCITY_LIMIT_ADDR, DXL_DATA_4_BYTE) );
}

void DXLHandler::SyncReadMaxPositionLimit(uint8_t ucIdNum, uint8_t* pucIdList){
	this->_ucParsingType = P_MAX_POSITION_LIMIT;
	this->_ucTxIdCnt = ucIdNum;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxSyncReadPacket(ucIdNum, pucIdList, MAX_POSITION_LIMIT_ADDR, DXL_DATA_4_BYTE) );
}

void DXLHandler::SyncReadMinPositionLimit(uint8_t ucIdNum, uint8_t* pucIdList){
	this->_ucParsingType = P_MIN_POSITION_LIMIT;
	this->_ucTxIdCnt = ucIdNum;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxSyncReadPacket(ucIdNum, pucIdList, MIN_POSITION_LIMIT_ADDR, DXL_DATA_4_BYTE) );
}

void DXLHandler::SyncReadTorqueEnable(uint8_t ucIdNum, uint8_t* pucIdList){
	this->_ucParsingType = P_TORQUE_ENABLE;
	this->_ucTxIdCnt = ucIdNum;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxSyncReadPacket(ucIdNum, pucIdList, TORQUE_ENABLE_ADDR, DXL_DATA_1_BYTE) );
}

void DXLHandler::SyncReadLED(uint8_t ucIdNum, uint8_t* pucIdList){
	this->_ucParsingType = P_LED;
	this->_ucTxIdCnt = ucIdNum;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxSyncReadPacket(ucIdNum, pucIdList, LED_ADDR, DXL_DATA_1_BYTE) );
}

void DXLHandler::SyncReadGoalPosition(uint8_t ucIdNum, uint8_t* pucIdList){
	this->_ucParsingType = P_GOAL_POSITION;
	this->_ucTxIdCnt = ucIdNum;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxSyncReadPacket(ucIdNum, pucIdList, GOAL_POSITION_ADDR, DXL_DATA_4_BYTE) );
}

void DXLHandler::SyncReadMoving(uint8_t ucIdNum, uint8_t* pucIdList){
	this->_ucParsingType = P_MOVING;
	this->_ucTxIdCnt = ucIdNum;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxSyncReadPacket(ucIdNum, pucIdList, MOVING_ADDR, DXL_DATA_1_BYTE) );
}

void DXLHandler::SyncReadPresentCurrent(uint8_t ucIdNum, uint8_t* pucIdList){
	this->_ucParsingType = P_PRESENT_CURRENT;
	this->_ucTxIdCnt = ucIdNum;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxSyncReadPacket(ucIdNum, pucIdList, PRESENT_CURRENT_ADDR, DXL_DATA_2_BYTE) );
}

void DXLHandler::SyncReadPresentVelocity(uint8_t ucIdNum, uint8_t* pucIdList){
	this->_ucParsingType = P_PRESENT_VELOCITY;
	this->_ucTxIdCnt = ucIdNum;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxSyncReadPacket(ucIdNum, pucIdList, PRESENT_VELOCITY_ADDR, DXL_DATA_4_BYTE) );
}

void DXLHandler::SyncReadPresentPosition(uint8_t ucIdNum, uint8_t* pucIdList){
	this->_ucParsingType = P_PRESENT_POSITION;
	this->_ucTxIdCnt = ucIdNum;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxSyncReadPacket(ucIdNum, pucIdList, PRESENT_POSITION_ADDR, DXL_DATA_4_BYTE) );
}


// DXL Sync Write Call -------------------------------------------------------------------------------------
void DXLHandler::SyncWriteOperatingMode(uint8_t ucIdNum, int32_t* pnTargetParams){
	this->_ucParsingType = P_SYNC_WRITE;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxSyncWritePacket(ucIdNum, pnTargetParams, OPERATING_MODE_ADDR, DXL_DATA_1_BYTE) );
}

void DXLHandler::SyncWriteHomingOffset(uint8_t ucIdNum, int32_t* pnTargetParams){
	this->_ucParsingType = P_SYNC_WRITE;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxSyncWritePacket(ucIdNum, pnTargetParams, HOMING_OFFSET_ADDR, DXL_DATA_4_BYTE) );
}

void DXLHandler::SyncWriteCurrentLimit(uint8_t ucIdNum, int32_t* pnTargetParams){
	this->_ucParsingType = P_SYNC_WRITE;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxSyncWritePacket(ucIdNum, pnTargetParams, CURRENT_LIMIT_ADDR, DXL_DATA_2_BYTE) );
}

void DXLHandler::SyncWriteVelocityLimit(uint8_t ucIdNum, int32_t* pnTargetParams){
	this->_ucParsingType = P_SYNC_WRITE;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxSyncWritePacket(ucIdNum, pnTargetParams, VELOCITY_LIMIT_ADDR, DXL_DATA_4_BYTE) );
}

void DXLHandler::SyncWriteMaxPositionLimit(uint8_t ucIdNum, int32_t* pnTargetParams){
	this->_ucParsingType = P_SYNC_WRITE;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxSyncWritePacket(ucIdNum, pnTargetParams, MAX_POSITION_LIMIT_ADDR, DXL_DATA_4_BYTE) );
}

void DXLHandler::SyncWriteMinPositionLimit(uint8_t ucIdNum, int32_t* pnTargetParams){
	this->_ucParsingType = P_SYNC_WRITE;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxSyncWritePacket(ucIdNum, pnTargetParams, MIN_POSITION_LIMIT_ADDR, DXL_DATA_4_BYTE) );
}

void DXLHandler::SyncWriteTorqueEnable(uint8_t ucIdNum, int32_t* pnTargetParams){
	this->_ucParsingType = P_SYNC_WRITE;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxSyncWritePacket(ucIdNum, pnTargetParams, TORQUE_ENABLE_ADDR, DXL_DATA_1_BYTE) );
}

void DXLHandler::SyncWriteLED(uint8_t ucIdNum, int32_t* pnTargetParams){
	this->_ucParsingType = P_SYNC_WRITE;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxSyncWritePacket(ucIdNum, pnTargetParams, LED_ADDR, DXL_DATA_1_BYTE) );
}

void DXLHandler::SyncWriteGoalCurrent(uint8_t ucIdNum, int32_t* pnTargetParams){
	this->_ucParsingType = P_SYNC_WRITE;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxSyncWritePacket(ucIdNum, pnTargetParams, GOAL_CURRENT_ADDR, DXL_DATA_2_BYTE) );
}

void DXLHandler::SyncWriteGoalVelocity(uint8_t ucIdNum, int32_t* pnTargetParams){
	this->_ucParsingType = P_SYNC_WRITE;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxSyncWritePacket(ucIdNum, pnTargetParams, GOAL_VELOCITY_ADDR, DXL_DATA_4_BYTE) );
}

void DXLHandler::SyncWriteGoalPosition(uint8_t ucIdNum, int32_t* pnTargetParams){
	this->_ucParsingType = P_SYNC_WRITE;
	this->TransmitAndWaitUntilCplt( this->_DXLProtocol.GetTxSyncWritePacket(ucIdNum, pnTargetParams, GOAL_POSITION_ADDR, DXL_DATA_4_BYTE) );
}

