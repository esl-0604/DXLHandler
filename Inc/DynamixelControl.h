#pragma once
#include <stdint.h>
#include <map>
#include "DynamixelDefine.h"

class DynamixelControl {
//variable
private:
  bool _bEStopFlag;
  bool _bContinueFlag;
  bool _bTorqueOnOFFflag;
  uint8_t _ucToqueStatFlag;
  uint8_t _ucOperationMode;
  uint8_t _ucDriveMode;
  uint8_t _ucReturnLev;
  uint8_t _ucSyncIdNum;
  uint8_t _ucPeriodTarget;
  int16_t _sCurCurrent;
  int32_t _iTargetPos;
  int32_t _iCurPos;
  int32_t _iPreHomingOffset;
  int32_t _iCurHomingOffset;
  int32_t _iPasringData;

  std::map<uint8_t,uint16_t> _mapSecondId;
public:

  //method  
private:
  
public:
    DynamixelControl();
    
    void xSetToqueStatFlag(uint8_t ucToqueFlag);
    void xSetEStopFlag(bool bEStopFlag);
    void xSetOperationMode(uint8_t ucOperMode);
    void xSetDriveMode(uint8_t ucDriveMode);
    void xSetTargetPos(int32_t iTarPos);
    void xSetCurPos(int32_t iCurPos);
    void xSetPreHOffset(int32_t iPreHOffset);
    void xSetCurHOffset(int32_t iCurHOffset);
    uint8_t ucGetTorqueFlag();
    void xSetCurCurrent(int16_t Current);
    void xSetPeriodTarget(uint8_t ucTarget);
    uint8_t ucGetPeriodTarget();
    int16_t sGetCurCurrent();
    int32_t iGetPreHOffset();
    int32_t iGetCurHOffset();
    int32_t iGetCurPos();
    
    void xSetContFlag(bool bContFlag);
    bool bGetContFlag();
    
    void xSetTorqueOnOff(bool bTorqueFlag);
    bool bGetTorqueOnOff();
};  