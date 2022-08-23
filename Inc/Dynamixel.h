#pragma once

#include <map>

#include "DynamixelControl.h"
#include "DynamixelHandler.h"
#include "DynamixelPacket.h"
#include "DynamixelProtocol.h"
//#include "PhotoSensor.h"

class DynamixelHandler;
class DynamixelProtocol;
class DynamixelPackets;
  
class Dynamixel {
//variable
private:
        uint8_t _ucID;
        bool _bPhotoSenor;
        uint8_t _ucHomingFlag;
        DynamixelHandler* _dynamixHandler;
        DynamixelProtocol* _dynamixProtocol;
        DynamixelControl* _dynamixControl;
//        PhotoSensor* _PhotoSensor;
        static std::map<uint8_t, Dynamixel*> m_Dynamixel;
        static DynamixelPackets m_DynamixelTxPackets;

//Motor Variable
        int16_t _sCurrent;
        int32_t _iPosition;

public:
  

//method

private:
  
public:

  Dynamixel(DynamixelHandler* DynamixelHandler, uint8_t ucID);
  virtual ~Dynamixel();
//Protocol Instruction
  void Dxl_Ping();
  void xTransmitDxlRPacket(uint16_t address, uint16_t usdata_length);
  void xTransmitDxlWPacket(uint16_t address, int32_t sdata, uint16_t usdata_length);
  void xTransmitShadowID(uint8_t ucShdowID);
  void xTransmitSyncDxlPacket(uint16_t address, int32_t* data, uint16_t usdata_length,uint8_t ucIdNum, ...); //가변인자는 ID
  void xTransmitRebootDxl();
  void xTransmitFacReset(uint8_t ucMode);  
  void ReceiveSyncDxlPacket(uint16_t address, uint16_t usdata_length,uint8_t ucIdNum, ...); 
  void xEnableTorque();
  bool bDisableTorque();
  bool bGetUpdateFlag();
  bool bParsingCheck();
  
//Homing Mode
  void xInitDxl(int mode,int lev);
  void xHomingMode(int32_t iGoalPosition,Dynamixel* DynamixelOther);
  bool bCompareGoalPresentPos(int32_t iGoalPos,int32_t iPresentPos);
  void xSetHomingFlag(uint8_t bHomingFlag);
  void xSetCurPosToControl(int32_t iCurPos);
  bool bGetHomingOperationFlag();
  uint8_t ucGetHomingFlag();
  int32_t iGetPositionFromRxPacket();

//Photo Senser
  bool bCheckPhotoSensor();
  void xSetPhotoFlag(bool bPhotoFlag);
  bool bGetPhotoFlag();
  
//ongoing Category
  bool bCheckTHCurrent();
  uint8_t ucGetTorqueStat();
  int16_t sGetCurrent();
  void xSetCurrent(int16_t sCurrent);
  uint32_t uiGetTargetAddr();
  bool bGetTorque();
  uint8_t ucGetPeriodTarget();
  void xSetTargetAddr(uint16_t usTargetAddr);
  void xSetUpdateFlag(bool bUpdateFlag);
  
  
  /////
  int32_t iGetCurrentPosition();
  void xSetTorqueStat(uint8_t ucTorqueStat);
  
  void xSetContFlag(bool bContFlag);
  bool bGetContFlag();
  void xSetTxPacketZero();
  void Dxl_Init();
  void xSetTorqueOnOff(bool bTorqueflag);
  bool bGetTorqueOnOff();
  void xSetCur(int16_t sCur);
  void xSetPos(int32_t iPos);
  int16_t sGetCur();
  int32_t iGetPos();
  
  uint8_t ucGetInst();
  uint8_t ucGetId();
  uint8_t ucGetIdLen();
  /////
};