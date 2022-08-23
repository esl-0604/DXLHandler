#pragma once

#include <stdio.h>
#include <cstring>

class DynamixelPackets {

  //variable

private:
  uint8_t _ucWRPackets[PACKET_SIZE];
  uint8_t _pucTempPackets[PACKET_SIZE];
  uint8_t _ucPacketsLen;
public:
  

  //method

private:
  
public:
 DynamixelPackets();
 
 DynamixelPackets& operator=(const DynamixelPackets& _DynamixelPackets);
 void xSetPackets(uint8_t* ucpPackets,uint8_t ucPacketLen);
 void xSetPacketLen(uint8_t packetLen);
 void xSetPackZero();
 uint8_t GetPacketLen();
 uint16_t usGetCrcData();
 uint8_t* GetPackets();
 uint8_t* GetTempPackets();
 
 //test
// void xSetSyncPackets(uint8_t* pPackData,uint8_t ucPacketLen,uint8_t ucNum);
};
