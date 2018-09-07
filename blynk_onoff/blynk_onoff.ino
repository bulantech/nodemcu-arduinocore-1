#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#define BLYNK_PRINT Serial        // Comment this out to disable prints and save space
#include "FS.h"
#include <ArduinoJson.h>
#include <Ticker.h>               //for LED status
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include <BlynkSimpleEsp8266.h>

// define button set token
#define SW_PIN 0

WidgetLED led0(V0);
WidgetLED led1(V1);
WidgetLED led2(V2);
WidgetLED led3(V3);
WidgetLED led4(V4);
const int btnPin0 = D0;
const int btnPin1 = D1;
const int btnPin2 = D2;
const int btnPin3 = D3;
const int btnPin4 = D4;
char mqtt_server[40];
char mqtt_port[6] = "8080";
char blynk_token[33] = "YOUR_BLYNK_TOKEN";
char auth[] = "34c0acad7fee4dc2b2d37d555e110eb9";
char blynk_token_f[34] = "";
char blynk_token_s[34] = "";
int count = 0;
int count1 = 0;
File f;
Ticker ticker;
const char *filename = "/config.json";  // <- SD library uses 8.3 filenames


void tick(){
  //toggle state
  int state = digitalRead(BUILTIN_LED);  // get the current state of GPIO1 pin
  digitalWrite(BUILTIN_LED, !state);     // set pin to the opposite state
}

void tick1(){
  //toggle state
  int state = digitalRead(BUILTIN_LED);  // get the current state of GPIO1 pin
  digitalWrite(BUILTIN_LED, !state);     // set pin to the opposite state
  delay(10);
  digitalWrite(BUILTIN_LED, state);     // set pin to the opposite state
}

boolean btnState0 = false;
boolean btnState1 = false;
boolean btnState2 = false;
boolean btnState3 = false;
boolean btnState4 = false;
void buttonLedWidget(const int *btnPin,boolean *btnState,int Vo){
  // Read button
  boolean isPressed = (digitalRead(*btnPin) == LOW);

  // If state has changed...
  if (isPressed != *btnState) {
    if (isPressed) {
      switch(Vo){
        case 0:
          led0.on();
        break;
        case 1:
          led1.on();
        break;
        case 2:
          led2.on();
        break;
        case 3:
          led3.on();
        break;
        case 4:
          led4.on();
        break;
      }
    } else {
       switch(Vo){
        case 0:
          led0.off();
        break;
        case 1:
          led1.off();
        break;
        case 2:
          led2.off();
        break;
        case 3:
          led3.off();
        break;
        case 4:
          led4.off();
        break;
      }
    }
    *btnState = isPressed;
  }
}

bool loadConfig(const char *filename){
//  // always use this to "mount" the filesystem
//    bool result = SPIFFS.begin();
//    Serial.println("SPIFFS opened: " + result);
    Serial.printf("loadConfig file name ->%s<\n\r",filename);
    // this opens the file "f.txt" in read-mode
     //f = SPIFFS.open("/config5.json", "r");
     f = SPIFFS.open(filename, "r");
    
    if (!f) {
      Serial.println("File doesn't exist yet. Creating it");
  
      // open the file in write mode
      //f = SPIFFS.open("/config5.json", "w");
      f = SPIFFS.open(filename, "w");
      if (!f) {
        Serial.println("file creation failed");
      }
      // Allocate the JSON document
      //
      // Inside the brackets, 200 is the RAM allocated to this document.
      // Don't forget to change this value to match your requirement.
      // Use arduinojson.org/assistant to compute the capacity.
      StaticJsonDocument<200> doc;
    
      // StaticJsonObject allocates memory on the stack, it can be
      // replaced by DynamicJsonDocument which allocates in the heap.
      //
      // DynamicJsonDocument  doc(200);
    
      // Make our document be an object
      JsonObject root = doc.to<JsonObject>();
    
      // Add values in the object
      //
      // Most of the time, you can rely on the implicit casts.
      // In other case, you can do root.set<long>("time", 1351824120);
      root["blynk_token"] = auth;
      // now write two lines in key/value style with  end-of-line characters
      
      // Serialize JSON to file
      if (serializeJson(doc, f) == 0) {
        Serial.println(F("Failed to write to file"));
      }
    } else {
      // we could open the file
      while(f.available()) {
        //Lets read line by line from the file
        String line = f.readStringUntil('\n');
        Serial.println(line);

        // Allocate the JSON document
        //
        // Inside the brackets, 200 is the RAM allocated to this document.
        // Don't forget to change this value to match your requirement.
        // Use arduinojson.org/assistant to compute the capacity.
        StaticJsonDocument<200> doc;
        
        // Deserialize the JSON document
        DeserializationError error = deserializeJson(doc, line);
      
        // Test if parsing succeeds.
        if (error) {
          Serial.print(F("deserializeJson() failed: "));
          Serial.println(error.c_str());
          return false;
        }
        
        // Get the root object in the document
        JsonObject root = doc.as<JsonObject>();
        //const char* blynk_token_f = (const char*)root["blynk_token"];   //can not use
        //const char* blynk_token_f = root["blynk_token"];                //can not use
        //const char* blynk_token_f = root["blynk_token"];                //can not use
        strcpy(blynk_token_f, root["blynk_token"]);
        Serial.printf("blynk_token_f->%s<\n\r",blynk_token_f);
        Blynk.config(blynk_token_f);
        Blynk.config(blynk_token_f,IPAddress(10,1,3,180),8080);
        
        //Blynk.config(auth);                                             //Available                                         
        
      }
  
    }
    f.close();
    return true;
}

bool saveConfig(const char *filename,const char* blynk_token_in){
  Serial.printf("saveConfig file name ->%s<\n\r",filename);
  //f = SPIFFS.open("/config5.json", "w");
  f = SPIFFS.open(filename, "w");
      if (!f) {
        Serial.println("file creation failed");
        return false;
      }  

      StaticJsonDocument<200> doc;
      JsonObject root = doc.to<JsonObject>();

      root["blynk_token"] = blynk_token_in;
      // now write two lines in key/value style with  end-of-line characters
      
      // Serialize JSON to file
      if (serializeJson(doc, f) == 0) {
        Serial.println(F("Failed to write to file"));
      }
      f.close();
      return true;
}

bool setWifiManager(){
  ticker.attach(0.2, tick);  // led toggle faster
  Serial.printf("<<<- setWifiManager Start ->>>");
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
 
  //reset saved settings
  if(digitalRead(SW_PIN) == LOW) // Press button
  {
    Serial.println();
    Serial.println("Reset wifi config");
    wifiManager.resetSettings(); 
  }    
  // ip 192.168.4.1
  //fetches ssid and pass from eeprom and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  WiFiManagerParameter custom_blynk_token("Blynk", "blynk token", blynk_token_s, 34);
  WiFiManagerParameter custom_mqtt_server("server1", "mqtt server", mqtt_server, 40);
  WiFiManagerParameter custom_mqtt_port("port1", "mqtt port", mqtt_port, 5);
  wifiManager.addParameter(&custom_blynk_token);
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);

  wifiManager.autoConnect("AutoConnectAP");
  //or use this for auto generated name ESP + ChipID
  //wifiManager.autoConnect();

  
  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");

  const char* blynk_token1 = custom_blynk_token.getValue();
  strcpy(blynk_token_s, blynk_token1);

    //read updated parameters
  strcpy(mqtt_server, custom_mqtt_server.getValue());
  strcpy(mqtt_port, custom_mqtt_port.getValue());
  strcpy(blynk_token, custom_blynk_token.getValue());

  Serial.printf("mqtt_server->%s<\n\r",mqtt_server);
  Serial.printf("mqtt_port->%s<\n\r",mqtt_port);
  Serial.printf("blynk_token->%s<\n\r",blynk_token);

  
  Serial.printf("blynk_token_s->%s<\n\r",blynk_token_s);

  //if (strcmp(blynk_token_s, "BLYNK_TOKEN")){  //blynk_token_s == "BLYNK_TOKEN" use auth from file 
  if (strcmp(blynk_token_s, "")){  //blynk_token_s == "BLYNK_TOKEN" use auth from file 
  //if (blynk_token_s == ""){  //blynk_token_s == "BLYNK_TOKEN" use auth from file 
    Serial.println("blynk_token is set");
    saveConfig(filename,blynk_token_s);
    //Blynk.config(blynk_token_s);
    Blynk.config(blynk_token_s,IPAddress(10,1,3,180),8080);
    
  }else{
    Serial.println("blynk_token in file");  
    //Blynk.config(auth);
    loadConfig(filename);
  }
  return true;
  
}

void morniterIO(){
  
}

void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200);
    while (!Serial) continue;
    pinMode(SW_PIN, INPUT_PULLUP);  
    // always use this to "mount" the filesystem
    bool result = SPIFFS.begin();
    Serial.println("SPIFFS opened: " + result);
    setWifiManager();
    ticker.detach(); 
    
}

void loop() {
    // put your main code here, to run repeatedly:
    Blynk.run();

    if(++count >= 100000){
      count = 0;
      tick1();
      //buttonLedWidget(&btnPin3,&btnState3,3);
      //buttonLedWidget(int btnPin,boolean btnState,int Vo){
    }

    if(++count1 >= 1000){
      count1 = 0;
      buttonLedWidget(&btnPin0,&btnState0,0);
      buttonLedWidget(&btnPin1,&btnState1,1);
      buttonLedWidget(&btnPin2,&btnState2,2);
      //buttonLedWidget(&btnPin3,&btnState3,3);
      buttonLedWidget(&btnPin4,&btnState4,4);
    }
    
  //reset saved settings
  if(digitalRead(SW_PIN) == LOW){ // Press button
    delay(1000); 
    int i = 5;
    Serial.printf("Reset wifi config?:\n\r");
    while(digitalRead(SW_PIN) == LOW){
      Serial.print(String(i)+" "); 
      delay(1000);
      if((--i) == 0){
        setWifiManager(); 
        ticker.detach(); 
      }
    }
    Serial.printf("\n\rexit wifi config\n\r");
  } 
    
}
