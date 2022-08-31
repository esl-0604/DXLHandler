#pragma once

#include <main.h>
#include <map>
#include "DynamixelProtocol.h"
#include "DynamixelPacket.h"
#include "Dynamixel.h"

class Dynamixel;

class DynamixelHandler{
//variable
private:
  UART_HandleTypeDef* _huart;
  uint8_t _ucID;
  bool _bParsingFlag;   // true : Parsing data exist
  bool _bCrcCheckFlag; // true : Receive CRC correct false: Receive CRC incorrect
  bool _bUpdatePacketFlag;
  bool _bHomingOperationFlag;
  bool _bTxFlag;
  DynamixelPackets* _rxPackets;
  uint8_t _ucErrorFlag;
  uint8_t _ucRxPackSize;
  uint16_t _usRxEndPos;
  uint16_t _usRxPreEndPos;
  uint16_t _usTargetAddr;

public:
  
//method  
private:
  
public:
  DynamixelHandler();
  DynamixelHandler(UART_HandleTypeDef* huart);
  UART_HandleTypeDef* GetUartHandle();
  //Transmit & Receive
  void xTransmitPacket(DynamixelPackets& l_dynamixelPackets);
  bool bCheckRxCRC();
  void xRxCpltFunction();
  void xSetRxPackets(uint8_t* pTempPack, uint8_t ucPackLen,uint8_t ucTempStartPos,uint8_t ucPackStartPos);
  void xSetRxTempPackets(uint8_t* pTempPack, uint8_t usRxPackEndPos);
  void xSetRxPreEndIndex(uint16_t usEndPos);
  void xSetRxPacketZero();
  void xSetRxPingPackSize();
  void xSetRxRebootPackSize();
  void xSetRxPackSize(uint16_t usLength,bool bSyncFlag,uint8_t ucSyncIdNum);
  void xSetUpdatePacketFlag(bool bPackUpadteflag);
  bool bGetUpdatePacketFlag();
  uint8_t usGetRxPackSize();
  uint8_t usGetRxEndPos();
  uint8_t ucGetHeaderIndex(uint8_t ucStartPos);
  uint8_t usGetRxPreEndPos();
  uint8_t* pucGetRxPackets();

  //Parsing
  bool bParsingFunction(Dynamixel* g_mapDynamixel,uint8_t ucSelMotor);
  void xSetParsingFlag(bool bflag);
  bool bGetParsingFlag();

  //homing
  void xSetHomingOperationFlag(bool bOperationFlag);
  bool bGetHomingOperationFlag();
  
  //ongoing category
  void xSetTxFlag(bool bTxFlag);
  bool bGetTxFlag();
  
  void xSetTargetAddr(uint16_t usTargetAddr);
  uint16_t uiGetTargetAddr();
};
