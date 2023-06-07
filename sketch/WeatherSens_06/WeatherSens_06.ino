#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include <PubSubClient.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include "HT_SSD1306Wire.h"

/* Put personalized SSIDAP & PasswordAP */
const char* ssidAP = "MTESP32";       // Enter SSIDAP here
const char* passwordAP = "12345678";  //Enter PasswordAP here

/* MQTT settings*/
const char* ssid = "iPhone 13 Pro di Matteo";
const char* password = "terranovamtt";
char* mqttServer = "broker.hivemq.com";
int mqttPort = 1883;

/* Put IP Address details */
IPAddress local_ip(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

WebServer server(80);
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

SSD1306Wire display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED);  // addr , freq , i2c , group , resolution , rst

#define SEALEVELPRESSURE_HPA (1020.25)  // if negative measure increment sea level HPA
#define BMP_SCK (26)
#define BMP_MISO (33)
#define BMP_MOSI (48)
#define BMP_CS (47)

Adafruit_BMP280 bmp(BMP_CS, BMP_MOSI, BMP_MISO, BMP_SCK);  //BMP in SPI softwere mode

// Variable to store BMP280 measure
float temperature, pressure, altitude, oldt, oldp, olda;
;

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
  for (int16_t i = 0; i < display.getHeight() / 2; i += 2) {
    display.drawRect(i, i, display.getWidth() - 2 * i, display.getHeight() - 2 * i);
    display.display();
    delay(10);
  }
  for (int16_t i = 1; i < display.getHeight() / 2; i += 2) {
    display.drawRect(i, i, display.getWidth() - 2 * i, display.getHeight() - 2 * i);
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
  display.drawCircleQuads(display.getWidth() / 2, display.getHeight() / 2, display.getHeight() / 4, 0b00000001);
  display.display();
  delay(200);
  display.drawCircleQuads(display.getWidth() / 2, display.getHeight() / 2, display.getHeight() / 4, 0b00000011);
  display.display();
  delay(200);
  display.drawCircleQuads(display.getWidth() / 2, display.getHeight() / 2, display.getHeight() / 4, 0b00000111);
  display.display();
  delay(200);
  display.drawCircleQuads(display.getWidth() / 2, display.getHeight() / 2, display.getHeight() / 4, 0b00001111);
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
  display.drawString(display.getWidth() / 2, display.getHeight() / 2 - 16 / 2, "SETUP DONE");
  display.display();
  delay(2000);

  display.setFont(ArialMT_Plain_10);
}

// Sensor setup in SPI mode (4pin)
void sensorsetup() {

  do{
    display.clear();
    display.display();
    display.drawString(display.getWidth() / 2, display.getHeight() / 2 - 16 / 2, "ERROR SENSOR");
    display.display();
  }
 while (!(bmp.begin(0x77)));// is BMP280 in SPI mode

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

void handle_NotFound() {
  server.send(404, "text/plain", "Not found");
}

String SendHTML() {
#define ms_per_hour 3600000
#define ms_per_min 60000
#define ms_per_sec 1000

  long now = millis();

  int hh = (now / ms_per_hour);
  now -= (hh * ms_per_hour);
  int mm = (now / ms_per_min);
  now -= (mm * ms_per_min);
  int ss = (now / ms_per_sec);

  String tup = (String)hh;
  tup += ":";
  tup += (String)mm;
  tup += ":";
  tup += (String)ss;

  String ptr = "<!DOCTYPE HTML><html><head>";
  ptr += "<title>BMP280 Web Server</title>";
  ptr += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  ptr += "<link rel=\"stylesheet\" href=\"https://use.fontawesome.com/releases/v5.7.2/css/all.css\" integrity=\"sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr\" crossorigin=\"anonymous\">";
  ptr += "<link rel=\"icon\" href=\"data:,\"> <style>";
  ptr += "html {font-family: Arial; display: inline-block; text-align: center;}p {font-size: 1.2rem; margin: 8px;}body {  margin: 0;}h5{margin: 15px;}h4{margin-bottom: 5px;}";
  ptr += ".topnav { overflow: hidden; background-color: #4B1D3F; color: white; font-size: 1.7rem; }";
  ptr += ".content { padding: 20px; }.card { background-color: white; box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5); }";
  ptr += ".cards { max-width: 700px; margin: 0 auto; display: grid; grid-gap: 2rem; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); }";
  ptr += ".reading { font-size: 2.8rem; } p.old { margin: 0px; }";
  ptr += ".card.temperature { color: #0e7c7b; }";
  ptr += ".card.altitude { color: #17bebb; }";
  ptr += ".card.pressure { color: #3fca6b; }";
  ptr += "</style></head><body>";
  ptr += "<div class=\"topnav\"><h3>BMP280 WEB SERVER</h3><h5>Last update: ";
  ptr += tup;
  ptr += "</h5></div>";
  ptr += "<div class=\"content\"><div class=\"cards\">";
  ptr += "<div class=\"card temperature\"><h4><i class=\"fas fa-thermometer-half\"></i> TEMPERATURE</h4><p class=\"old\"><span ><span id=\"temp\">";
  ptr += (String)oldt;
  ptr += "</span> &deg;C</span></p><p><span class=\"reading\"><span id=\"temp\">";
  ptr += (String)temperature;
  ptr += "</span> &deg;C</span></p></div>";
  ptr += "<div class=\"card pressure\"><h4><i class=\"fas fa-angle-double-down\"></i> PRESSURE</h4><p class=\"old\"><span ><span id=\"pres\">";
  ptr += (String)oldp;
  ptr += "</span> hPa</span></p><p><span class=\"reading\"><span id=\"pres\">";
  ptr += (String)pressure;
  ptr += "</span> hPa</span></p></div>";
  ptr += "<div class=\"card altitude\"><h4><i class=\"fas fa-arrow-up\"></i> ALTITUDE</h4><p class=\"old\"><span ><span id=\"alt\">";
  ptr += (String)olda;
  ptr += "</span> m</span></p><p><span class=\"reading\"><span id=\"alt\">";
  ptr += (String)altitude;
  ptr += "</span> m</span></p></div>";
  ptr += "</div></div></body></html>";

  return ptr;
}

void wifisetupAP() {
  Serial.println("Connecting to ");


  WiFi.softAP(ssidAP, passwordAP);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  Serial.print("ssid AccessPoint: ");
  Serial.println(ssidAP);

  server.on("/", handle_OnConnect);
  server.on("/measure", getMeas);
  server.onNotFound(handle_NotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void getMeas() {

  oldt = temperature;
  oldp = pressure;
  olda = altitude;

  temperature = bmp.readTemperature();
  pressure = bmp.readPressure() / 100.0F;
  altitude = bmp.readAltitude(SEALEVELPRESSURE_HPA);
  server.send(200, "text/html", SendHTML());
  printOledMeas();
}

void printOledMeas() {

  String T, P, A;

  T += "Temperature = ";
  T += (String)temperature;
  T += " C";
  display.drawString(0, 17, T);

  P += "Pressure = ";
  P += (String)pressure;
  P += " hPa";
  display.drawString(0, 26, P);

  A += "Altitude = ";
  A += (String)altitude;
  A += " m";
  display.drawString(0, 35, A);

  delay(10);
  display.display();
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Callback - ");
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
  }
  Serial.println(" ");
}

void setupMQTT() {
  mqttClient.setServer(mqttServer, mqttPort);
  mqttClient.setCallback(callback);
}

void reconnect() {
  Serial.println("Connecting to MQTT Broker...");
  print("Connecting to MQTT Broker...");
  while (!mqttClient.connected()) {
    Serial.println("Reconnecting to MQTT Broker..");
    print("Reconnecting to MQTT Broker..");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    Serial.println(clientId);
    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("MQTT Connected.");
      print("MQTT Connected.");
      // subscribe to topic
      mqttClient.subscribe("MTesp32/message");
    }
  }
}

void setup() {

  Serial.begin(115200);

  uint8_t LED1pin = 35;
  pinMode(LED1pin, OUTPUT);
  ;  // Set GPIO35 as digital output pin
  delay(100);
  digitalWrite(LED1pin, HIGH);
  delay(500);
  digitalWrite(LED1pin, LOW);
  delay(500);
  digitalWrite(LED1pin, HIGH);
  delay(500);
  digitalWrite(LED1pin, LOW);

  displaysetup();
  sensorsetup();
  wifisetupAP();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    display.clear();
    display.display();
    display.drawString(display.getWidth() / 2, display.getHeight() / 2 - 16 / 2, "Connect To WiFi");
    display.display();
  }
  Serial.println("");
  Serial.println("Connected to Wi-Fi");
  setupMQTT();
}

void loop() {
  display.clear();
  display.display();

  drawCircle();
  delay(500);

  print("MEASUREMENT");
  getMeas();

  if (!mqttClient.connected())
    reconnect();
  mqttClient.loop();
  char tempString[8];
  dtostrf(temperature, 1, 2, tempString);
  mqttClient.publish("MTesp32/temperature", tempString);
  dtostrf(pressure, 1, 2, tempString);
  mqttClient.publish("MTesp32/pressure", tempString);
  dtostrf(altitude, 1, 2, tempString);
  mqttClient.publish("MTesp32/altitude", tempString);

  // draw the progress bar
  for (int i = 1; i < 1000; i++) {
    server.handleClient();
    int progress = (i / 10) % 100;
    display.drawProgressBar(0, 53, 120, 10, progress);
    display.display();
    delay(10);
  }
}
