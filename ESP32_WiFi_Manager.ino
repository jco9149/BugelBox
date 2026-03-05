#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include "LittleFS.h"
#include "DFRobotDFPlayerMini.h"
#include <LiquidCrystal_I2C.h>
#include "time.h"

#define RXD1 27
#define TXD1 26

#define mySoftwareSerial Serial1
#define BUSY 4

DFRobotDFPlayerMini myDFPlayer;



// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Search for parameter in HTTP POST request
const char* PARAM_INPUT_1 = "ssid";
const char* PARAM_INPUT_2 = "pass";
const char* PARAM_INPUT_3 = "ip";
const char* PARAM_INPUT_4 = "gateway";

//Variables to save values from HTML form
String ssid;
String pass;
String ip;
String gateway;

// File paths to save input values permanently
const char* ssidPath = "/ssid.txt";
const char* passPath = "/pass.txt";
const char* ipPath = "/ip.txt";
const char* gatewayPath = "/gateway.txt";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -18000;
const int   daylightOffset_sec = 3600;

IPAddress localIP;
//IPAddress localIP(192, 168, 1, 200); // hardcoded

// Set your Gateway IP address
IPAddress localGateway;
//IPAddress localGateway(192, 168, 1, 1); //hardcoded
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);   // Critical for NTP resolution
IPAddress secondaryDNS(8, 8, 4, 4);

// Timer variables
unsigned long previousMillis = 0;
const long interval = 10000;  // interval to wait for Wi-Fi connection (milliseconds)

// Set LED GPIO
const int ledPin = 2;
// Stores LED state

String ledState;


// set the LCD number of columns and rows
int lcdColumns = 16;
int lcdRows = 2;

// set LCD address, number of columns and rows
// if you don't know your display address, run an I2C scanner sketch
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);  


// Initialize LittleFS
void initLittleFS() {
  if (!LittleFS.begin(true)) {
    Serial.println("An error has occurred while mounting LittleFS");
  }
  Serial.println("LittleFS mounted successfully");
}

// Read File from LittleFS
String readFile(fs::FS &fs, const char * path){
  Serial.printf("Reading file: %s\r\n", path);

  File file = fs.open(path);
  if(!file || file.isDirectory()){
    Serial.println("- failed to open file for reading");
    return String();
  }
  
  String fileContent;
  while(file.available()){
    fileContent = file.readStringUntil('\n');
    break;     
  }
  return fileContent;
}

// Write file to LittleFS
void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file){
    Serial.println("- failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("- file written");
  } else {
    Serial.println("- write failed");
  }
}

// Initialize WiFi
bool initWiFi() {
  if(ssid=="" || ip==""){
    Serial.println("Undefined SSID or IP address.");
    return false;
  }
  WiFi.mode(WIFI_STA);
  localIP.fromString(ip.c_str());
  localGateway.fromString(gateway.c_str());
  


  if (!WiFi.config(localIP, localGateway, subnet, primaryDNS, secondaryDNS)){
    Serial.println("STA Failed to configure");
    return false;
  }
  WiFi.begin(ssid.c_str(), pass.c_str());
  Serial.println("Connecting to WiFi...");





  unsigned long currentMillis = millis();
  previousMillis = currentMillis;

  while(WiFi.status() != WL_CONNECTED) {
    currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      Serial.println("Failed to connect.");
      return false;
    }
  }

  Serial.println(WiFi.localIP());
  if(WiFi.status() == WL_CONNECTED){
  }
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  return true;
}

// Replaces placeholder with LED state value
String processor(const String& var) {
  if(var == "STATE") {
    if(digitalRead(ledPin)) {
      ledState = "ON";
    }
    else {
      ledState = "OFF";
    }
    return ledState;
  }
  return String();
}



void setup() {
  // Serial port for debugging purposes
  Serial.begin(115200);
  initLittleFS();

  pinMode(4, INPUT_PULLUP);       // GPIO 4 is Busy signal
  Serial1.begin(9600, SERIAL_8N1, RXD1, TXD1);
  delay(3000);
    // Init and get the time
    // initialize LCD
  lcd.init();
    // turn on LCD backlight                      
  lcd.backlight();

  // Set GPIO 2 as an OUTPUT
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  
  // Load values saved in LittleFS
  ssid = readFile(LittleFS, ssidPath);
  pass = readFile(LittleFS, passPath);
  ip = readFile(LittleFS, ipPath);
  gateway = readFile (LittleFS, gatewayPath);
  Serial.println(ssid);
  Serial.println(pass);
  Serial.println(ip);
  Serial.println(gateway);

  if(initWiFi()) {

    // Route for root / web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(LittleFS, "/index.html", "text/html", false, processor);
    });
    server.serveStatic("/", LittleFS, "/");
    
    // Route to set GPIO state to HIGH
    server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request) {
      digitalWrite(ledPin, HIGH);
      request->send(LittleFS, "/index.html", "text/html", false, processor); 
    });

    // Route to set GPIO state to LOW
    server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request) {
      digitalWrite(ledPin, LOW);
      request->send(LittleFS, "/index.html", "text/html", false, processor);
    });

    Serial.println();
    Serial.println(F("\nESP32-S3 DFRobot DFPlayer Mini Demo"));
    Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));

    if (!myDFPlayer.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with mp3.
      Serial.println(F("Unable to begin:"));
      Serial.println(F("1.Please recheck the connection!"));
      Serial.println(F("2.Please insert the SD card!"));
      while (true);
    }
    Serial.println(("DFPlayer Mini online."));
    myDFPlayer.volume(10);  //Set volume value. From 0 to 30
    myDFPlayer.play(1);     //Play the first mp3

  }
  else {
    // Connect to Wi-Fi network with SSID and password
    Serial.println("Setting AP (Access Point)");

    // NULL sets an open Access Point
    WiFi.softAP("ESP-WIFI-MANAGER", NULL);
    
    // print message

    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP addres: ");
    Serial.println(IP); 
    
    lcd.setCursor(1, 0);
    lcd.print("Go to ESP-WIFI");

    lcd.setCursor(2, 1);

    lcd.print(IP);


    // Web Server Root URL
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(LittleFS, "/wifimanager.html", "text/html");
    });
    
    server.serveStatic("/", LittleFS, "/");
    
    server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
      int params = request->params();
      for(int i=0;i<params;i++){
        const AsyncWebParameter* p = request->getParam(i);
        if(p->isPost()){
          // HTTP POST ssid value
          if (p->name() == PARAM_INPUT_1) {
            ssid = p->value().c_str();
            Serial.print("SSID set to: ");
            Serial.println(ssid);
            // Write file to save value
            writeFile(LittleFS, ssidPath, ssid.c_str());
          }
          // HTTP POST pass value
          if (p->name() == PARAM_INPUT_2) {
            pass = p->value().c_str();
            Serial.print("Password set to: ");
            Serial.println(pass);
            // Write file to save value
            writeFile(LittleFS, passPath, pass.c_str());
          }
          // HTTP POST ip value
          if (p->name() == PARAM_INPUT_3) {
            ip = p->value().c_str();
            Serial.print("IP Address set to: ");
            Serial.println(ip);
            // Write file to save value
            writeFile(LittleFS, ipPath, ip.c_str());
          }
          // HTTP POST gateway value
          if (p->name() == PARAM_INPUT_4) {
            gateway = p->value().c_str();
            Serial.print("Gateway set to: ");
            Serial.println(gateway);
            // Write file to save value
            writeFile(LittleFS, gatewayPath, gateway.c_str());
          }
          //Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
        }
      }
      request->send(200, "text/plain", "Done. ESP will restart, connect to your router and go to IP address: " + ip);
      delay(3000);
      ESP.restart();
    });
    server.begin();
    
    
  }

}

void loop() {
  

  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");

    return;
  }
  lcd.clear();
  // set cursor to first column, first row
  lcd.setCursor(5, 0);
  // print message
  lcd.print(&timeinfo, "%H:%M");
  // clears the display to print new message
  // set cursor to first column, second row
  lcd.setCursor(3,1);
  lcd.print(&timeinfo, "%m-%d-%Y");
  delay(1000);
  lcd.clear(); 
  int hour = timeinfo.tm_hour;
  int minute = timeinfo.tm_min;
  int second = timeinfo.tm_sec;
  Serial.println(hour);
  Serial.println(minute);
  Serial.println(second);
  if(hour == 6 && minute == 30 && second == 0){
    myDFPlayer.play(2);
  }
  if(hour == 17 && minute == 00 && second == 0){
    myDFPlayer.play(3);
  }
  if(hour == 21 && minute == 30 && second == 0){
    myDFPlayer.play(4);
  }
}