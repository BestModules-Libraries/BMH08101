/*************************************************
File:       	    BMH08101.cpp
Author:           BEST MODULES CORP.
Description:      UART communication with the BMH08101 and obtain the corresponding value
Version:          V1.0.1   --2025-06-11
**************************************************/
#include "BMH08101.h"

/*************************************************
Description:  Constructor
Parameters:    *theSerial  : UART object if your board has more than one UART interface            
Return:             
Others:             
*************************************************/
BMH08101::BMH08101(HardwareSerial *theSerial)
{
  _softSerial = NULL;
  _serial = theSerial;
}
/************************************************
Description:  Constructor
Parameters:   rxPin : Receiver pin of the UART  
              txPin : transmit pin of the UART          
Return:            
Others:           
*************************************************/
BMH08101::BMH08101(uint16_t rxPin,uint16_t txPin)
{
  _serial = NULL;
  _rxPin = rxPin;
  _txPin = txPin;
  _softSerial = new SoftwareSerial(_rxPin,_txPin);
}
/*************************************************
Description:  Module Initial
Parameters:                      
Return:             
Others:             
*************************************************/
void BMH08101::begin()
{
  if (_softSerial != NULL)
  {
    _softSerial->begin(38400); 
  }
  else
  {
    _serial->begin(38400); 
  }
  delay(250);
}
/*************************************************
Description:  beginMeasure
Parameters:   void
Return:     
        0 - Success
        1 - Failure       
Others:           
*************************************************/
uint8_t BMH08101::beginMeasure()
{
  uint8_t rxBuf[5]={0};
  uint8_t txBuf[5]={0x55,0xb1,0x00,0xb1,0xaa};
  writeBytes(txBuf,5);
  delay(50);
  if (readBytes(rxBuf, 5, 30)==0x00 && rxBuf[0] == 0x55 && rxBuf[2] == 0x00 && rxBuf[4] == 0xaa)
  {
    return 0;
  }
  else
  {
    return 1;
  }
}
/*************************************************
Description:  endMeasure
Parameters:   void
Return:     
        0 - Success
        1 - Failure       
Others:           
*************************************************/
uint8_t BMH08101::endMeasure()
{
  uint8_t rxBuf[5]={0};
  uint8_t txBuf[5]={0x55,0xb1,0x01,0xb2,0xaa};
  writeBytes(txBuf,5);
  delay(50);
  if (readBytes(rxBuf, 5, 30)==0x00 && rxBuf[0] == 0x55 && rxBuf[2] == 0x01 && rxBuf[4] == 0xaa)
  {
    return 0;
  }
  else
  {
    return 1;
  }
}
/*************************************************
Description:  sleep
Parameters:   void
Return:     
        0 - Success
        1 - Failure       
Others:           
*************************************************/
uint8_t BMH08101::sleep()
{
  uint8_t rxBuf[5]={0};
  uint8_t txBuf[5]={0x55,0xb1,0x02,0xb3,0xaa};
  writeBytes(txBuf,5);
  delay(50);
  if (readBytes(rxBuf, 5, 30)==0x00 && rxBuf[0] == 0x55 && rxBuf[2] == 0x02 && rxBuf[4] == 0xaa)
  {
    return 0;
  }
  else
  {
    return 1;
  }
}
/*************************************************
Description:  update Scaling Factor
Parameters:   void
Return:     
        0 - Success
        1 - Failure       
Others:           
*************************************************/
uint8_t BMH08101::updateScalingFactor()
{
  uint8_t rxBuf[5]={0};
  uint8_t txBuf[5]={0x55,0xb1,0x03,0xb4,0xaa};
  writeBytes(txBuf,5);
  delay(50);
  if (readBytes(rxBuf, 5, 30)==0x00 && rxBuf[0] == 0x55 && rxBuf[2] == 0x03 && rxBuf[4] == 0xaa)
  {
    return 0;
  }
  else
  {
    return 1;
  }
}

/*************************************************
Description:  Judge the module measurement status and read the data.      
Parameters:   BUFF[]:18 byte
Return:       0: Sensors error 
              1: No finger detected
              2: finger detected but measurement not completed.
              3: finger detected and measurement completed.
              0xff : Other
Others:
*************************************************/
uint8_t BMH08101::requestInfoPackage(uint8_t buff[])
{
  uint8_t returnFlag = 0;
  uint8_t txBuf[5]={0x55,0xb1,0x04,0xb5,0xaa};
  writeBytes(txBuf,5);
  delay(50);
  if (readBytes(buff, 18, 30)==0x00 && buff[0] == 0x55 && buff[1] == 0xb0 && buff[17] == 0xaa)
  {
    returnFlag = buff[2];
  }
  else
  {
    return 0xff;
  }
  return returnFlag;
}

/**********************************************************
Description: Query whether the 18-byte data sent by the module is received
Parameters: void
Return: true(1): 18-byte data received
        false(0): 18-byte data not received
Others: Only used in the mode of Tx Auto Output Info
**********************************************************/
bool BMH08101::isInfoAvailable()
{
  uint8_t header[2] = {0x55, 0xB0}; // Fixed code for first 2 bytes of 15-byte data
  uint8_t recBuf[18] = {0}, recLen = 18;
  uint8_t i, num = 0, readCnt = 0, failCnt = 0, checkCode = 0;
  bool isHeader = false, result = false;

  /* Select hardSerial or softSerial according to the setting */
  if (_softSerial != NULL)
  {
    num = _softSerial->available();
  }
  else if (_serial != NULL)
  {
    num = _serial->available();
  }

  /* Serial buffer contains at least one 18-byte data */
  if (num >= recLen)
  {
    while (failCnt < 2) // Didn't read the required data twice, exiting the loop
    {
      /* Find 2-byte data header */
      for (i = 0; i < 2;)
      {
        if (_softSerial != NULL)
        {
          recBuf[i] = _softSerial->read();
        }
        else if (_serial != NULL)
        {
          recBuf[i] = _serial->read();
        }

        if (recBuf[i] == header[i])
        {
          isHeader = true; // Fixed code is correct
          i++;             // Next byte
        }
        else if (recBuf[i] != header[i] && i > 0)
        {
          isHeader = false; // Next fixed code error
          failCnt++;
          break;
        }
        else if (recBuf[i] != header[i] && i == 0)
        {
          readCnt++; // 0x55 not found, continue
        }
        if (readCnt >= (num - 2))
        {
          readCnt = 0;
          isHeader = false; // Fixed code not found
          break;
        }
      }
      /* Find the correct fixed code */
      if (isHeader)
      {
        checkCode = recBuf[1]; // Sum checkCode
        for (i = 2; i < recLen; i++) // Read subsequent 16-byte data
        {
          if (_softSerial != NULL)
          {
            recBuf[i] = _softSerial->read();
          }
          else if (_serial != NULL)
          {
            recBuf[i] = _serial->read();
          }
          checkCode += recBuf[i]; // Sum checkCode
        }
        checkCode = checkCode - recBuf[recLen - 1] - recBuf[recLen - 2];

        /* Compare whether the check code is correct */
        if (checkCode == recBuf[recLen - 2])
        {
          for (i = 0; i < recLen; i++)
          {
            _recBuf[i] = recBuf[i]; // True, assign data to _recBuf[]
          }
          result = true;
          break; // Exit "while (failCnt < 2)" loop
        }
        else
        {
          failCnt++; // Error, failCnt plus 1, return "while (failCnt < 2)" loop
          checkCode = 0;
        }
      }
    }
  }
  return result;
}
/**********************************************************
Description: Read the 18-byte data of sent by the module
Parameters: array: The array for storing the 18-byte module information
                  (refer to datasheet for meaning of each bit)
Return: void
Others: Use after isInfoAvailable()
**********************************************************/
void BMH08101::readInfoPackage(uint8_t buff[])
{
  for (uint8_t i = 0; i < 18; i++)
  {
    buff[i] = _recBuf[i];
  }
}
/*************************************************
Description:  set Request-Send Mode
Parameters:   void   
Return: 
        0 - Success
        1 - Failure   
Others: 
*************************************************/
uint8_t BMH08101::setRequestSendMode()
{
  uint8_t rxBuf[18]={0};
  uint8_t txBuf[5]={0x55,0xb1,0x04,0xb5,0xaa};
  writeBytes(txBuf,5);
  delay(50);
  if (readBytes(rxBuf, 18, 30)==0x00 && rxBuf[0] == 0x55 && rxBuf[1] == 0xb0 && rxBuf[17] == 0xaa)
  {
    return 0;
  }
  else
  {
    return 1;
  }
}
/*************************************************
Description:  set continuous transmission mode
Parameters:   void   
Return:       void 
Others:           
*************************************************/
void BMH08101::setContinuousTransMode()
{
  uint8_t txBuf[5]={0x55,0xb1,0x05,0xb6,0xaa};
  writeBytes(txBuf,5);
  delay(50);
}


/**********************************************************
Description: Read data through UART
Parameters: rbuf: Used to store received data
            rlen: Length of data to be read
            timeout：timeout time
Return:  0:OK
         1:CHECK_ERROR
         2: timeout error
Others: void
**********************************************************/
uint8_t BMH08101::readBytes(uint8_t rbuf[], uint8_t rlen, uint16_t timeout)
{
  uint8_t i = 0, delayCnt = 0,checkSum=0;
/* Select SoftwareSerial Interface */
  if (_softSerial != NULL)
  {
    for (i = 0; i < rlen; i++)
    {
      delayCnt = 0;
      while (_softSerial->available() == 0)
      {
        if (delayCnt > timeout)
        {
          return TIMEOUT_ERROR; // Timeout error
        }
        delay(1);
        delayCnt++;
      }
      rbuf[i] = _softSerial->read();
    }
  }
/* Select HardwareSerial Interface */
  else
  {
    for (i = 0; i < rlen; i++)
    {
      delayCnt = 0;
      while (_serial->available() == 0)
      {
        if (delayCnt > timeout)
        {
          return TIMEOUT_ERROR; // Timeout error
        }
        delay(1);
        delayCnt++;
      }
      rbuf[i] = _serial->read();
    }
  }
  /* check Sum */
  for (i = 1; i < (rlen - 2); i++)
  {
    checkSum += rbuf[i];
  }
  if (checkSum == rbuf[rlen - 2])
  {
    return CHECK_OK; // Check correct
  }
  else
  {
    return CHECK_ERROR; // Check error
  }
}



/**********************************************************
Description: Write data through uart
Parameters: wbuf:The array for storing Data to be sent
            wlen:Length of data sent
Return: void
Others: void
**********************************************************/
void BMH08101::writeBytes(uint8_t wbuf[], uint8_t wlen)
{
  /* Select SoftwareSerial Interface */
  if (_softSerial != NULL)
  {
    while (_softSerial->available() > 0)
    {
      _softSerial->read();
    }
    _softSerial->write(wbuf, wlen);
  }
  /* Select HardwareSerial Interface */
  else
  {
    while (_serial->available() > 0)
    {
      _serial->read();
    }
    _serial->write(wbuf, wlen);
  }
}
