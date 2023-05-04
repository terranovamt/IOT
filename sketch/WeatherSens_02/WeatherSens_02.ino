#include "HT_SSD1306Wire.h"
#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

#define SEALEVELPRESSURE_HPA (1013.25)
#define BMP_SCK  (48)
#define BMP_MISO (34)
#define BMP_MOSI (47)
#define BMP_CS   (33)

SSD1306Wire  display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED); // addr , freq , i2c group , resolution , rst
Adafruit_BMP280 bmp(BMP_CS, BMP_MOSI, BMP_MISO,  BMP_SCK);
 
float temperature, pressure, altitude;

// Adapted from Adafruit_SSD1306
void drawRect(void) {
  for (int16_t i=0; i<display.getHeight()/2; i+=2) {
    display.drawRect(i, i, display.getWidth()-2*i, display.getHeight()-2*i);
    display.display();
    delay(10);
  }
}

void drawRectIvert(void) {
  for (int16_t i=1; i<display.getHeight()/2; i+=2) {
    display.drawRect(i, i, display.getWidth()-2*i, display.getHeight()-2*i);
    display.display();
    delay(10);
  }
}

// Adapted from Adafruit_SSD1306
void drawCircle(void) {
  // This will draw the part of the circel in quadrant 1
  // Quadrants are numberd like this:
  //   0010 | 0001
  //  ------|-----
  //   0100 | 1000
  //
  display.drawCircleQuads(display.getWidth()/2, display.getHeight()/2, display.getHeight()/4, 0b00000001);
  display.display();
  delay(200);
  display.drawCircleQuads(display.getWidth()/2, display.getHeight()/2, display.getHeight()/4, 0b00000011);
  display.display();
  delay(200);
  display.drawCircleQuads(display.getWidth()/2, display.getHeight()/2, display.getHeight()/4, 0b00000111);
  display.display();
  delay(200);
  display.drawCircleQuads(display.getWidth()/2, display.getHeight()/2, display.getHeight()/4, 0b00001111);
  display.display();
}


void print(char* buffer) {
  // Initialize the log buffer
  // allocate memory to store 8 lines of text and 30 chars per line.
  display.setLogBuffer(4, 30);
  
  display.clear();
    // Print to the screen
    display.println(buffer);
    // Draw it to the internal screen buffer
    display.drawLogBuffer(0, 0);
    // Display it on the screen
    display.display();
    delay(100);
}

void displaysetup() {

  display.init();
  display.clear();
  display.display();
  display.setContrast(255);

  drawRect();
  delay(500);
  drawRectIvert();
  delay(1000);
  display.clear();
  delay(1000); 

  drawCircle();
  delay(500);
  display.clear();
  
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.clear();
  display.display();
  display.screenRotate(ANGLE_0_DEGREE);
  display.setFont(ArialMT_Plain_16);
  display.drawString(display.getWidth()/2, display.getHeight()/2-16/2, "SETUP DONE");
  display.display();
  delay(2000);

  display.setFont(ArialMT_Plain_10);
}

void sensorsetup(){

  unsigned status = bmp.begin(0x77);

  if (!status) {
    print("ERROR SENSOR");
    while (1) delay(10);
  }

  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
}


void setup() {

  pinMode(35, OUTPUT); // Set GPIO35 as digital output pin
  delay(100);

  sensorsetup();
  displaysetup();

  
}

void loop() { 

  display.clear();
  display.display();

  /*   digitalWrite(35, HIGH); // Set GPIO35 active high
  delay(100);  // delay of one second
  digitalWrite(35, LOW); // Set GPIO35 active low
  delay(500); // delay of one second */
  drawCircle();
  delay(500);
 
  print("MEASUREMENT");

  // Print SSID and RSSI for each network found
    display.drawString(0, 17,("Temperature = "));
    display.drawString((14)*5,17, (String)(bmp.readTemperature()));
    display.drawString((19)*5,17, (" C"));
    display.drawString(0, 26,("Pressure = "));
    display.drawString(10*5, 26, (String)(bmp.readPressure()));
    display.drawString(20*5, 26, (" Pa"));
    display.drawString(0, 35,("Altitude = "));
    display.drawString(9*5, 35, (String)(bmp.readAltitude(SEALEVELPRESSURE_HPA)));
    display.drawString(14*5, 35, (" m"));
    delay(10);
    display.display();
    
  for (int i=1; i < 500; i++) {
    int progress = (i / 5) % 100;
    // draw the progress bar
    display.drawProgressBar(0,53, 120, 10, progress);
    display.display();
    delay(10);
  }
  
}