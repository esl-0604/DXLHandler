#include <stdlib.h>
#include "CrcCheck.h"
#include "Dynamixel.h"
#include "DynamixelDefine.h"
#include "DynamixelHandler.h"

extern uint16_t g_Parsing_cnt;
extern uint8_t Dxl_flag;
extern int32_t g_iParsingData;
extern int32_t g_iPositionData;
extern int16_t g_sCurrentData;
  
DynamixelHandler::DynamixelHandler(UART_HandleTypeDef* huart){
  this->_huart=huart;
  this->_ucID = _ucID;
  this->_bParsingFlag = false;
  this->_bCrcCheckFlag = false;
  this->_bTxFlag = true;
  this->_rxPackets = new DynamixelPackets;
  this->_ucRxPackSize=0;
  this->_usRxEndPos=0;
  this->_usRxPreEndPos=0;
  this->_bUpdatePacketFlag=false;
  this->_bHomingOperationFlag=false;
  this->_ucErrorFlag = ERROR_NONE; 
  this->_usTargetAddr = 0;
}


uint8_t* l_ucPackets = 0;
void DynamixelHandler::xTransmitPacket(DynamixelPackets& l_dynamixelPackets){
    uint8_t l_PackLen = l_dynamixelPackets.GetPacketLen();
    l_ucPackets = l_dynamixelPackets.GetPackets();
    while(bGetTxFlag() != true);
    this->xSetTxFlag(false);
    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_SET);
    HAL_UART_Transmit_IT(this->_huart, l_ucPackets, l_PackLen);
}

bool DynamixelHandler::bParsingFunction(Dynamixel* g_mapDynamixel,uint8_t ucSelMotor){
  
  const  uint16_t l_usPrePos = this->usGetRxPreEndPos();
  const  uint16_t l_usEndPos = this->usGetRxEndPos();
  const uint8_t l_ucBuffPos = this->ucGetHeaderIndex(l_usPrePos);
  const  uint16_t l_usPackSize = l_usEndPos > l_ucBuffPos ? (l_usEndPos - l_ucBuffPos) : ((PACKET_SIZE - l_ucBuffPos) + l_usEndPos); 
  this->_rxPackets->xSetPacketLen((uint8_t)l_usPackSize);
  
  if(l_ucBuffPos < PACKET_NOTFOUND){
    this->xSetRxPacketZero();
    if(l_usEndPos > l_ucBuffPos)
      this->xSetRxPackets(this->_rxPackets->GetTempPackets(),l_usPackSize,l_ucBuffPos,0);
    else{
      uint8_t l_ucRestSize= PACKET_SIZE-l_ucBuffPos;
      this->xSetRxPackets(this->_rxPackets->GetTempPackets(),l_ucRestSize,l_ucBuffPos,0);
      this->xSetRxPackets(this->_rxPackets->GetTempPackets(),l_usEndPos,0,l_ucRestSize);
    }
    if(this->bCheckRxCRC()){
      uint8_t* l_pucTempPackPtr = this->pucGetRxPackets();
      this->xSetRxPreEndIndex(l_usEndPos);
      if(g_mapDynamixel[ucSelMotor].ucGetInst() == 0x8A){
        if(g_mapDynamixel[ucSelMotor].uiGetTargetAddr() == PRESENT_POSITION){
            if(l_pucTempPackPtr[9] == g_mapDynamixel[ucSelMotor].ucGetId()){
              g_mapDynamixel[ucSelMotor].xSetPos((l_pucTempPackPtr[13]<<24) | (l_pucTempPackPtr[11]<<12) | (l_pucTempPackPtr[11]<<8) | (l_pucTempPackPtr[10]));
              if(ucSelMotor == 0){
                g_mapDynamixel[1].xSetPos((l_pucTempPackPtr[21]<<24) | (l_pucTempPackPtr[20]<<16) | (l_pucTempPackPtr[19]<<8) | (l_pucTempPackPtr[18]));
              }
              else if(ucSelMotor == 1){
                g_mapDynamixel[0].xSetPos((l_pucTempPackPtr[21]<<24) | (l_pucTempPackPtr[20]<<16) | (l_pucTempPackPtr[19]<<8) | (l_pucTempPackPtr[18]));
              }
            }
            else{
                g_mapDynamixel[1].xSetPos((l_pucTempPackPtr[13]<<24) | (l_pucTempPackPtr[12]<<16) | (l_pucTempPackPtr[11]<<8) | (l_pucTempPackPtr[10]));
                g_mapDynamixel[0].xSetPos((l_pucTempPackPtr[21]<<24) | (l_pucTempPackPtr[20]<<16) | (l_pucTempPackPtr[19]<<8) | (l_pucTempPackPtr[18]));
            }
        }
        else if(this->_usTargetAddr == PRESENT_CURRENT){
          if(l_pucTempPackPtr[9] == g_mapDynamixel[ucSelMotor].ucGetId()){
            g_mapDynamixel[ucSelMotor].xSetCur((l_pucTempPackPtr[11]<<8) | (l_pucTempPackPtr[10]));
            if(ucSelMotor == 0){
              g_mapDynamixel[1].xSetCur((l_pucTempPackPtr[17]<<8) | (l_pucTempPackPtr[16]));
            }
            else if(ucSelMotor == 1){
              g_mapDynamixel[0].xSetCur((l_pucTempPackPtr[17]<<8) | (l_pucTempPackPtr[16]));
            }
          }
          else{
                g_mapDynamixel[1].xSetCur((l_pucTempPackPtr[11]<<8) | (l_pucTempPackPtr[10]));
                g_mapDynamixel[0].xSetCur((l_pucTempPackPtr[17]<<8) | (l_pucTempPackPtr[16]));
          }
        }
      }
      else{
        if(this->_usTargetAddr == PRESENT_POSITION){
         g_mapDynamixel[ucSelMotor].xSetPos((l_pucTempPackPtr[12]<<24) | (l_pucTempPackPtr[11]<<16) | (l_pucTempPackPtr[10]<<8) | (l_pucTempPackPtr[9]));
        }
        else if(this->_usTargetAddr == PRESENT_CURRENT){
          g_mapDynamixel[ucSelMotor].xSetCur((l_pucTempPackPtr[10]<<8) | (l_pucTempPackPtr[9]));
        }
        else if(this->_usTargetAddr ==  TORQUE_ADDR){
          g_iParsingData = (l_pucTempPackPtr[9]);
        }
      }
      return true;
    }
    return false;
  }
  return false;
}

  

UART_HandleTypeDef* DynamixelHandler::GetUartHandle(){
return this->_huart;
}

void DynamixelHandler::xSetParsingFlag(bool bflag){
  this->_bParsingFlag=bflag;
}

bool DynamixelHandler::bGetParsingFlag(){
  return this->_bParsingFlag;
}

void DynamixelHandler::xSetRxPackSize(uint16_t usDataLength,bool bSyncFlag,uint8_t ucSyncIdNum){
  if(bSyncFlag) // true= sync Read
    this->_ucRxPackSize=(8+(ucSyncIdNum*(4+usDataLength)));
  else
    this->_ucRxPackSize= (11+usDataLength);
}

void DynamixelHandler::xSetRxPingPackSize(){
  this->_ucRxPackSize= PING_PACKET_SIZE;
}

void DynamixelHandler::xSetRxRebootPackSize(){
  this->_ucRxPackSize= REBOOT_PACKET_SIZE;
}


void DynamixelHandler::xSetRxPackets(uint8_t* pTempPack, uint8_t ucPackLen,uint8_t ucTempStartPos,uint8_t ucPackStartPos){
  uint8_t* l_pucTempStartIndex = pTempPack+ucTempStartPos;
  uint8_t* l_pucPackStartIndex = this->_rxPackets->GetPackets()+ucPackStartPos;
  memcpy(l_pucPackStartIndex,l_pucTempStartIndex,ucPackLen);
}

void DynamixelHandler::xSetRxTempPackets(uint8_t* pTempPack, uint8_t usRxEndPos){
  memcpy(this->_rxPackets->GetTempPackets(), pTempPack, sizeof(uint8_t)*PACKET_SIZE);
  this->_usRxEndPos=usRxEndPos;
}

uint8_t DynamixelHandler::ucGetHeaderIndex(uint8_t ucStartPos){
  uint8_t* l_pTempPacket = this->_rxPackets->GetTempPackets();
  if(ucStartPos<(PACKET_SIZE-2)){
    for(uint8_t i=ucStartPos; i<PACKET_SIZE-2;i++){
    if((l_pTempPacket[i]==0xFF)&&(l_pTempPacket[i+1]==0xFF)&&(l_pTempPacket[i+2]==0xFD))
      return i;
    }
  }
  else{
    if((l_pTempPacket[PACKET_SIZE-2]==0xFF)&&(l_pTempPacket[PACKET_SIZE-1]==0xFF)&&(l_pTempPacket[0]==0xFD))
      return (uint8_t)(PACKET_SIZE-2);
    if((l_pTempPacket[PACKET_SIZE-1]==0xFF)&&(l_pTempPacket[0]==0xFF)&&(l_pTempPacket[1]==0xFD))
      return (uint8_t)(PACKET_SIZE-1);
  }
    return PACKET_NOTFOUND;
}

uint8_t DynamixelHandler::usGetRxPackSize(){
  return this->_ucRxPackSize;
}

uint8_t DynamixelHandler::usGetRxEndPos(){
  return this->_usRxEndPos;
}

uint8_t DynamixelHandler::usGetRxPreEndPos(){
  return this->_usRxPreEndPos;
}

void DynamixelHandler::xSetRxPacketZero(){
  memset(this->_rxPackets->GetPackets(),0,sizeof(uint8_t)*PACKET_SIZE);
}

uint16_t g_tempdata= 0;
uint8_t* g_pucTemp =NULL;
uint16_t g_tempLen=0;

bool DynamixelHandler::bCheckRxCRC(){
  g_tempdata = usUpdateCRC(0,this->_rxPackets->GetPackets(),this->_rxPackets->GetPacketLen()-2);
  g_tempLen =this->_rxPackets->GetPacketLen()-2;
  g_pucTemp = this->_rxPackets->GetPackets();
  if(this->_rxPackets->usGetCrcData() == usUpdateCRC(0,this->_rxPackets->GetPackets(),this->_rxPackets->GetPacketLen()-2))
    return true;
  return false;
}

void DynamixelHandler::xSetRxPreEndIndex(uint16_t usEndpos){
  if(usEndpos == PACKET_SIZE)
    usEndpos=0;
  this->_usRxPreEndPos=usEndpos;
}

uint8_t* DynamixelHandler::pucGetRxPackets(){
  return this->_rxPackets->GetPackets();
}

void DynamixelHandler::xSetUpdatePacketFlag(bool bPackUpadteflag){
  this->_bUpdatePacketFlag= bPackUpadteflag;
}

bool DynamixelHandler::bGetUpdatePacketFlag(){
  return this->_bUpdatePacketFlag;
}

void DynamixelHandler::xSetHomingOperationFlag(bool bOperationFlag){
  this->_bHomingOperationFlag=bOperationFlag;
}

bool DynamixelHandler::bGetHomingOperationFlag(){
  return this->_bHomingOperationFlag;
}

void DynamixelHandler::xSetTxFlag(bool bTxFlag){
  this->_bTxFlag = bTxFlag;
}

bool DynamixelHandler::bGetTxFlag(){
  return this->_bTxFlag;
}

void DynamixelHandler::xSetTargetAddr(uint16_t usTargetAddr){
  this->_usTargetAddr = usTargetAddr;
}

uint16_t DynamixelHandler::uiGetTargetAddr(){
  return this->_usTargetAddr;
}
