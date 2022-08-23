#include "PhotoSensor.h"

PhotoSensor::PhotoSensor(){
  this->_bPhotoFlag= true;
  this->_bStartPhotoFlag= false;
}

bool PhotoSensor::bGetPhotoFlag(){
  return this->_bPhotoFlag;
}

void PhotoSensor::xSetPhotoFlag(bool bPhotoFlag){
  this->_bPhotoFlag = bPhotoFlag;
}

bool PhotoSensor::bGetStartPhotoFlag(){
  return this->_bStartPhotoFlag;
}

void PhotoSensor::xSetStartPhotoFlag(bool bStartFlag){
  this->_bStartPhotoFlag = bStartFlag;
}
