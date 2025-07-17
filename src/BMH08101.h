/*************************************************
File:       	       BMH08101.h
Author:              BEST MODULES CORP.
Description:         Define classes and required variables
Version:             V1.0.1   --2025-06-11
**************************************************/
#ifndef _BMH08101_H__
#define _BMH08101_H__

#include <Arduino.h>
#include <SoftwareSerial.h>

#define START_MEASURE   0x00
#define STOP_MEASURE    0x01
#define ENTER_HALT      0x02
#define CHECK_OK        0
#define CHECK_ERROR     1
#define TIMEOUT_ERROR   2

/*  Oximetry data */

class BMH08101
{
public:
   BMH08101(HardwareSerial *theSerial = &Serial);
   BMH08101(uint16_t rxPin,uint16_t txPin);
   void begin();
   uint8_t beginMeasure();
   uint8_t endMeasure();
   uint8_t sleep();
   uint8_t updateScalingFactor();
   uint8_t requestInfoPackage(uint8_t buff[]);
   bool isInfoAvailable();
   void readInfoPackage(uint8_t buff[]);
   uint8_t setRequestSendMode();
   void setContinuousTransMode();
   
private:
   HardwareSerial *_serial = NULL;
   SoftwareSerial *_softSerial = NULL;
   uint16_t _rxPin;
   uint16_t _txPin;
   uint16_t _checksum;
   uint8_t _recBuf[18] = {0}; // Array for storing received data
   void writeBytes(uint8_t wbuf[], uint8_t wlen);
   uint8_t readBytes(uint8_t rbuf[], uint8_t rlen, uint16_t timeout);  
};

#endif
