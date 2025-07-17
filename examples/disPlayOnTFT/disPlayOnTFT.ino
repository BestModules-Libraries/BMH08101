/*************************************************
File:       		  disPlayOnTFT
Description:      1.SoftwareSerial interface (BAUDRATE 38400) is used to communicate with mySpo2.
                  2.hardware Serial (BAUDRATE 9600) is used to communicate with Serial port monitor. 
Note:
**************************************************/
#include "BMD58T280.h"
#include "BMH08101.h"
#include "picture.h"
#include <SPI.h>

uint8_t Status=0;
uint8_t Status_Temp=0;
uint8_t Status_Change=0; //1(1-2) 2(2-3) 3(3-1) 4(2-1)

uint8_t rBuf[18]={0};
uint8_t  SpO2=0;
uint8_t  HeartRate=0;
float Pi=0;

uint8_t flag=1;//1:PASS 0:FAIL
uint8_t TimeNum= 0;//Measurement time
uint8_t TimeLine = 0; //Measurement time Line
uint8_t TimeLine_Flag = 0;
uint8_t disPlay_Data_Flag = 0;
uint8_t ResultTimeNum= 8;//Result retention time
uint32_t scanTime = 100; //<=1000
uint32_t lastTime = 0;

uint8_t data_y0 = 5; //Name
uint8_t data_y1 = 5+1*26; //Line:result
uint8_t data_y2 = 5+2*26; //Line:SpO2
uint8_t data_y3 = 5+3*26; //Line:PR
uint8_t data_y4 = 5+4*26; //Line:PI

uint8_t text_y1 = 5+3*26;  //Line:text1
uint8_t text_y2 = 5+4*26;  //Line:text2
uint8_t text_y3 = 5+5*26;  //Line:text3
uint8_t text_y4 = 5+6*26;  //Line:[- -]

BMD58T280 TFTscreen(&SPI);
BMH08101 mySpo2(&Serial1);

void setup()
{
  Status=1;
  TimeNum=0;
  ResultTimeNum= 8;

  Serial.begin(9600);
  mySpo2_Init();

  TFT_Init();
  TFTscreen.text("Please Put Your", 16, text_y1,BM_ILI9341::YELLOW,BM_ILI9341::BLACK);
  TFTscreen.text("Finger.", 5*18, text_y2,BM_ILI9341::YELLOW,BM_ILI9341::BLACK);
  
  mySpo2.beginMeasure();
}

void loop()
{
  Status_Temp = mySpo2.requestInfoPackage(rBuf);
  delay(scanTime);
  
  if((Status == 1) && (Status_Temp == 2)) //Status:1-2
  {
    disPlay_Data_Flag = 1;
    Status = Status_Temp;
    TimeNum=0;
    Status_Change = 1;

    test_init();
    TimeLine = 47+12;
    lastTime = millis();
    
    
  }
  if(Status_Change == 1) //1-2
  {
    if(TimeNum>30)  //30s time out
    {
      //mySpo2.endMeasure(); //stop Measure
      delay(500);
      clearText();
      TFTscreen.text("measure failed!", 16, text_y1,BM_ILI9341::RED,BM_ILI9341::BLACK);
      TFTscreen.text("Please Put Your", 16, text_y2,BM_ILI9341::YELLOW,BM_ILI9341::BLACK);
      TFTscreen.text("Finger again.", 2*18, text_y3,BM_ILI9341::YELLOW,BM_ILI9341::BLACK);

    }
    else
    {
      if((millis()-lastTime) >= 1000 )
      {
        TimeNum++;
        lastTime = millis();
        if((TimeNum!=0)&&(TimeNum%2==0))//Change Line every 2 seconds
        {
          TimeLine_Flag = 1;
        }
      }
      if(TimeLine_Flag == 1) 
      {
        testing();
        TimeLine_Flag = 0;
      }
    }

  }

  
  if((Status == 2) && (Status_Temp == 3)) //Status:2-3
  {
    Status = Status_Temp;
    Status_Change = 2;
    SpO2 = rBuf[3];
    HeartRate = rBuf[4];
    Pi = (float)rBuf[5]/10;
    lastTime = millis();
  }
  if(Status_Change == 2)
  {
    if((TimeNum>=10)&&(disPlay_Data_Flag==1))
    {
      delay(500);
      disPlay_Data();
      disPlay_Data_Flag =0; //Display only once
      Status_Change = 0;
    }
    else
    {
      if((millis()-lastTime) >= 1000 )
      {
        TimeNum ++;
        lastTime = millis();
        if((TimeNum!=0)&&(TimeNum%2==0))//Change Line every 2 seconds
        {
          TimeLine_Flag = 1;
        }
      }
      if(TimeLine_Flag == 1) 
      {
        testing();
        TimeLine_Flag = 0;
      }
    }
  }

  if((Status == 3) && (Status_Temp == 1) ) //Status:3-1
  {
    Status_Change = 3;
    Status = Status_Temp;
    if(disPlay_Data_Flag == 0)//If the data has already been output
    {
       ResultTimeNum=8;
       drawNum(ResultTimeNum,2, 285, data_y4,BM_ILI9341::YELLOW,BM_ILI9341::BLACK);
       TFTscreen.text("s", 303, data_y4,BM_ILI9341::YELLOW,BM_ILI9341::BLACK);
       lastTime = millis();
    }
    else
    {
      ResultTimeNum = 0;
    }
  }
  if((Status_Change == 3)) //3-1
  {
    if(ResultTimeNum==0)
    {
      Status_Change = 0;
      clearText();
      TFTscreen.text("Please Put Your", 16, text_y1,BM_ILI9341::YELLOW,BM_ILI9341::BLACK);
      TFTscreen.text("Finger.", 5*18, text_y2,BM_ILI9341::YELLOW,BM_ILI9341::BLACK);

      Status_Change = 0;
    }
    else
    {
      if((millis()-lastTime) >=1000 )
      {
        ResultTimeNum--;
        drawNum(ResultTimeNum,1, 285, data_y4,BM_ILI9341::YELLOW,BM_ILI9341::BLACK);
        lastTime = millis();
      }
    }
  }

  if((Status == 2) && (Status_Temp == 1)) //Status:2-1
  {
    Status = Status_Temp;
    Status_Change = 4;
    clearText();
    TFTscreen.text("Please Put Your", 16, text_y1,BM_ILI9341::YELLOW,BM_ILI9341::BLACK);
    TFTscreen.text("Finger.", 5*18, text_y2,BM_ILI9341::YELLOW,BM_ILI9341::BLACK);
    TimeNum=0;
  }
  
}

void TFT_Init()
{
  TFTscreen.begin();
  TFTscreen.setRotation(1);//旋轉180°
  TFTscreen.background(BM_ILI9341::BLACK);

  TFTscreen.drawPixels(239,0,80,35, gImage_BM_LOGO);
  TFTscreen.setTextSize(3);
  TFTscreen.noFill();
  TFTscreen.stroke(BM_ILI9341::YELLOW);
  TFTscreen.text("BMH08101", 5*12, data_y0,BM_ILI9341::GREEN,BM_ILI9341::BLACK);
  //TFTscreen.setTextSize(2);
}

void mySpo2_Init()
{
  mySpo2.begin();
  mySpo2.setRequestSendMode();
}

void test_init()
{
    clearText();
    TFTscreen.setTextSize(2);
    TFTscreen.text("[-               -]", 47, text_y4,BM_ILI9341::YELLOW,BM_ILI9341::BLACK);
    TFTscreen.setTextSize(3);
    TFTscreen.text("Don't move finger!", 0, text_y1,BM_ILI9341::YELLOW,BM_ILI9341::BLACK);
    TFTscreen.text("Testing...", 80, text_y2,BM_ILI9341::YELLOW,BM_ILI9341::BLACK);
    
}

void testing()
{
    TimeLine = TimeLine + 12;
    TFTscreen.setTextSize(2);
    TFTscreen.text(">", TimeLine, text_y4,BM_ILI9341::YELLOW,BM_ILI9341::BLACK);
    TFTscreen.setTextSize(3);
    TFTscreen.text("Don't move finger!", 0, text_y1,BM_ILI9341::YELLOW,BM_ILI9341::BLACK);
    TFTscreen.text("Testing...", 80, text_y2,BM_ILI9341::YELLOW,BM_ILI9341::BLACK);
}

void disPlay_Data()
{
  clearText();

  flag = 1;
  if((SpO2<70) || (SpO2>99))
  {
    flag = 0;
  }
  if((HeartRate<30) || (HeartRate>250))
  {
    flag = 0;
  }
  if((Pi<0.5) || (Pi>25))
  {
    flag = 0;
  }

  if(flag != 0)
  {
    //TFTscreen.text("+++ Result", 2*18, data_y1,BM_ILI9341::YELLOW,BM_ILI9341::BLACK);

    TFTscreen.text("SpO2: ", 18, data_y2,BM_ILI9341::YELLOW,BM_ILI9341::BLACK);
    drawNum(SpO2,2, 18+6*18, data_y2,BM_ILI9341::YELLOW,BM_ILI9341::BLACK);
    TFTscreen.text("%", 18+9*18, data_y2,BM_ILI9341::YELLOW,BM_ILI9341::BLACK);

    TFTscreen.text("PR  : ", 18, data_y3,BM_ILI9341::YELLOW,BM_ILI9341::BLACK);
    drawNum(HeartRate,3, 18+6*18, data_y3,BM_ILI9341::YELLOW,BM_ILI9341::BLACK);
    TFTscreen.text("bpm", 18+9*18, data_y3,BM_ILI9341::YELLOW,BM_ILI9341::BLACK);

    TFTscreen.text("PI  : ", 18, data_y4,BM_ILI9341::YELLOW,BM_ILI9341::BLACK);
    int8_t Pi_10 = Pi*10;
    int8_t Pi_Ten = (uint8_t)Pi_10/10%10;
    drawNum(Pi_Ten,1, 18+6*18, data_y4,BM_ILI9341::YELLOW,BM_ILI9341::BLACK);
    TFTscreen.text(".", 18+7*18, data_y4,BM_ILI9341::YELLOW,BM_ILI9341::BLACK);
    int8_t Pi_Point = (uint8_t)Pi_10/1%10;
    drawNum(Pi_Point,1, 18+8*18, data_y4,BM_ILI9341::YELLOW,BM_ILI9341::BLACK);
    TFTscreen.text("%", 18+9*18, data_y4,BM_ILI9341::YELLOW,BM_ILI9341::BLACK);

  }
  else
  {
      TFTscreen.text("measure failed!", 16, text_y1,BM_ILI9341::YELLOW,BM_ILI9341::BLACK);
      TFTscreen.text("Please Put Your Finger again.", 16, text_y2,BM_ILI9341::YELLOW,BM_ILI9341::BLACK);
      TFTscreen.text("Finger again.", 2*18, text_y3,BM_ILI9341::YELLOW,BM_ILI9341::BLACK);
  }
}

void drawNum(uint32_t num, uint8_t numLen, int16_t x, int16_t y, uint16_t textcolor, uint16_t textbgcolor)
{
  String numString = String(num);
  char stringBuff[20]; // Character array to be printed to the screen
  numString.toCharArray(stringBuff,numLen+1);
  TFTscreen.text(stringBuff, x, y,textcolor,textbgcolor);
}
void clearText()
{
  TFTscreen.text("             ", 0, data_y1,BM_ILI9341::YELLOW,BM_ILI9341::BLACK);
  TFTscreen.text("             ", 0, data_y2,BM_ILI9341::YELLOW,BM_ILI9341::BLACK);
  TFTscreen.text("             ", 0, data_y3,BM_ILI9341::YELLOW,BM_ILI9341::BLACK);
  TFTscreen.text("             ", 0, data_y4,BM_ILI9341::YELLOW,BM_ILI9341::BLACK);
  TFTscreen.text("                  ", 0, text_y1,BM_ILI9341::YELLOW,BM_ILI9341::BLACK);
  TFTscreen.text("                  ", 0, text_y2,BM_ILI9341::YELLOW,BM_ILI9341::BLACK);
  TFTscreen.text("                  ", 0, text_y3,BM_ILI9341::YELLOW,BM_ILI9341::BLACK);
  TFTscreen.text("                  ", 0, text_y4,BM_ILI9341::YELLOW,BM_ILI9341::BLACK);
}

