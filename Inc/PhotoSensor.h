#pragma once


class PhotoSensor {
private:
  bool _bPhotoFlag;
  bool _bStartPhotoFlag;
public:

private:
  
public:
  PhotoSensor();
  bool bGetPhotoFlag();
  void xSetPhotoFlag(bool bPhotoFlag);
  bool bGetStartPhotoFlag();
  void xSetStartPhotoFlag(bool bStartFlag);
};