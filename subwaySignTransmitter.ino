#include "SPI.h"
#include "nRF24L01.h"
#include "RF24.h"

RF24 radio(7, 8); // CE, CSN
const byte address[][6] = {"00001", "00002"};
const byte infoSize = 16;
byte information[infoSize];
byte check = 0;
byte onOff = 1;


void setup() 
{
  Serial.begin(250000);
  radio.begin();
  radio.openWritingPipe(address[0]);
  radio.openReadingPipe(1, address[1]);
  radio.setPALevel(RF24_PA_MIN);
  //radio.startListening();
  radio.stopListening();
}

void loop() 
{
  if(Serial.available() >= infoSize + 1)
  {
    if(Serial.read() == 100)
    {
      for(byte i = 0; i < infoSize; i++)
      {
        information[i] = Serial.read();
      }
    }
    
    Serial.flush();
    radio.write(&information, sizeof(information)); 
  } 
  
  delay(25);
  long timer = millis();
  radio.startListening();
    
  while(millis() < timer + 475)
  {
    if(radio.available())
    {
      radio.read(&onOff, sizeof(onOff));
      Serial.write(onOff);
    }
  }
  radio.stopListening(); 
}
