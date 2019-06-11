# subwaySign
Code to fetch mta train times and communicate it to sign thru nrf24l01

This project was inspired by the New-York city Subway which has signs in every station keeping passengers updated 
on the arrival time of the next few trains. We all know how frustrating it is to go underground only to find out that
there are no trains coming for the next 20min. Thus to fix this inconvenience I made myself a sign that displays the same information
in the comfort of my home.

This is a three part project where I have a python script running on my desktop computer that fetches the data the MTA provides
online, an arduino uno and a nrf24l01 which will serve as an antenna to wirelessly communicate the information to the sign 
and finally the sign itself is made with an arduino nano, a nrf24l01, a 16x32 rgb led matrix and a tcrt5000.

There are many other ways to build this circuit such as having a raspberry pi in the sign handling both the display and fetching of data or
having the python script store the MTA data in a sql database and have an esp8266 fetching the data from our server. 
The reason I used an nrf24l01 is because I have different devices accross my home all link to my computer which acts as a hub.

If you wish to find out more about the hardware used in this project you can review:
nrf24l01 at https://howtomechatronics.com/tutorials/arduino/arduino-wireless-communication-nrf24l01-tutorial/
rgb ledmatrix at https://learn.adafruit.com/32x16-32x32-rgb-led-matrix/test-example-code
tcrt5000 at https://www.instructables.com/id/Using-IR-Sensor-TCRT-5000-With-Arduino-and-Program/

The arduino libraries required are Adafruit_GFX.h, RGBmatrixPanel.h, SPI.h, nRF24L01.h and RF24.h which can all be downloaded from
the libraries managment tool of arduino idle.

The python libraries used are google.transit, protobuf_to_dict, numpy, requests, time, serial, struct which are included in python 3.7 
or can be downloaded from your command prompt using pip.

