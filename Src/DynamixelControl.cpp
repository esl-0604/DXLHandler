
#include "DynamixelControl.h"

DynamixelControl::DynamixelControl(){
  this->_ucToqueStatFlag = TORQUE_STATUS_OFF;
  this->_bEStopFlag = false;
  this->_sCurCurrent = 0;
  this->_ucOperationMode = 3;
  this->_ucDriveMode = 0;
  this->_ucReturnLev = 2;
  this->_ucSyncIdNum = 0;
  this->_ucPeriodTarget = PERIOD_NONE;
  this->_iTargetPos = 0;
  this->_iCurPos = 0;
  this->_iPreHomingOffset = 0;
  this->_iCurHomingOffset = 0;
  this->_iPasringData = 0;
  this->_bContinueFlag = false;
  this->_bTorqueOnOFFflag = false;
}

void DynamixelControl::xSetToqueStatFlag(uint8_t ucToqueFlag){
  this->_ucToqueStatFlag = ucToqueFlag;
}

void DynamixelControl::xSetEStopFlag(bool bEStopFlag){
  this->_bEStopFlag = bEStopFlag;
}

void DynamixelControl::xSetOperationMode(uint8_t ucOperMode){
  this->_ucOperationMode = ucOperMode;
}

void DynamixelControl::xSetDriveMode(uint8_t ucDrivMode){
  this->_ucDriveMode = ucDrivMode;
}

void DynamixelControl::xSetTargetPos(int32_t iTarPos){
  this->_iTargetPos = iTarPos;
}

void DynamixelControl::xSetCurPos(int32_t iCurPos){
  this->_iCurPos = iCurPos;
}

void DynamixelControl::xSetPreHOffset(int32_t iPreHOffset){
  this->_iPreHomingOffset= iPreHOffset;
}

void DynamixelControl::xSetCurHOffset(int32_t iCurHOffset){
  this->_iCurHomingOffset = iCurHOffset;
}

int32_t DynamixelControl::iGetPreHOffset(){
  return this->_iPreHomingOffset;
}

int32_t DynamixelControl::iGetCurHOffset(){
  return this->_iCurHomingOffset;
}

int32_t DynamixelControl::iGetCurPos(){
  return this->_iCurPos;
}

uint8_t DynamixelControl::ucGetTorqueFlag(){
  return this->_ucToqueStatFlag;
}

int16_t DynamixelControl::sGetCurCurrent(){
  return this->_sCurCurrent;
}

void DynamixelControl::xSetCurCurrent(int16_t sCurrent){
  this->_sCurCurrent = sCurrent;
}

void DynamixelControl::xSetPeriodTarget(uint8_t ucTarget){
  this->_ucPeriodTarget = ucTarget;
}

uint8_t DynamixelControl::ucGetPeriodTarget(){
  return this->_ucPeriodTarget;
}

void DynamixelControl::xSetContFlag(bool bContFlag){
  this->_bContinueFlag = bContFlag;
}

bool DynamixelControl::bGetContFlag(){
  return this->_bContinueFlag;
}


void DynamixelControl::xSetTorqueOnOff(bool bTorqueOnOff){
  this->_bTorqueOnOFFflag= bTorqueOnOff;
}

bool DynamixelControl::bGetTorqueOnOff(){
  return this->_bTorqueOnOFFflag;
}