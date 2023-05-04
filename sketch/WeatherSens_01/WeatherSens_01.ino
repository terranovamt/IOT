#include "HT_SSD1306Wire.h"
#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define SEALEVELPRESSURE_HPA (1013.25)

SSD1306Wire  display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED); // addr , freq , i2c group , resolution , rst
Adafruit_BME280 bme;
 
float temperature, humidity, pressure, altitude;

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
void fillRect(void) {
  uint8_t color = 1;
  for (int16_t i=0; i<display.getHeight()/2; i+=3) {
    display.setColor((color % 2 == 0) ? BLACK : WHITE); // alternate colors
    display.fillRect(i, i, display.getWidth() - i*2, display.getHeight() - i*2);
    display.display();
    delay(10);
    color++;
  }
  // Reset back to WHITE
  display.setColor(WHITE);
}

// Adapted from Adafruit_SSD1306
void drawCircle(void) {
  for (int16_t i=0; i<display.getHeight(); i+=2) {
    display.drawCircle(display.getWidth()/2, display.getHeight()/2, i);
    display.display();
    delay(10);
  }
  delay(1000);
  display.clear();

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
    delay(500);
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

void setup() {
  pinMode(35, OUTPUT); // Set GPIO35 as digital output pin
  delay(100);

  display.init();
  display.clear();
  display.display();
  display.setContrast(255);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  drawRect();
  delay(500);
  drawRectIvert();
  delay(2000);
  display.clear();

  delay(1000); 

  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.clear();
  display.display();
  display.screenRotate(ANGLE_0_DEGREE);
  display.setFont(ArialMT_Plain_16);
  display.drawString(display.getWidth()/2, display.getHeight()/2-16/2, "SETUP DONE");
  display.display();
  delay(2000);

  display.setFont(ArialMT_Plain_10);

  print("scan start");

  delay(2000);

}

void loop() { 

  display.clear();

  digitalWrite(35, HIGH); // Set GPIO35 active high
  delay(1000);  // delay of one second
  digitalWrite(35, LOW); // Set GPIO35 active low
  delay(1000); // delay of one second
    
 
  print("LOOP start");

  delay(1000);
  print("LEO");
  delay(1000);

    // WiFi.scanNetworks will return the number of networks found
    int n = WiFi.scanNetworks();
    display.println("scan done");
    if (n == 0) {
        display.println("no networks found");
    } else {
        display.print(n);
        display.println(" networks found");
        for (int i = 0; i < n; ++i) {
            // Print SSID and RSSI for each network found
            display.print(i + 1);
            display.print(": ");
            display.print(WiFi.SSID(i));
            display.print(" (");
            display.print(WiFi.RSSI(i));
            display.print(")");
            display.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
            delay(10);
        }
    }
    display.println("");

    // Wait a bit before scanning again
    delay(5000);
  
  
}