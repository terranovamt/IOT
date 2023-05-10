#include "HT_SSD1306Wire.h"
#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

/* Put personalized SSID & Password */
const char* ssid = "ESP32";  // Enter SSID here
const char* password = "12345678";  //Enter Password here

/* Put IP Address details */
IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

WebServer server(80);

SSD1306Wire  display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED); // addr , freq , i2c group , resolution , rst

#define SEALEVELPRESSURE_HPA (1020.25) // if negative measure increment sea level HPA
#define BMP_SCK  (26)
#define BMP_MISO (33)
#define BMP_MOSI (48)
#define BMP_CS   (47)

Adafruit_BMP280 bmp(BMP_CS, BMP_MOSI, BMP_MISO, BMP_SCK); //BMP in SPI softwere mode

// Variable to store BMP280 measure
float temperature, pressure, altitude;

// Variable to store the HTTP request
String header;

// Current time
unsigned long currentTime = millis();

// Previous time
unsigned long previousTime = 0; 

// Define timeout time in milliseconds 
const long timeoutTime = 5000;


// Adapted from Adafruit_SSD1306 OLED
void drawRect(void) {
  for (int16_t i=0; i<display.getHeight()/2; i+=2) {
    display.drawRect(i, i, display.getWidth()-2*i, display.getHeight()-2*i);
    display.display();
    delay(10);
  } 
  for (int16_t i=1; i<display.getHeight()/2; i+=2) {
    display.drawRect(i, i, display.getWidth()-2*i, display.getHeight()-2*i);
    display.display();
    delay(10);
  }
}

// Adapted from Adafruit_SSD1306 OLED
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

// Adafruit_SSD1306 OLED routine to print one line
void print(String buffer) {
  // Initialize the log buffer
  // allocate memory to store 4 lines of text and 30 chars per line.
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

// Adafruit_SSD1306 OLED setup and settigns routine
void displaysetup() {

  display.init();
  display.clear();
  display.display();
  display.setContrast(255);

  drawRect();
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

// Sensor setup in SPI mode (4pin)
void sensorsetup(){

  unsigned status = bmp.begin(0x77); // is BMP280 in SPI mode

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

void handle_OnConnect() {
  server.send(200, "text/html", SendHTML()); 
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

String SendHTML(){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr+="<!DOCTYPE html><html>";
  ptr+="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  ptr+="<link rel=\"icon\" href=\"data:,\">";

  // CSS to style the table 
  ptr+="<style>body { text-align: center; font-family: \"Trebuchet MS\", Arial;}";
  ptr+="table { border-collapse: collapse; width:35%; margin-left:auto; margin-right:auto; }";
  ptr+="th { padding: 12px; background-color: #0043af; color: white; }";
  ptr+="tr { border: 1px solid #ddd; padding: 12px; }";
  ptr+="tr:hover { background-color: #bcbcbc; }";
  ptr+="td { border: none; padding: 12px; }";
  ptr+=".sensor { color:white; font-weight: bold; background-color: #bcbcbc; padding: 1px; }";
  
  // Web Page Heading
  ptr+="</style></head><body><h1>ESP32 with BME280</h1>";
  ptr+="<table><tr><th>MEASUREMENT</th><th>VALUE</th></tr>";
  ptr+="<tr><td>Temp. Celsius</td><td><span class=\"sensor\">";
  ptr+= (String) temperature;
  ptr+=" *C</span></td></tr>";  
  ptr+="<tr><td>Pressure</td><td><span class=\"sensor\">";
  ptr+= (String) pressure;
  ptr+=" hPa</span></td></tr>";
  ptr+="<tr><td>Approx. Altitude</td><td><span class=\"sensor\">";
  ptr+= (String) altitude ;
  ptr+=" m</span></td></tr>"; 
  ptr+="</body></html>";
  
  return ptr;
}

void wifisetup(){
  Serial.println("Connecting to ");
  Serial.println(ssid);

  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  Serial.print("Got IP: ");  Serial.println(WiFi.localIP());

  server.on("/", handle_OnConnect);
  server.on("/measure", getMeas);
  server.onNotFound(handle_NotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void getMeas(){
  temperature = bmp.readTemperature();
  pressure = bmp.readPressure() / 100.0F;
  altitude = bmp.readAltitude(SEALEVELPRESSURE_HPA);
  server.send(200, "text/html", SendHTML());
}

void printOledMeas(){

  String T, P, A;

  drawCircle();
  delay(500);

  T+="Temperature = ";
  T+=(String) temperature;
  T+=" C";
    display.drawString(0, 17, T);

  P+="Pressure = ";
  P+=(String) pressure;
  P+=" Pa";
    display.drawString(0, 26, P);
    
  A+="Altitude = ";
  A+=(String) altitude;
  A+=" m";
  display.drawString(0, 35, A);
  
  delay(10);
  display.display();
}

void setup() {

  Serial.begin(115200);

  uint8_t LED1pin = 35;
  pinMode(LED1pin, OUTPUT);; // Set GPIO35 as digital output pin
  delay(100);
  digitalWrite(LED1pin, HIGH);
  delay(500);
  digitalWrite(LED1pin, LOW);

  sensorsetup();
  wifisetup();
  displaysetup();
  
}

void loop() { 

  display.clear();
  display.display();

  print("MEASUREMENT");
  getMeas();
  printOledMeas();

  // draw the progress bar  
  for (int i=1; i < 1000; i++) {
    int progress = (i / 10) % 100;
    display.drawProgressBar(0,53, 120, 10, progress);
    display.display();
    delay(10);
  }
  
}