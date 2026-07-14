#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include "LittleFS.h"
#include "DFRobotDFPlayerMini.h"
#include <LiquidCrystal_I2C.h>
#include "time.h"
#include <ESPmDNS.h>

#define RXD1 7
#define TXD1 8

#define BUSY 4

HardwareSerial mySerial(1);


DFRobotDFPlayerMini myDFPlayer;



// Create AsyncWebServer object on port 80
AsyncWebServer server(80);


const char* PARAM_INPUT_5 = "revellie";
const char* PARAM_INPUT_6 = "retreat";
const char* PARAM_INPUT_7 = "taps";
const char* PARAM_INPUT_8 = "tattoo";
const char* PARAM_INPUT_9 = "anthem";
const char* PARAM_INPUT_4 = "bugledays";
const char* PARAM_INPUT_10 = "revellieToggle";
const char* PARAM_INPUT_11 = "anthemToggle";
const char* PARAM_INPUT_12 = "retreatToggle";
const char* PARAM_INPUT_13 = "tattooToggle";
const char* PARAM_INPUT_14 = "tapsToggle";

//Variables to save values from HTML form
String revellie;
String anthem;
String retreat;
String taps;
String tattoo;
String days;
String revTog;
String anthemTog;
String retreatTog;
String tapsTog;
String tattooTog;

bool initLoad = true;

// File paths to save input values permanently
const char* revelliePath = "/revellie.txt";
const char* anthemPath = "/anthem.txt";
const char* retreatPath = "/retreat.txt";
const char* tapsPath = "/taps.txt";
const char* tattooPath = "/tattoo.txt";
const char* bugleDays = "/bugledays.txt";
const char* revTogPath = "/revTogPath.txt";
const char* anthemTogPath = "/anthemTogPath.txt";
const char* retTogPath = "/retreatTogPath.txt";
const char* tapsTogPath = "/tapsTogPath.txt";
const char* tattooTogPath = "/tattooTogPath.txt";


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
  mySerial.begin(9600, SERIAL_8N1, RXD1, TXD1);

  delay(8000);

   if (!myDFPlayer.begin(mySerial)) {
    Serial.println(F("Unable to begin:"));
    while(true);
  }


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
  days = readFile (LittleFS, bugleDays);
  revTog = readFile (LittleFS, revTogPath);
  anthemTog = readFile (LittleFS, anthemTogPath);
  retreatTog = readFile (LittleFS, retTogPath);
  tapsTog = readFile (LittleFS, tapsTogPath);
  tattooTog = readFile(LittleFS, tattooTogPath);

  Serial.println(revellie);
  Serial.println(anthem);
  Serial.println(retreat);
  Serial.println(taps);
  Serial.println(tattoo);
  Serial.println(days);
  
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
  res = wm.autoConnect("Bugle Portal","password"); // password protected ap


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
      writeFile(LittleFS, tattooPath, "20:45");
    }

    if(anthem == ""){
      writeFile(LittleFS, anthemPath, "12:00");

    }

    if(days == ""){
      writeFile(LittleFS, bugleDays, "0123456");
    }

    if(revTog == ""){
      writeFile(LittleFS, revTogPath, "on");
    }
    if(anthemTog == ""){
      writeFile(LittleFS, anthemTogPath, "on");
    }
    if(retreatTog == ""){
      writeFile(LittleFS, retTogPath, "on");
    }
    if(tapsTog == ""){
      writeFile(LittleFS, tapsTogPath, "on");
    }
    if(tattooTog == ""){
      writeFile(LittleFS, tattooTogPath, "on");
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
    server.on("/getDays", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(200, "text/plain", days);
    });
    server.on("/getRevTog", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(200, "text/plain", revTog);
    });
    server.on("/getAntTog", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(200, "text/plain", anthemTog);
    });
    server.on("/getRetTog", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(200, "text/plain", retreatTog);
    });
    server.on("/getTatTog", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(200, "text/plain", tattooTog);
    });
    server.on("/getTapTog", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(200, "text/plain", tapsTog);
    });
    server.on("/playRev", HTTP_GET, [](AsyncWebServerRequest *request){
      myDFPlayer.play(2);
    });
    server.on("/playRet", HTTP_GET, [](AsyncWebServerRequest *request){
      myDFPlayer.play(3);
    });
    server.on("/playTaps", HTTP_GET, [](AsyncWebServerRequest *request){
      myDFPlayer.play(4);
    });
    server.on("/playTat", HTTP_GET, [](AsyncWebServerRequest *request){
      myDFPlayer.play(5);
    });
    server.on("/playAnt", HTTP_GET, [](AsyncWebServerRequest *request){
      myDFPlayer.play(6);
    });
    server.serveStatic("/", LittleFS, "/");

    server.on("/times", HTTP_POST, [](AsyncWebServerRequest *request) {
      int params = request->params();
      for(int i=0;i<params;i++){
        const AsyncWebParameter* p = request->getParam(i);
        if(p->isPost()){
          if (p->name() == PARAM_INPUT_4) {
            days = p->value().c_str();
            Serial.print("days is set to: ");
            Serial.println(days);
            // Write file to save value
            writeFile(LittleFS, bugleDays, days.c_str());
          }
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
          if (p->name() == PARAM_INPUT_10) {
            revTog = p->value().c_str();
            Serial.print("rev Tog Path set to: ");
            Serial.println(revTog);
            // Write file to save value
            writeFile(LittleFS, revTogPath, revTog.c_str());
          }
          if (p->name() == PARAM_INPUT_11) {
            anthemTog = p->value().c_str();
            Serial.print("ant tog Path set to: ");
            Serial.println(anthemTog);
            // Write file to save value
            writeFile(LittleFS, anthemTogPath, anthemTog.c_str());
          }
          if (p->name() == PARAM_INPUT_12) {
            retreatTog = p->value().c_str();
            Serial.print("ret tog Path set to: ");
            Serial.println(retreatTog);
            // Write file to save value
            writeFile(LittleFS, retTogPath, retreatTog.c_str());
          }
          if (p->name() == PARAM_INPUT_13) {
            tattooTog = p->value().c_str();
            Serial.print("tat tog Path set to: ");
            Serial.println(tattooTog);
            // Write file to save value
            writeFile(LittleFS, tattooTogPath, tattooTog.c_str());
          }
          if (p->name() == PARAM_INPUT_14) {
            tapsTog = p->value().c_str();
            Serial.print("taps tog Path set to: ");
            Serial.println(tapsTog);
            // Write file to save value
            writeFile(LittleFS, tapsTogPath, tapsTog.c_str());
          }
          //Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
        }
      }
      delay(3000);
      request->send(200, "text/html", "<h1>Success!</h1><p>Your dates and times were submited click the back button to reload the page.</p><a href=\"/\">Back</a>");
    });
    
    Serial.println();
    Serial.println(F("\nESP32-S3 DFRobot DFPlayer Mini Demo"));
    Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));

    // if (!myDFPlayer.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with mp3.
    //   Serial.println(F("Unable to begin:"));
    //   Serial.println(F("1.Please recheck the connection!"));
    //   Serial.println(F("2.Please insert the SD card!"));
    //   while (true);
    // }
    Serial.println(("DFPlayer Mini online."));
    myDFPlayer.volume(50);  //Set volume value. From 0 to 30
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
  String currentDay = String(timeinfo.tm_wday);

  
  

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
  if(!currentTime.compareTo(revellie) && second == 0 && days.indexOf(currentDay) >= 0){
    myDFPlayer.play(2);
  }
  if(!currentTime.compareTo(retreat) && second == 0 && days.indexOf(currentDay) >= 0){
    myDFPlayer.play(3);
  }
  if(!currentTime.compareTo(anthem) && second == 0 && days.indexOf(currentDay) >= 0){
    myDFPlayer.play(6);
  }
  if(!currentTime.compareTo(taps) && second == 0 && days.indexOf(currentDay) >= 0){
    myDFPlayer.play(4);
  }
  if(!currentTime.compareTo(tattoo) && second == 0 && days.indexOf(currentDay) >= 0){
    
    myDFPlayer.play(5);
  }   
}