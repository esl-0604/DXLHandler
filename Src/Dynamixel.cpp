#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "Dynamixel.h"

uint16_t g_ucRxLen = 0;
uint8_t l_ucTmp[30] = {0 , };


//test

//Control var
extern int8_t g_cMode;
extern int8_t g_cLevel;
extern uint32_t g_iGoalPosition;
extern uint8_t Dxl_flag;
extern int32_t g_iParsingData;
extern bool g_bCartOutFlag;
extern bool g_bCartInFlag;
extern bool g_bCartEndFlag;
bool g_bInitCompleteFlag = false;
extern uint8_t g_ucSelectMotor;
//

int32_t g_testRight = 10000;
int32_t g_testLeft = 10000;


int32_t g_iTempTickCnt = 0;

std::map<uint8_t, Dynamixel*> Dynamixel::m_Dynamixel;
DynamixelPackets Dynamixel::m_DynamixelTxPackets;

bool DelayFun(uint32_t delayTick){
  if(HAL_GetTick() - g_iTempTickCnt > delayTick){
    return true;
  }
  return false;
}


Dynamixel::Dynamixel(DynamixelHandler* DynamixelHandler, uint8_t ucID){
  this->_ucID = ucID;
  this->_dynamixHandler = DynamixelHandler;
  this->_bPhotoSenor = true;
  this->_dynamixProtocol = new DynamixelProtocol(DynamixelHandler, ucID);
  this->_dynamixControl = new DynamixelControl();
  this->_ucHomingFlag=0;
  Dynamixel::m_Dynamixel.insert(std::pair<uint8_t, Dynamixel*>(ucID,this));

  this->_sCurrent = 0;
  this->_iPosition = 0;

}

Dynamixel::~Dynamixel() {
  delete this->_dynamixProtocol;
  delete this->_dynamixControl;
//  delete this->_PhotoSensor;
}

void Dynamixel::Dxl_Ping(){
  //Transmit Ping Protocol
  _dynamixProtocol->xSetTxPingPacket(this->_ucID);
  DynamixelPackets l_dynamixelPackets = _dynamixProtocol->GetTxPingPacket();
  this->_dynamixHandler->xTransmitPacket(l_dynamixelPackets);

}

void Dynamixel::xInitDxl(int mode,int lev){
  Dynamixel::m_Dynamixel[this->_ucID] =this;
  
    this->xTransmitDxlWPacket(TORQUE_ADDR,0,1);
    HAL_Delay(2);
  // address : 64  토크 on/off default : 0
    this->xTransmitDxlWPacket(OPERATING_MODE,mode,1);               // address : 11  operating Mode default : 3
    HAL_Delay(2);
    this->xTransmitDxlWPacket(STATUS_RETURN_LEVEL,lev,1);               // address : 68  Return level : 1
    HAL_Delay(2);
    this->xTransmitDxlWPacket(PROFILE_VELOCITY,300,4);
    HAL_Delay(2);
    this->xTransmitDxlWPacket(PROFILE_ACCELERATION,60,4);
    HAL_Delay(2);
//    this->xTransmitDxlWPacket(POSITION_I_GAIN,10,4);
//    HAL_Delay(2);
//    this->xTransmitDxlWPacket(POSITION_D_GAIN,100,4);
//    HAL_Delay(2);
    this->xTransmitDxlWPacket(POSITION_P_GAIN,1300,4);
    HAL_Delay(2);
    this->xEnableTorque();                       // address : 64  토크 on/off default : 0
}

 
void Dynamixel::xTransmitDxlWPacket(uint16_t address, int32_t usdata, uint16_t usdata_length){
    this->_dynamixProtocol->xSetWritePacket(this->_ucID,WRITE_INST,address,usdata,usdata_length);
    Dynamixel::m_DynamixelTxPackets = this->_dynamixProtocol->GetWritePacket();
    this->_dynamixHandler->xTransmitPacket(m_DynamixelTxPackets);
}

void Dynamixel::xTransmitDxlRPacket(uint16_t address, uint16_t usdata_length){

    //Transmit Read Protocol
    this->_dynamixProtocol->xSetReadPacket(READ_INST,address,usdata_length);
    Dynamixel::m_DynamixelTxPackets = this->_dynamixProtocol->GetReadPacket();
    this->_dynamixHandler->xTransmitPacket(m_DynamixelTxPackets);
    
    //Receive Packet Setting
    this->_dynamixHandler->xSetTargetAddr(address);
}

uint8_t testBuffer[30];
void Dynamixel::xTransmitSyncDxlPacket(uint16_t address, int32_t* data, uint16_t usdata_length,uint8_t ucIdNum, ...){
  //parm setting
  uint16_t l_uiSyncDataLen=(ucIdNum*(1+usdata_length));
  uint8_t* l_pSyncData= (uint8_t*)malloc(l_uiSyncDataLen); // ID number + data*ID number
//  uint8_t l_pSyncData[10]= {0,};
//  uint8_t l_pucIDArr[2] = {5,6};
  va_list l_pID;  
  va_start(l_pID,ucIdNum);
  for (int i=0; i<ucIdNum;i++){
    l_pSyncData[(i*usdata_length)+i]= va_arg(l_pID,int); 
//    testBuffer[(i*usdata_length)+i]= l_pucIDArr[i];
      for(int j=0;j<usdata_length;j++) {
        l_pSyncData[((i*usdata_length)+i)+j+1]=(data[i]>>(j*8))&0xFF; //ID Index 1
        testBuffer[((i*usdata_length)+i)+j+1]=(data[i]>>(j*8))&0xFF;
      }
    }
  va_end(l_pID);
  
  this->_dynamixProtocol->xSetSyncWritePacket(l_uiSyncDataLen,SYNC_WRITE_INST,address,usdata_length,l_pSyncData);
  DynamixelPackets l_DynamixelPackets = this->_dynamixProtocol->GetSyncPacket();
  this->_dynamixHandler->xTransmitPacket(l_DynamixelPackets);
  free(l_pSyncData);
}

void Dynamixel::xTransmitRebootDxl(){
  this->_dynamixProtocol->xSetRebootPackets();
  DynamixelPackets l_dynamixelPackets = _dynamixProtocol->GetTxPingPacket();
  this->_dynamixHandler->xTransmitPacket(l_dynamixelPackets);
}
  
void Dynamixel::xTransmitFacReset(uint8_t ucMode){
  this->_dynamixProtocol->xSetRxFacResetPackets(ucMode);
  DynamixelPackets l_dynamixelPackets = _dynamixProtocol->GetTxFacResetPacket();
  this->_dynamixHandler->xTransmitPacket(l_dynamixelPackets);
}

void Dynamixel::ReceiveSyncDxlPacket(uint16_t address, uint16_t usdata_length,uint8_t ucIdNum, ...){
  
  //Transmit Read Protocol
 
  //parm setting
  uint8_t* l_pSyncData= (uint8_t*)malloc(ucIdNum); // ID number
  
  va_list l_pID;  
  va_start(l_pID,ucIdNum);
  for (int i=0; i<ucIdNum;i++)
    l_pSyncData[i]=va_arg(l_pID,int); 
  va_end(l_pID);
  
  this->_dynamixProtocol->xSetSyncReadPacket(ucIdNum,STNC_FAST_READ_INST,address,usdata_length,l_pSyncData);
  DynamixelPackets l_DynamixelPackets = this->_dynamixProtocol->GetSyncPacket();
    
  //Transmit Packet
  this->_dynamixHandler->xTransmitPacket(l_DynamixelPackets);
  
  this->_dynamixHandler->xSetTargetAddr(address);
  
  free(l_pSyncData);
}

//void Dynamixel::xTransmitShadowID(uint8_t ucShdowID){
//  //Toque off
//  this->_dynamixProtocol->xSetWritePacket(WRITE_INST,TORQUE_ADDR,0,1);
//  DynamixelPackets l_DynamixelPackets = this->_dynamixProtocol->GetWritePacket();
//  // this->_dynamixHandler->xTransmitPacket(&l_DynamixelPackets); TODO
//
//  this->_dynamixProtocol->xSetWritePacket(WRITE_INST,SECONDARY_ID,ucShdowID,1);
//  l_DynamixelPackets = this->_dynamixProtocol->GetWritePacket();
//  //this->_dynamixHandler->xTransmitPacket(&l_DynamixelPackets); TODO
//}

bool g_bCurPosFlag= false;


void Dynamixel::xHomingMode(int32_t iGoalPosition,Dynamixel* DynamixelOther){
 if(bCheckTHCurrent()){
  switch(this->ucGetHomingFlag()){
  case HOMING_INIT:    
    this->xInitDxl(g_cMode,g_cLevel);
    if(g_bInitCompleteFlag) {
      this->xSetHomingFlag(HOMING_T_GOAL_POS);
    }
    break;
  case HOMING_T_GOAL_POS:
    this->xTransmitDxlWPacket(GOAL_POSITION,iGoalPosition,4);
    this->xSetHomingFlag(HOMING_CHECK_POS);
    break;
  case HOMING_CHECK_POS:
    this->_dynamixHandler->xSetHomingOperationFlag(true); //position Flag 설정해야함
    this->xSetHomingFlag(HOMING_WAIT);
    break;
  case HOMING_WAIT:
    if(this->bCompareGoalPresentPos(g_iGoalPosition,this->iGetPos())){
      this->_dynamixHandler->xSetHomingOperationFlag(false);
      this->xSetHomingFlag(HOMING_GOTO_PHOTO);
    }
    break;
  case HOMING_GOTO_PHOTO:
    this->xTransmitDxlWPacket(TORQUE_ADDR,0,1);
    HAL_Delay(2);
    this->xTransmitDxlWPacket(OPERATING_MODE,1,1);
    HAL_Delay(2);
    this->xTransmitDxlWPacket(TORQUE_ADDR,1,1);
    HAL_Delay(2);
    this->xTransmitDxlWPacket(GOAL_VELOCITY,250,4);
    HAL_Delay(2);
    this->_dynamixHandler->xSetHomingOperationFlag(true); // position Flag 설정해야함
    this->xSetHomingFlag(HOMING_CHECK_PHOTO);
    break;
  case HOMING_CHECK_PHOTO:
    if(this->bGetPhotoFlag()==false){
      this->_dynamixHandler->xSetHomingOperationFlag(false);
      this->xTransmitDxlWPacket(TORQUE_ADDR,0,1);
      HAL_Delay(2);
      this->xTransmitDxlWPacket(TORQUE_ADDR,1,1);
      HAL_Delay(2);
      this->xTransmitDxlWPacket(GOAL_VELOCITY,-50,4);
      HAL_Delay(2);
      this->_dynamixHandler->xSetHomingOperationFlag(true);
      this->xSetHomingFlag(HOMING_CHECK_PHOTO_SEC);
      }
    break;
  case HOMING_CHECK_PHOTO_SEC:
    if(this->bGetPhotoFlag()){
      this->_dynamixHandler->xSetHomingOperationFlag(false);
      this->xTransmitDxlWPacket(TORQUE_ADDR,0,1);
      this->xSetHomingFlag(HOMING_OFFSET_SET);
    }
    break;
  case HOMING_OFFSET_SET:
    this->_dynamixControl->xSetCurHOffset((this->_dynamixControl->iGetPreHOffset()+this->iGetPos()));
    HAL_Delay(2);
    this->xTransmitDxlWPacket(HOMING_OFFSET,((-1)*this->_dynamixControl->iGetCurHOffset()),4);
    HAL_Delay(2);
    this->_dynamixControl->xSetPreHOffset(this->_dynamixControl->iGetCurHOffset());
    this->xTransmitDxlWPacket(TORQUE_ADDR,1,1);
    HAL_Delay(2);
    this->xTransmitDxlRPacket(PRESENT_POSITION,4);
    this->xSetHomingFlag(HOMING_GOTO_CARTRIDGE);

    break;
  case HOMING_GOTO_CARTRIDGE:
    this->xTransmitDxlWPacket(TORQUE_ADDR,0,1);
    HAL_Delay(2);
    this->xTransmitDxlWPacket(PROFILE_ACCELERATION,0,4);
    HAL_Delay(2);
    this->xTransmitDxlWPacket(PROFILE_VELOCITY,0,4);
    HAL_Delay(2);
    this->xTransmitDxlWPacket(OPERATING_MODE,4,1);
    HAL_Delay(2);
    this->xTransmitDxlWPacket(TORQUE_ADDR,1,1);
    HAL_Delay(2);
    this->xTransmitDxlWPacket(GOAL_POSITION,END_OF_LM_GUIDE,4);
    this->xSetHomingFlag(0x11);
        // 완료했으니 right로
    if(g_ucSelectMotor == 0){
      g_ucSelectMotor =1;
      this->xSetHomingFlag(HOMING_INIT);
      g_bInitCompleteFlag = false;
    }
    else if(g_ucSelectMotor == 1){
      this->xSetHomingFlag(HOMING_STEP_GOING);
    }
    break;
  case HOMING_STEP_GOING:
    if(g_bCurPosFlag){
      this->ReceiveSyncDxlPacket(PRESENT_POSITION,4,2,5,6);
      g_bCurPosFlag = false;
    }
    else{
      this->ReceiveSyncDxlPacket(PRESENT_CURRENT,2,2,5,6);
      g_bCurPosFlag = true;
    }
    if(g_bCartOutFlag){
      int32_t l_iTargetPosLeft = this->iGetPos()+g_testLeft;
      int32_t l_iTargetPosRight = DynamixelOther->iGetPos()+g_testRight;
      int32_t  l_piSyncTargetPos[2]={l_iTargetPosRight,l_iTargetPosLeft};
      this->xTransmitSyncDxlPacket(GOAL_POSITION,l_piSyncTargetPos,4,2,5,6);
      g_bCartOutFlag = false;
    }
    if(g_bCartInFlag){
      int32_t l_iTargetPosLeft = this->iGetPos()-g_testLeft;
      int32_t l_iTargetPosRight = DynamixelOther->iGetPos()-g_testRight;
      int32_t  l_piSyncTargetPos[2]={l_iTargetPosRight,l_iTargetPosLeft};
      this->xTransmitSyncDxlPacket(GOAL_POSITION,l_piSyncTargetPos,4,2,5,6);
      g_bCartInFlag = false;
    }
    if(g_bCartEndFlag){
            int32_t  l_piSyncTargetPos[2]={MID_OF_LM_GUIDE,MID_OF_LM_GUIDE};
      this->xTransmitSyncDxlPacket(GOAL_POSITION,l_piSyncTargetPos,4,2,5,6);
      HAL_Delay(2);
//      this->xTransmitFacReset(0x02); //정해진거리 체크하는거 필요함
      this->xSetHomingFlag(HOMING_EXIT);
      g_bCartEndFlag = false;
    }
    break;
  case HOMING_EXIT:
    this->xSetHomingFlag(HOMING_INIT);
    this->_dynamixHandler->xSetHomingOperationFlag(false);
    Dxl_flag=0;
    break;
  case HOMING_EMER_EXIT:
    if(this->bDisableTorque()){
    this->_dynamixHandler->xSetHomingOperationFlag(false);
    Dxl_flag = 0;
    }
    break;
  default:
    break;
  }
 }
 else{
   Dxl_flag = 0;
 }
}

int32_t Dynamixel::iGetPositionFromRxPacket(){
  uint8_t* l_pucPackPtr=this->_dynamixHandler->pucGetRxPackets();
  int32_t l_iTempData=((l_pucPackPtr[12]<<24) | (l_pucPackPtr[11]<<16) | (l_pucPackPtr[10]<<8) | l_pucPackPtr[9] )& 0xffffffff;
  this->_dynamixHandler->xSetUpdatePacketFlag(false);
  return l_iTempData;
}

bool Dynamixel::bGetUpdateFlag(){
  return this->_dynamixHandler->bGetUpdatePacketFlag();
}

void Dynamixel::xSetHomingFlag(uint8_t ucHomingFlag){
  this->_ucHomingFlag= ucHomingFlag;
}

uint8_t Dynamixel::ucGetHomingFlag(){
  return this->_ucHomingFlag;
}

bool Dynamixel::bCompareGoalPresentPos(int32_t iGoalPos,int32_t iPresentPos){
  uint32_t l_uiDiffer= labs((int32_t)(iGoalPos-iPresentPos));
  if(l_uiDiffer < HOMING_COMPAREOFFSET)
     return true;
  return false;
}
           
void Dynamixel::xSetPhotoFlag(bool bPhotoFlag){
  this->_bPhotoSenor= (bPhotoFlag);
}

bool Dynamixel::bGetHomingOperationFlag(){
  return  this->_dynamixHandler->bGetHomingOperationFlag();
}

void Dynamixel::xSetCurPosToControl(int32_t iCurPos){
  this->_dynamixControl->xSetCurPos(iCurPos);
  this->_dynamixHandler->xSetUpdatePacketFlag(false);
}

void Dynamixel::xEnableTorque(){
  this->xTransmitDxlWPacket(TORQUE_ADDR,1,1);
  this->xSetTorqueOnOff(true);
}




bool Dynamixel::bDisableTorque(){
  uint8_t l_ucTorqueStat = this->ucGetTorqueStat();
  
  switch(l_ucTorqueStat){
    case TORQUE_STATUS_OFF:
    this->xTransmitDxlWPacket(TORQUE_ADDR,0,1);
    this->xSetTorqueStat(TORQUE_STATUS_READ);
    break;
  case TORQUE_STATUS_READ:
    this->xTransmitDxlRPacket(TORQUE_ADDR,1);
    this->xSetTorqueStat(TORQUE_STATIUS_CHECK);
    break;
  case TORQUE_STATIUS_CHECK:
    if(g_iParsingData == 0)
      this->xSetTorqueStat(TORQUE_STATUS_EXIT);
    else if(g_iParsingData == 1)
      this->xSetTorqueStat(TORQUE_STATUS_OFF);
    break;
  case TORQUE_STATUS_EXIT:
    this->xSetTorqueOnOff(false);
    this->xSetTorqueStat(TORQUE_STATUS_OFF);
    return true;
    break;
  }
  return false;
}

uint8_t g_ucCount = 0; //current count 용 3개의 평균값 활용
bool g_bDisableFlag= true;

  
// 긴급정지인데 시스를 홀드할때와 11나눠야함
  bool Dynamixel::bCheckTHCurrent(){
    int16_t l_sCurrent= this->sGetCur();
    if(g_bDisableFlag){
      if(abs(l_sCurrent)< 40){             // packet자체가 이상하다고 봐야함 current는 일반적으로 10이하
        if(THRESHOLD_CURRENT<abs(l_sCurrent)){
          g_ucCount++;
        }
        else
          g_ucCount=0;
      }
      if(g_ucCount >= 3){
        g_bDisableFlag = false;
      }
      else{
        return true;
      }
    }
    else{
      this->xSetHomingFlag(HOMING_EMER_EXIT);
    }
  return true;
}


uint8_t Dynamixel::ucGetTorqueStat(){
  return this->_dynamixControl->ucGetTorqueFlag();
}

void Dynamixel::xSetCurrent(int16_t sCurrent){
  this->_dynamixControl->xSetCurCurrent(sCurrent);
  this->_dynamixHandler->xSetUpdatePacketFlag(false);
}

int16_t Dynamixel::sGetCurrent(){
  return this->_dynamixControl->sGetCurCurrent();
}

uint32_t Dynamixel::uiGetTargetAddr(){
  return this->_dynamixProtocol->uiGetTargetAddr();

}

bool Dynamixel::bGetPhotoFlag(){
  return this->_bPhotoSenor;
}

bool Dynamixel::bGetTorque(){
  uint8_t* l_pucTempPackptr = this->_dynamixHandler->pucGetRxPackets();
  if(this->uiGetTargetAddr() == TORQUE_ADDR){
    bool l_bTempTorque = (l_pucTempPackptr[9] & 1);
    return l_bTempTorque;
  }
  return this->ucGetTorqueStat();
}

uint8_t Dynamixel::ucGetPeriodTarget(){
  return this->_dynamixControl->ucGetPeriodTarget();
}


int32_t Dynamixel::iGetCurrentPosition() {
  //return this->_dynamixControl->iGetCurPos();
  return iGetPositionFromRxPacket();
}

void Dynamixel::xSetTargetAddr(uint16_t usTargetAddr){
  this->_dynamixHandler->xSetTargetAddr(usTargetAddr);
}

void Dynamixel::xSetUpdateFlag(bool bUpdateFlag){
  this->_dynamixHandler->xSetUpdatePacketFlag( bUpdateFlag);
}

void Dynamixel::Dxl_Init(){
   this->xTransmitDxlWPacket(TORQUE_ADDR,0,1);
}
  
void Dynamixel::xSetTorqueStat(uint8_t ucTorqueStat){
  this->_dynamixControl->xSetToqueStatFlag(ucTorqueStat);
}

void Dynamixel::xSetContFlag(bool bContFlag){
  this->_dynamixControl->xSetContFlag(bContFlag);
}

bool Dynamixel::bGetContFlag(){
  return this->_dynamixControl->bGetContFlag();
}

void Dynamixel::xSetTxPacketZero(){
  this->m_DynamixelTxPackets.xSetPackZero();
}

void Dynamixel::xSetTorqueOnOff(bool bTorqueflag){
  this->_dynamixControl->xSetTorqueOnOff(bTorqueflag);
}

bool Dynamixel::bGetTorqueOnOff(){
  return this->_dynamixControl->bGetTorqueOnOff();
}


void Dynamixel::xSetCur(int16_t sCur){
  this->_sCurrent = sCur;
}
  
void Dynamixel::xSetPos(int32_t iPos){
  this->_iPosition = iPos;
}

int16_t Dynamixel::sGetCur(){
  return this->_sCurrent;
}

int32_t Dynamixel::iGetPos(){
  return this->_iPosition;
}

uint8_t Dynamixel::ucGetInst(){
  return this->_dynamixProtocol->ucGetInstruction();
}

uint8_t Dynamixel::ucGetId(){
  return this->_dynamixProtocol->ucGetId();
}

uint8_t Dynamixel::ucGetIdLen(){
  return this->_dynamixProtocol->ucGetPSynIdLen();
}