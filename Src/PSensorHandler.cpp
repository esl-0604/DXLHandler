/*
 * PSensorHandler.cpp
 *
 *  Created on: Mar 27, 2024
 *      Author: eslee
 */



#include <PSensorHandler.h>

tPSensorStatus g_tDebugPSStatus[5];


PSensorHandler::PSensorHandler(uint8_t ucTotalPSensorCnt) : _ucTotalPSensorCnt(ucTotalPSensorCnt){
}

PSensorHandler::~PSensorHandler(){
	this->SetPSensorMapClear();
}


// PSensor Status Interface -----------------------------------------------------------------------
uint8_t PSensorHandler::GetPSensorStatusSensorFlag(uint8_t ucID){
	return(this->_mPSensorStatusList[ucID]->ucPSensorDetectFlag);
}

uint8_t PSensorHandler::GetPSensorStatusHomingState(uint8_t ucID){
	return(this->_mPSensorStatusList[ucID]->ucPSensorHomingState);
}

void PSensorHandler::SetPSensorStatusSensorFlag(uint8_t ucID, uint8_t ucFlag){
	this->_mPSensorStatusList[ucID]->ucPSensorDetectFlag = ucFlag;\


	// Debuging 코드 ---------------------------------------
	g_tDebugPSStatus[ucID] = *(this->_mPSensorStatusList[ucID]);
	// Debuging 코드 ---------------------------------------
}

void PSensorHandler::SetPSensorStatusHomingState(uint8_t ucID, uint8_t ucHomingState){
	this->_mPSensorStatusList[ucID]->ucPSensorHomingState = ucHomingState;


	// Debuging 코드 ---------------------------------------
	g_tDebugPSStatus[ucID] = *(this->_mPSensorStatusList[ucID]);
	// Debuging 코드 ---------------------------------------
}

void PSensorHandler::SetPSensorStatusHomingStateReset(){
	for(auto &iter : this->_mPSensorStatusList) {
		this->SetPSensorStatusHomingState(iter.first, HOMING_START);
	}
}

// PSensor Handler Interface ----------------------------------------------------------------------
void PSensorHandler::SetPSensorMapInit(uint8_t* pucIdList){
	for(size_t i=0; i<this->_ucTotalPSensorCnt; ++i){
		this->_mPSensorStatusList[pucIdList[i]] = new tPSensorStatus;
	}
}

void PSensorHandler::SetPSensorMapClear(){
	for (auto& pair : this->_mPSensorStatusList) {
		delete pair.second;
	}
	this->_mPSensorStatusList.clear();
}
