#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include "LittleFS.h"
#include "DFRobotDFPlayerMini.h"
#include <LiquidCrystal_I2C.h>
#include "time.h"
#include <ESPmDNS.h>

#define RXD1 27
#define TXD1 26

#define mySoftwareSerial Serial1
#define BUSY 4

DFRobotDFPlayerMini myDFPlayer;



// Create AsyncWebServer object on port 80
AsyncWebServer server(80);


const char* PARAM_INPUT_5 = "revellie";
const char* PARAM_INPUT_6 = "retreat";
const char* PARAM_INPUT_7 = "taps";
const char* PARAM_INPUT_8 = "tattoo";
const char* PARAM_INPUT_9 = "anthem";

//Variables to save values from HTML form
String revellie;
String anthem;
String retreat;
String taps;
String tattoo;

bool initLoad = true;

// File paths to save input values permanently
const char* revelliePath = "/revellie.txt";
const char* anthemPath = "/anthem.txt";
const char* retreatPath = "/retreat.txt";
const char* tapsPath = "/taps.txt";
const char* tattooPath = "/tattoo.txt";


const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -18000;
const int   daylightOffset_sec = 3600;

// Timer variables
unsigned long previousMillis = 0;

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

void configModeCallback () {
  ESP.restart();
}


void setup() {
      // Serial port for debugging purposes
  WiFi.mode(WIFI_STA);
  Serial.begin(115200);
  initLittleFS();
  Serial1.begin(9600, SERIAL_8N1, RXD1, TXD1);


  pinMode(4, INPUT_PULLUP);       // GPIO 4 is Busy signal
    // Init and get the time
    // initialize LCD
  lcd.init();
    // turn on LCD backlight                      
  lcd.backlight();

  // Load values saved in LittleFS
  revellie = readFile(LittleFS, revelliePath);
  anthem = readFile(LittleFS, anthemPath);
  retreat = readFile(LittleFS, retreatPath);
  taps = readFile(LittleFS, tapsPath);
  tattoo = readFile (LittleFS, tattooPath);
  Serial.println(revellie);
  Serial.println(anthem);
  Serial.println(retreat);
  Serial.println(taps);
  Serial.println(tattoo);
  
  // Optional, but often required for hostname to stick

    // Set the hostname using the standard WiFi library function
    // This must be called BEFORE the WiFi interface connects
  WiFiManager wm;
  wm.setSaveConfigCallback(configModeCallback);
    // reset settings - wipe stored credentials for testing
    // these are stored by the esp library
  bool res;
  wm.setHostname("bugle"); 
    // res = wm.autoConnect(); // auto generated AP name from chipid
    // res = wm.autoConnect("AutoConnectAP"); // anonymous ap
  res = wm.autoConnect("AutoConnectAP","password"); // password protected ap


  if(!res) {
        Serial.println("Failed to connect");
        initLoad = true;
        wm.resetSettings();
        ESP.restart();
  } 
  else {
    MDNS.begin("bugle");
    if(revellie == ""){
      writeFile(LittleFS, revelliePath, "06:30");

    }

    if(retreat == ""){
      writeFile(LittleFS, retreatPath, "17:00" );

    }
    
    if(taps == ""){
      writeFile(LittleFS, tapsPath, "21:00");
    }
    
    if(tattoo == ""){
      writeFile(LittleFS, tattooPath, "22:00");
    }

    if(anthem == ""){
      writeFile(LittleFS, anthemPath, "12:00");

    }
    
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
        // Route for root / web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(LittleFS, "/index.html", "text/html", false);
    });

    server.on("/getRevellie", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(200, "text/plain", revellie);
    });

    server.on("/getRetreat", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(200, "text/plain", retreat);
    });

    server.on("/getTaps", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(200, "text/plain", taps);
    });

    server.on("/getTattoo", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(200, "text/plain", tattoo);
    });
    server.on("/getAnthem", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(200, "text/plain", anthem);
    });
    server.serveStatic("/", LittleFS, "/");

    server.on("/times", HTTP_POST, [](AsyncWebServerRequest *request) {
      int params = request->params();
      for(int i=0;i<params;i++){
        const AsyncWebParameter* p = request->getParam(i);
        if(p->isPost()){
          // HTTP POST ssid value
          if (p->name() == PARAM_INPUT_5) {
            revellie = p->value().c_str();
            Serial.print("revellie set to: ");
            Serial.println(revellie);
            // Write file to save value
            writeFile(LittleFS, revelliePath, revellie.c_str());
          }
          // HTTP POST pass value
          if (p->name() == PARAM_INPUT_6) {
            retreat = p->value().c_str();
            Serial.print("retreat set to: ");
            Serial.println(retreat);
            // Write file to save value
            writeFile(LittleFS, retreatPath, retreat.c_str());
          }
          // HTTP POST ip value
          if (p->name() == PARAM_INPUT_7) {
            taps = p->value().c_str();
            Serial.print("taps is set to: ");
            Serial.println(taps);
            // Write file to save value
            writeFile(LittleFS, tapsPath, taps.c_str());
          }
          // HTTP POST gateway value
          if (p->name() == PARAM_INPUT_8) {
            tattoo = p->value().c_str();
            Serial.print("tattoo set to: ");
            Serial.println(tattoo);
            // Write file to save value
            writeFile(LittleFS, tattooPath, tattoo.c_str());
          }
          if (p->name() == PARAM_INPUT_9) {
            anthem = p->value().c_str();
            Serial.print("anthem set to: ");
            Serial.println(anthem);
            // Write file to save value
            writeFile(LittleFS, anthemPath, anthem.c_str());
          }
          //Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
        }
      }
      delay(3000);
      ESP.restart();
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
    myDFPlayer.volume(30);  //Set volume value. From 0 to 30
    myDFPlayer.play(1);     //Play the first mp3
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("starting server");
      server.begin();
    }
    
    
  }
  
}

void loop() {
    struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");

    return;
  }
  String hour = String(timeinfo.tm_hour);
  String minute = String(timeinfo.tm_min);
  int second = timeinfo.tm_sec;

  // Serial.println(String(timeinfo.tm_wday));

  if(second == 0 || initLoad){
  lcd.clear();
  // set cursor to first column, first row
  lcd.setCursor(5, 0);
  // print message
  lcd.print(&timeinfo, "%H:%M");
  // clears the display to print new message
  // set cursor to first column, second row
  lcd.setCursor(3,1);
  lcd.print(&timeinfo, "%m-%d-%Y");
  initLoad = false;

  }
  String currentTime = hour + ":" + minute;
  if(!currentTime.compareTo(revellie) && second == 0){
    myDFPlayer.play(2);
  }
  if(!currentTime.compareTo(retreat) && second == 0){
    myDFPlayer.play(3);
  }
  if(!currentTime.compareTo(anthem) && second == 0){
    myDFPlayer.play(6);
  }
  if(!currentTime.compareTo(taps) && second == 0){
    myDFPlayer.play(4);
  }
  if(!currentTime.compareTo(tattoo) && second == 0){
    
    myDFPlayer.play(5);
  }   
}