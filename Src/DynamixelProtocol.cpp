#include <main.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "CrcCheck.h"
#include "DynamixelDefine.h"
#include "DynamixelPacket.h"
#include "DynamixelProtocol.h"
#include "DynamixelHandler.h"

void DynamixelProtocol::xSetParam(uint8_t _ucID,uint8_t _ucInstruction,uint32_t _uiData,uint16_t  _usDataLength){
this->_ucID=_ucID;
this->_ucInstruction=_ucInstruction;
this->_uiData=_uiData;
this->_usDataLength=_usDataLength;
}

//DynamixelProtocol& DynamixelProtocol::operator=(const DynamixelProtocol& _DynamxielProtocol){
//  this->_ucID = _DynamxielProtocol._ucID;
//  this->_
//  
//}


/*
default packet
H1,H2,H3,RSRV,PacketID, LEN1,LEN2, Inst, CRC1,CRC2 = 10
*/

DynamixelProtocol::DynamixelProtocol(DynamixelHandler* dynamixelHandler,uint8_t ucID) : _ucID(ucID) {
this->_ucInstruction=0;
this->_uiData=0;
this->_uiTargetAddress=0;
this->_usDataLength=0;
this->_ucTotalPackLen=0;
}

void DynamixelProtocol::xSetTxPingPacket(uint8_t _ucID){
  this->_ucID=_ucID;
  this->_ucInstruction=0x01;
  this->_uiData=0;
  this->_uiTargetAddress=0;
  this->_usDataLength=3; 
  this->_ucTotalPackLen= 10;
}

void DynamixelProtocol::xSetRxPingPacket(){
  this->_ucInstruction=STATUS_PACKET;
  this->_ucTotalPackLen= 14;
  this->_usDataLength=7;
}

void DynamixelProtocol::xSetRxRebootPacket(){
  this->_ucInstruction=STATUS_PACKET;
  this->_ucTotalPackLen= 11;
  this->_usDataLength=4;
}


void DynamixelProtocol::xSetRebootPackets(){
  this->_ucInstruction=REBOOT;
  this->_uiData=0;
  this->_uiTargetAddress=0;
  this->_usDataLength=3; 
  this->_ucTotalPackLen= 10;
}

void DynamixelProtocol::xSetRxFacResetPackets(uint8_t ucMode){
  this->_ucInstruction=FACTORY_RESET;
  this->_uiData=ucMode;
  this->_uiTargetAddress=0;
  this->_usDataLength=4; 
  this->_ucTotalPackLen= 11;
}

uint8_t DynamixelProtocol::ucGetPacketLen(){
return this->_ucTotalPackLen;
}

DynamixelPackets DynamixelProtocol::GetTxPingPacket(){
  
   DynamixelPackets l_dynamixelPackets;
   l_dynamixelPackets.xSetPacketLen(this->_ucTotalPackLen);
  uint8_t* l_pucPacket= (uint8_t*)malloc(this->_ucTotalPackLen);
  this->xSetPacketHeader(l_pucPacket);
  l_pucPacket[4]=this->_ucID;
  l_pucPacket[5]=0x03;
  l_pucPacket[6]=0x00;
  l_pucPacket[7]=this->_ucInstruction;
  uint16_t l_uiCRC_data = usUpdateCRC(0,l_pucPacket,this->_ucTotalPackLen-2);
  l_pucPacket[this->_ucTotalPackLen-2]= (l_uiCRC_data) & 0xFF;
  l_pucPacket[this->_ucTotalPackLen-1]= (l_uiCRC_data>>8) & 0xFF;
  l_dynamixelPackets.xSetPackets(l_pucPacket,this->_ucTotalPackLen);
  free(l_pucPacket);
  return l_dynamixelPackets;
}

DynamixelPackets DynamixelProtocol::GetTxFacResetPacket(){
  
   DynamixelPackets l_dynamixelPackets;
   l_dynamixelPackets.xSetPacketLen(this->_ucTotalPackLen);
  uint8_t* l_pucPacket= (uint8_t*)malloc(this->_ucTotalPackLen);
  this->xSetPacketHeader(l_pucPacket);
  l_pucPacket[4]=this->_ucID;
  l_pucPacket[5]=0x04;
  l_pucPacket[6]=0x00;
  l_pucPacket[7]=this->_ucInstruction;
  l_pucPacket[8]=(uint8_t)(this->_uiData);
  uint16_t l_uiCRC_data = usUpdateCRC(0,l_pucPacket,this->_ucTotalPackLen-2);
  l_pucPacket[this->_ucTotalPackLen-2]= (l_uiCRC_data) & 0xFF;
  l_pucPacket[this->_ucTotalPackLen-1]= (l_uiCRC_data>>8) & 0xFF;
  l_dynamixelPackets.xSetPackets(l_pucPacket,this->_ucTotalPackLen);
  free(l_pucPacket);
  return l_dynamixelPackets;
}


void DynamixelProtocol::xSetWritePacket(uint8_t ucID,uint8_t _ucInstruction,uint32_t  _uiTargetAddress,uint32_t _uiData,uint16_t _usDataLength){
  this->_ucID= ucID;
  this->_ucInstruction=_ucInstruction;
  this->_usDataLength= _usDataLength+5; // Target address 2bytes+ CRC 2Bytes+ Instruction 1Byte
  this->_ucTotalPackLen= _usDataLength+12; // DataLength+ header 4bytes+ Len 2Bytes+ ID 1bytes
  this->_uiTargetAddress=_uiTargetAddress;
  this->_uiData=_uiData;
}

void DynamixelProtocol::xSetSyncWritePacket(uint16_t ucSyncLen,uint8_t ucInstruction,uint32_t uiTargetAddress,uint16_t usDataLength,uint8_t* pSyncData){
  this->_ucID=BROAD_CAST_ID;
  this->_ucInstruction=ucInstruction;
  this->_usDataLength=usDataLength;         //Data Len for each ID
  this->_usSyncIdLen= ucSyncLen;      // Length of all ID & Data
  this->_ucTotalPackLen= ucSyncLen+14;  // total Data Length = SyncDataLength+ header 4bytes+ Len 2Bytes+ ID 1bytes +CRC 2bytes+ Ins 1byte+ Target Addr 2Bytes+ Data Length 2Bytes
  this->_uiTargetAddress=uiTargetAddress;
  this->_pSyncData=pSyncData;
}

void DynamixelProtocol::xSetSyncReadPacket(uint8_t ucSyncLen,uint8_t ucInstruction,uint32_t uiTargetAddress,uint16_t usDataLength,uint8_t* pSyncData){
  this->_ucID=BROAD_CAST_ID;
  this->_ucInstruction=ucInstruction;
  this->_usDataLength=usDataLength;               //Data Len for each ID
  this->_usSyncIdLen= ucSyncLen;                // Length of all ID number
  this->_ucTotalPackLen= ucSyncLen+14;            // total Data Length = SyncDataLength+ header 4bytes+ Len 2Bytes+ ID 1bytes +CRC 2bytes+ Ins 1byte+ Target Addr 2Bytes+ Data Length 2Bytes
  this->_uiTargetAddress=uiTargetAddress;
  this->_pSyncData=pSyncData;
}

void DynamixelProtocol::xSetReadPacket(uint8_t _ucInstruction,uint32_t _uiTargetAddress,uint16_t _usDataLength){
  this->_ucInstruction=_ucInstruction;
  this->_usDataLength= 7; // Target address 2bytes+ DataLength 2bytes+ CRC 2Bytes+ Instruction 1Byte
  this->_ucTotalPackLen= 14; // DataLength+ header 4bytes+ Len 2Bytes+ ID 1bytes
  this->_uiTargetAddress=_uiTargetAddress;
  this->_uiData=_usDataLength;
}

void DynamixelProtocol::xSetReadStatusPacket(uint8_t _ucInstruction,uint32_t _uiTargetAddress,uint16_t  _usDataLength){
  this->_ucInstruction=_ucInstruction;
  this->_usDataLength= _usDataLength; // Target address 2bytes+ DataLength 2bytes+ CRC 2Bytes+ Instruction 1Byte
  this->_ucTotalPackLen= _usDataLength+11; // DataLength+ header 4bytes+ Len 2Bytes+ ID 1bytes
  this->_uiTargetAddress=_uiTargetAddress;
  this->_uiData=_usDataLength;
}

void DynamixelProtocol::xSetSyncPackLen(uint8_t ucIdNum){
  this->_ucTotalPackLen= 8+ucIdNum*(this->_usDataLength+4);
}


DynamixelPackets DynamixelProtocol::GetWritePacket(){
  DynamixelPackets l_dynamixelPackets;
  l_dynamixelPackets.xSetPacketLen(this->_ucTotalPackLen);
  uint8_t* l_pucPacket= (uint8_t*)malloc(this->_ucTotalPackLen);
  this->xSetPacketHeader(l_pucPacket);
  l_pucPacket[4]=this->_ucID;
  l_pucPacket[5]=(this->_usDataLength)&0xFF;
  l_pucPacket[6]=(this->_usDataLength >>8) &0xFF;
  l_pucPacket[7]=this->_ucInstruction;
  l_pucPacket[8]=(this->_uiTargetAddress)& 0xFF;
  l_pucPacket[9]=(this->_uiTargetAddress>>8)& 0xFF;
  for(uint8_t i=0; i< (this->_usDataLength);i++){
  l_pucPacket[10+i]=(this->_uiData >> i*8) & 0xFF;
  }
  uint16_t l_uiCRC_data = usUpdateCRC(0,l_pucPacket,this->_ucTotalPackLen-2);
  l_pucPacket[this->_ucTotalPackLen-2]= (l_uiCRC_data) & 0xFF;
  l_pucPacket[this->_ucTotalPackLen-1]= (l_uiCRC_data>>8) & 0xFF;
  l_dynamixelPackets.xSetPackets(l_pucPacket,this->_ucTotalPackLen);
  free(l_pucPacket);
  return l_dynamixelPackets;
}


DynamixelPackets DynamixelProtocol::GetSyncPacket(){
  
  DynamixelPackets l_dynamixelPackets;
  l_dynamixelPackets.xSetPacketLen(this->_ucTotalPackLen);
  uint8_t* l_pucPacket= (uint8_t*)malloc(this->_ucTotalPackLen);
  this->xSetPacketHeader(l_pucPacket);
  l_pucPacket[4]=this->_ucID;
  l_pucPacket[5]=((this->_usSyncIdLen)+7)&0xFF; //instruction 1byte + CRC 2bytes+ target Addr 2Bytes + data Length 2Bytes
  l_pucPacket[6]=((this->_usSyncIdLen+7) >>8) &0xFF;
  l_pucPacket[7]=this->_ucInstruction;
  l_pucPacket[8]=(this->_uiTargetAddress)& 0xFF;
  l_pucPacket[9]=(this->_uiTargetAddress>>8)& 0xFF;
  l_pucPacket[10]=(this->_usDataLength) &0xFF;
  l_pucPacket[11]=(this->_usDataLength>>8) & 0xFF;
  for(uint8_t i=0; i< this->_usSyncIdLen;i++) {
    l_pucPacket[12+i]=(this->_pSyncData[i]) & 0xFF;
  }
  uint16_t l_uiCRC_data = usUpdateCRC(0,l_pucPacket,this->_ucTotalPackLen-2);
  l_pucPacket[this->_ucTotalPackLen-2]= (l_uiCRC_data) & 0xFF;
  l_pucPacket[this->_ucTotalPackLen-1]= (l_uiCRC_data>>8) & 0xFF;
  l_dynamixelPackets.xSetPackets(l_pucPacket,this->_ucTotalPackLen);
  free(l_pucPacket);
  return l_dynamixelPackets;
}

DynamixelPackets DynamixelProtocol::GetReadPacket(){
  
  DynamixelPackets l_dynamixelPackets;
  l_dynamixelPackets.xSetPacketLen(this->_ucTotalPackLen);
  uint8_t* l_pucPacket= (uint8_t*)malloc(this->_ucTotalPackLen);
  this->xSetPacketHeader(l_pucPacket);
  l_pucPacket[4]=this->_ucID;
  l_pucPacket[5]=(this->_usDataLength)&0xFF;
  l_pucPacket[6]=(this->_usDataLength >>8) &0xFF;
  l_pucPacket[7]=this->_ucInstruction;
  l_pucPacket[8]=(this->_uiTargetAddress)& 0xFF;
  l_pucPacket[9]=(this->_uiTargetAddress>>8)& 0xFF;
  l_pucPacket[10]=(this->_uiData) & 0xFF;
  l_pucPacket[11]=(this->_uiData>>8) & 0xFF;
  uint16_t l_uiCRC_data = usUpdateCRC(0,l_pucPacket,this->_ucTotalPackLen-2);
  l_pucPacket[this->_ucTotalPackLen-2]= (l_uiCRC_data) & 0xFF;
  l_pucPacket[this->_ucTotalPackLen-1]= (l_uiCRC_data>>8) & 0xFF;
  l_dynamixelPackets.xSetPackets(l_pucPacket,this->_ucTotalPackLen);
  free(l_pucPacket);
  return l_dynamixelPackets;
}

void DynamixelProtocol::xSetPacketHeader(uint8_t* pucPacket){
  pucPacket[0]=0xFF;
  pucPacket[1]=0xFF;
  pucPacket[2]=0xFD;
  pucPacket[3]=0x00;
}
void DynamixelProtocol::xSetRxPacketLen(uint16_t data_length){
  this->_ucTotalPackLen= PACKET_HEADER+(3+data_length); // 3은 ERROR와 CRC 어떻게 할지 고민중
}

uint32_t DynamixelProtocol::uiGetTargetAddr(){
  return this->_uiTargetAddress;
}

uint8_t DynamixelProtocol::ucGetInstruction(){
  return this->_ucInstruction;
}

uint8_t DynamixelProtocol::ucGetId(){
  return this->_ucID;
}

uint8_t DynamixelProtocol::ucGetPSynIdLen(){
  return this->_usSyncIdLen;
}