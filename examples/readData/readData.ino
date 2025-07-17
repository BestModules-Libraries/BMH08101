/*************************************************
File:       		  readData
Description:      1.SoftwareSerial interface (BAUDRATE 38400) is used to communicate with mySpo2.
                  2.hardware Serial (BAUDRATE 9600) is used to communicate with Serial port monitor. 
Note:
**************************************************/
#include "BMH08101.h"
BMH08101 mySpo2(5,4);        //rxPin,txPin,Please comment out this line of code if you don't use SW Serial
//BMH08101 mySpo2(&Serial1);     //Please uncomment out this line of code if you use HW Serial1 on BMduino
//BMH08101 mySpo2(&Serial2);   //Please uncomment out this line of code if you use HW Serial2 on BMduino
// BMH08101 mySpo2(&Serial3);  //Please uncomment out this line of code if you use HW Serial3 on BMduino
// BMH08101 mySpo2(&Serial4);  //Please uncomment out this line of code if you use HW Serial4 on BMduino
uint8_t rBuf[18]={0};
uint8_t Status=0;
uint8_t flag=0;

void setup()
{
  Serial.begin(9600); // start serial for output
  mySpo2.begin();
  mySpo2.setRequestSendMode();//Request-Send Mode
  //mySpo2.setContinuousTransMode();//Continuous Trans Mode
  Serial.println("Please place your finger."); 
  delay(2000);  //Wait for finger placement
  mySpo2.beginMeasure();

}
void loop()
{
  /* Request-Send Mode  */
  Status= mySpo2.requestInfoPackage(rBuf);
  if (Status==0x03)
  {
    Serial.println("Measurement completed,Can remove finger."); 
    Serial.print("SpO2:"); 
    Serial.print(rBuf[3],DEC);
    Serial.println("%"); 
    Serial.print("Heart rate:"); 
    Serial.print(rBuf[4],DEC);
    Serial.println("BMP"); 
    Serial.print("PI:"); 
    Serial.print((float)rBuf[5] / 10);
    Serial.println("%");
    mySpo2.endMeasure(); //stop Measure
    mySpo2.sleep();   //enter Halt
  }
  if (Status==0x02&&flag!=1)
  {
      Serial.println("Don't move your finger.");
      flag=1;
  }
  if (Status==0x01&&flag!=0)
  {
      Serial.println("Please reposition your finger.");
      flag=0;
  }

  /* Continuous Trans Mode  */
  // if (mySpo2.isInfoAvailable() == true) // Scaning the serial port received buffer to receive the information sent by the module
  // {
  //   mySpo2.readInfoPackage(rBuf);
  //   for(uint8_t i=0;i<18;i++)
  //   {
  //     Serial.print(i);
  //     Serial.print(":");
  //     Serial.println(rBuf[i]);  
  //   }
  // }
}
