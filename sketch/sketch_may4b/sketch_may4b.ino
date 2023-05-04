#include <heltec-eink-modules.h>

#include <heltec.h>
#include <WiFi.h>
#include <Wire.h>  
#include "HT_SSD1306Wire.h"

SSD1306Wire  display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED); // addr , freq , i2c group , resolution , rst


void drawLines() {
  for (int16_t i=0; i<display.getWidth(); i+=4) {
    display.drawLine(0, 0, i, display.getHeight()-1);
    display.display();
    delay(10);
  }
  for (int16_t i=0; i<display.getHeight(); i+=4) {
    display.drawLine(0, 0, display.getWidth()-1, i);
    display.display();
    delay(10);
  }
  delay(250);

  display.clear();
  for (int16_t i=0; i<display.getWidth(); i+=4) {
    display.drawLine(0, display.getHeight()-1, i, 0);
    display.display();
    delay(10);
  }
  for (int16_t i=display.getHeight()-1; i>=0; i-=4) {
    display.drawLine(0, display.getHeight()-1, display.getWidth()-1, i);
    display.display();
    delay(10);
  }
  delay(250);

  display.clear();
  for (int16_t i=display.getWidth()-1; i>=0; i-=4) {
    display.drawLine(display.getWidth()-1, display.getHeight()-1, i, 0);
    display.display();
    delay(10);
  }
  for (int16_t i=display.getHeight()-1; i>=0; i-=4) {
    display.drawLine(display.getWidth()-1, display.getHeight()-1, 0, i);
    display.display();
    delay(10);
  }
  delay(250);
  display.clear();
  for (int16_t i=0; i<display.getHeight(); i+=4) {
    display.drawLine(display.getWidth()-1, 0, 0, i);
    display.display();
    delay(10);
  }
  for (int16_t i=0; i<display.getWidth(); i+=4) {
    display.drawLine(display.getWidth()-1, 0, i, display.getHeight()-1);
    display.display();
    delay(10);
  }
  delay(250);
}

void VextON(void)
{
  pinMode(Vext,OUTPUT);
  digitalWrite(Vext, LOW);
}

void VextOFF(void) //Vext default OFF
{
  pinMode(Vext,OUTPUT);
  digitalWrite(Vext, HIGH);
}

// the setup routine runs once when starts up
void setup(){
  VextON();
  delay(100);
  
  display.init();
  display.clear();
  display.display();

  // Initialize the Heltec ESP32 object
  Heltec.begin(true /*DisplayEnable Enable*/, true /*LoRa Disable*/, true /*Serial Enable*/, true /*PABOOST Enable*/, 470E6 /**/);
  delay(1000);
  display.clear();
  display.println("Setup done By MATTEO");

  pinMode(35, OUTPUT);
}

// the loop routine runs over and over again forever
void loop() {
  display.clear();
  display.println("START loop");
  digitalWrite(35, HIGH); 
  delay(100);  
  digitalWrite(35, LOW); 
  delay(10000); 

}