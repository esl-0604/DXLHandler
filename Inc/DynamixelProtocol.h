#pragma once

#include <main.h>
#include <cstring>

#include "DynamixelDefine.h"
#include "DynamixelPacket.h"
#include "DynamixelHandler.h"

class DynamixelPackets;
class DynamixelHandler;

class DynamixelProtocol {

  //variable
private:
    uint8_t _ucID;
    uint8_t _ucInstruction;
    uint32_t _uiData;
    uint32_t  _uiTargetAddress;
    uint16_t  _usDataLength;      //Data Length= byte number After LEN Section
    uint8_t _ucTotalPackLen;      // byte number of total packet
    
    //Sync section
    uint16_t _usSyncIdLen;
    uint8_t* _pSyncData;
    
public:
  
  //method
private:

public:
  DynamixelProtocol(DynamixelHandler* dynamixelHandler, uint8_t un8_ID);
//  DynamxielProtocol& operator = (const DynamixelProtocol& _DynamxielProtocol);
  
  DynamixelPackets GetWritePacket();
  DynamixelPackets GetReadPacket();
  DynamixelPackets GetSyncPacket();
  DynamixelPackets GetTxPingPacket();
  DynamixelPackets GetTxFacResetPacket();
  
  
  void xSetTxPingPacket(uint8_t _ucID);
  void xSetRxPingPacket(); 
  void xSetParam(uint8_t _ucID,uint8_t _ucInstruction,uint32_t _uiData,uint16_t  _usDataLength);
  void xSetWritePacket(uint8_t ucID,uint8_t _ucInstruction,uint32_t  _uiTargetAddress,uint32_t _uiData,uint16_t  _usDataLength);
  void xSetSyncWritePacket(uint16_t usSyncDataLen,uint8_t ucInstruction,uint32_t uiTargetAddress,uint16_t usDataLength,uint8_t* pSyncData);  
  void xSetReadPacket(uint8_t _ucInstruction,uint32_t  _uiTargetAddress,uint16_t  _usDataLength);
  void xSetSyncReadPacket(uint8_t ucSyncDataLen,uint8_t ucInstruction,uint32_t uiTargetAddress,uint16_t usDataLength,uint8_t* pSyncData);
  void xSetReadStatusPacket(uint8_t _ucInstruction,uint32_t _uiTargetAddress,uint16_t  _usDataLength);
  void xSetSyncPackLen(uint8_t ucIdNum);
  void xSetRxPacketLen(uint16_t data_length);
  void xSetRebootPackets();
  void xSetRxRebootPacket();
  void xSetRxFacResetPackets(uint8_t ucMode);
  void xSetPacketHeader(uint8_t* pucPacket);
  

  uint8_t ucGetPacketLen();
  uint32_t uiGetTargetAddr(); 
  uint8_t ucGetInstruction();  
  uint8_t ucGetId();
  uint8_t ucGetPSynIdLen();
};
