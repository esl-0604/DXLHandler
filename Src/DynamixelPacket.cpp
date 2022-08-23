#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "DynamixelDefine.h"
#include "DynamixelPacket.h"

/*operating mode
0  current control
1  Velocity control
3  Position control
4  Extended position control
5  Current base position control
16 PWM control
*/

DynamixelPackets::DynamixelPackets(){
  memset(_ucWRPackets,0,PACKET_SIZE);
  memset(_pucTempPackets,0,PACKET_SIZE);
  this->_ucPacketsLen=0;
}

DynamixelPackets& DynamixelPackets::operator=(const DynamixelPackets& _DynamixelPackets){
  this->_ucPacketsLen = _DynamixelPackets._ucPacketsLen;
  memcpy(this->_pucTempPackets,_DynamixelPackets._pucTempPackets,PACKET_SIZE);
  memcpy(this->_ucWRPackets,_DynamixelPackets._ucWRPackets,PACKET_SIZE);
  return *this;
}

void DynamixelPackets::xSetPacketLen(uint8_t packetLen){
  this->_ucPacketsLen=packetLen;
}

uint8_t DynamixelPackets::ucGetTargetAddr(){
  return _ucWRPackets[8];
}

void DynamixelPackets::xSetPackZero(){
  memset(this->_ucWRPackets,0,PACKET_SIZE);
  this->_ucPacketsLen=0;
}

uint8_t DynamixelPackets::GetPacketLen(){
  return this->_ucPacketsLen;
}

uint8_t* DynamixelPackets::GetPackets(){
  return (this->_ucWRPackets);
}

uint8_t* DynamixelPackets::GetTempPackets(){
  return this->_pucTempPackets;
}

void DynamixelPackets::xSetPackets(uint8_t* pPackets,uint8_t ucPacketLen){
  memcpy(this->_ucWRPackets,pPackets,(int)ucPacketLen);
}


uint16_t DynamixelPackets::usGetCrcData(){
  return (this->_ucWRPackets[this->GetPacketLen()-1]<<8 | this->_ucWRPackets[this->GetPacketLen()-2]);
}