#include "Adafruit_GFX.h"
#include "RGBmatrixPanel.h"
#include "SPI.h"
#include "nRF24L01.h"
#include "RF24.h"             //First we import the necessary libraries

RF24 radio(9, 10);            // CE, CSN for nrf24l01

const byte address[][6] = {"00001", "00002"};   //we set the address the nrf24 will use to communicate

#define CLK 8 
#define LAT A3
#define OE A4
#define A A0
#define B A1
#define C A2

RGBmatrixPanel matrix(A, B, C, CLK, LAT, OE, false);    //We set the 16x32 rgb led matrix we have on our sign 

const char trains[4] = {'A','B','C','D'};
const char dir[2][4] ={
  {'U','p',' ',' '},
  {'D','o','w','n'}  
};
const byte trainColor[4][3] = {
  {0, 0, 7},
  {7, 2, 0},
  {0, 0, 7},
  {7, 2, 0}
};

byte trainTime[16] = {99, 99, 99, 99,  99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99}; //create a variable where the time of the next trains will be stored 

const byte infoSize = 16;
unsigned long activeTimer = millis(); //set a timer so that the sign turns off automatically
byte onOff = 1;
const byte IRsensor = A5;             //TRCT5000 used to turn the sign on and off
int dataBK = 100;

void setup() 
{
  radio.begin();
  radio.openWritingPipe(address[1]);
  radio.openReadingPipe(1, address[0]);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();
  
  matrix.begin();
  matrix.setTextSize(1);    

  delay(500);
  pinMode(IRsensor, INPUT);
  waitingData();            //We wait to receive our first data before displaying anything
}

void loop() 
{
  while(onOff == 1)
  {
    trainTimeDisplay();
  }
  while(onOff != 1)
  {
    matrix.fillRect(0, 0, 32, 16, matrix.Color333(0, 0, 0));
    presence();
    delay(20);
    sendMode();
  }
  waitingData();
}

void waitingData()
{
  matrix.setCursor(1, 0);
  matrix.setTextColor(matrix.Color333(0,4,4));
  matrix.print('s');
  matrix.print('y');
  matrix.print('n');
  matrix.print('c');
  
  while(radio.available() == 0)
  {
    sendMode();
    delay(250);
  }
  radio.read(&trainTime, sizeof(trainTime));
  matrix.fillRect(0, 0, 32, 16, matrix.Color333(0, 0, 0));
}

void sendMode()
{
  radio.stopListening();
  radio.write(&onOff, sizeof(onOff));
  delay(5);
  radio.startListening();
}

void presence()
{
  int data = analogRead(IRsensor);
  byte result;
  if(data < dataBK - 30)
  {
    onOff = (onOff+1)%2;
    while(data < dataBK - 10)
    {
      data = analogRead(IRsensor);
      delay(10);
    }
    activeTimer = millis();
  }

  if(millis() > activeTimer + 300000)
  {
    onOff = 0;
  }
  
  dataBK = data;
}

void trainTimeDisplay()           // takes the values from trainTime and displays them 
{
  for(byte t = 0; t < 4; t++)
  {
    for(byte d = 0; d < 2; d++)
    {
      byte time1 = trainTime[byte(4*t + d*2)];
      byte time2 = trainTime[byte(4*t + d*2 + 1)];
      
      matrix.setCursor(1, 0);
      matrix.setTextColor(matrix.Color333(trainColor[t][0],trainColor[t][1],trainColor[t][2]));
      matrix.print(trains[t]);

      matrix.setCursor(8, 0);
      for(byte l = 0; l < 4; l++)
      {
        matrix.print(dir[d][l]);
      }
      matrix.setCursor(1, 9);
      matrix.print(trainTime[byte(4*t + d*2)]);     //trainTime[byte(4*t + d*2)]
      matrix.setCursor(20, 9);
      matrix.print(trainTime[byte(4*t + d*2 + 1)]);     //trainTime[byte(4*t + d*2 + 1)]
      
      long timer = millis();
      while(millis() < timer + 4000)          
      {
        if(radio.available())                           //check if new info available 
        {
          radio.read(&trainTime, sizeof(trainTime));
        }
        sendMode();                  //keep python code update on wether the sign is on or off
        presence();                 //checks if someone is waving in front of the sign 
        if(onOff != 1)
        {
          timer -= 4000;
          d = 2;
          t = 4;
        }
        delay(20);
      }
      
      matrix.fillRect(6, 0, 32, 8, matrix.Color333(0, 0, 0));
      matrix.fillRect(0, 8, 32, 16, matrix.Color333(0, 0, 0));
    }
    matrix.fillRect(0, 0, 6, 8, matrix.Color333(0, 0, 0));
  }
}
