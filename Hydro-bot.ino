
#include <Arduino.h>
#include <ArduinoJson.h>
//--------------- ESP32 Libraries
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiMulti.h>
//---------------------------------
#include "DHTesp.h"
#define DHTpin 14    //D15 of ESP32 DevKit
#include "CapacitiveMoisture.h"
//---------
const   unsigned short g_PinSensor = 36;
const   unsigned int  g_CallibrationMin = 1400; // Default min (DRY)
const   unsigned int  g_CallibrationMax = 3040; // Default max (under water)
const   unsigned int  g_LapseInterval = 2000;   // Lapse interval read on loop
                                                // if g_LapseInterval=0 -->no lapse
CapacitiveMoisture    g_SensorCap=CapacitiveMoisture();
DHTesp dht;
//---------------------------------


#define SDA_PIN 4
#define SCL_PIN 5

WiFiMulti wifiMulti;

const int16_t I2C_MASTER = 0x42;
const int16_t I2C_SLAVE = 0x08;

const char* ssid = "Epics";
const char* password = "expo1234";
//const char* ssid = "who is the hottest man";
//const char* password = "praveenk";
//const char* ssid = "TP-Link_81BC";
//const char* password = "09870987";


const char* fingerprint = "BB DC 45 2A 07 E3 4A 71 33 40 32 DA BE 81 F7 72 6F 4A 2B 6B";
const char* BotToken = "784890187:AAGuBQx0ZY_Vo02Dwc35N54NNnEN4kxxSxY";
const char* host = "api.telegram.org";
const int ssl_port = 443;

boolean first_check = true;
String G_cmd;
long G_chat_id;
long prv_update_id = 0, update_id = 0;


void scan(String cmd){
   HTTPClient http;
  if(cmd == "Status"){
    Serial.println("Checkpoint 2");
    String txt1 = "Moisture: " + String(g_SensorCap.read());// + "xx";//String(mos);
    String txt2 = "Temperature: "+ String(dht.toFahrenheit(dht.getTemperature()));// + "xx";//String(temp);
    String txt3 = "Humidity: "+ String(dht.getHumidity());// + "xx";//String(humi);
    Serial.print("txt = ");
    Serial.println(txt1);
    Serial.println(txt2);
    Serial.println(txt3);
    http.begin("https://api.telegram.org/bot784890187:AAGuBQx0ZY_Vo02Dwc35N54NNnEN4kxxSxY/sendMessage?chat_id=" + String(G_chat_id) + "&text=" + txt1);
    int httpCode = http.GET();
    Serial.print("httpCode");
    Serial.println(httpCode);
    http.begin("https://api.telegram.org/bot784890187:AAGuBQx0ZY_Vo02Dwc35N54NNnEN4kxxSxY/sendMessage?chat_id=" + String(G_chat_id) + "&text=" + txt2);
    httpCode = http.GET();
    Serial.print("httpCode");
    Serial.println(httpCode);
    http.begin("https://api.telegram.org/bot784890187:AAGuBQx0ZY_Vo02Dwc35N54NNnEN4kxxSxY/sendMessage?chat_id=" + String(G_chat_id) + "&text=" + txt3);
    httpCode = http.GET();
    Serial.print("httpCode");
    Serial.println(httpCode);
    if (httpCode > 0) {
      Serial.print("Send Success, code = ");
      Serial.println(httpCode);
    }
    http.end();
  }
  else if(cmd == "R1"){
    Serial.println("Switching Feedback pump");
    digitalWrite(26, LOW);
    delay(2000);
    digitalWrite(26, HIGH);
    Serial.println("Switching off pump");
  }
  else if(cmd == "R2"){
    Serial.println("Switching vlave");
    digitalWrite(25, HIGH);
    delay(10000);
    digitalWrite(25, LOW);
    Serial.println("Switching off Valve");
  }
  //else{text_to_send = "Unidentified command"};
  else{
      Serial.println("Checkpoint 2.1");
      String txt = "Unidentified command";
      http.begin("https://api.telegram.org/bot784890187:AAGuBQx0ZY_Vo02Dwc35N54NNnEN4kxxSxY/sendMessage?chat_id=" + String(G_chat_id) + "&text=" + txt);
      int httpCode = http.GET();
      Serial.print("httpCode = ");
      Serial.println(httpCode);
      if (httpCode > 0) {
        Serial.print("Send Success, code = ");
        Serial.println(httpCode);
      }
      http.end();
  }
}

boolean Reply(){
  extern boolean first_check;
  extern String G_cmd;
  extern long G_chat_id;
  HTTPClient http;
  http.begin("https://api.telegram.org/bot784890187:AAGuBQx0ZY_Vo02Dwc35N54NNnEN4kxxSxY/getUpdates?offset=-1");
  int httpCode = http.GET();
  Serial.println("HTTP connection started...\n httpCode = ");
  Serial.println(httpCode);
  if (httpCode > 0){
    StaticJsonDocument<768> doc;
    String payload = http.getString();
    Serial.print(payload);
    DeserializationError error = deserializeJson(doc, payload);
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      http.end();
      return false;
    }
    //-------------------Reading from Json Buffer
    JsonObject result_0_message = doc["result"][0]["message"];
    JsonObject result_0_message_chat = result_0_message["chat"];
    //String cmd = doc["result"][0]["message"]["text"];
    const char* cmd = result_0_message["text"]; // "Status"
    G_cmd = cmd;
    //update_id = doc["result"][0]["update_id"];
    long update_id = doc["result"][0]["update_id"];
    //String chat_id =  doc["result"][0]["message"]["chat"]["id"];
    long chat_id = result_0_message_chat["id"];
    G_chat_id = chat_id;
    http.end();
    Serial.println();
    Serial.print("cmd = ");
    Serial.println(G_cmd);
    Serial.print("chat_id = ");
    Serial.println(G_chat_id);
    Serial.print("update_id = ");
    Serial.println(update_id);
    Serial.print("prv_update_id = ");
    Serial.println(prv_update_id);
    //--------------------------validating if the update is new
  if(prv_update_id == 0){
    prv_update_id = update_id;
    return false;
  }
  else if(prv_update_id == update_id){
    //Serial.println("First Check\n");
    return false;
  }else{
    //first_check = false;
    prv_update_id = update_id;
    return true;
  }
  }
}

void Send(String txt, long chat_id){
  Serial.println("Checkpoint 3");
  HTTPClient http;
  Serial.println(String(G_chat_id));
  http.begin("https://api.telegram.org/bot784890187:AAGuBQx0ZY_Vo02Dwc35N54NNnEN4kxxSxY/sendMessage?chat_id="+String(G_chat_id)+"&text="+txt);
  int httpCode = http.GET();
  Serial.println(httpCode);
  http.end();
  Serial.println("Checkpoint 3.1");
}

void setup(){
  Serial.begin(115200);
  // Autodetect is not working reliable, don't use the following line
  // use this instead: 
  dht.setup(DHTpin, DHTesp::DHT11); //for DHT11 Connect DHT sensor to GPIO 17
  pinMode(26, OUTPUT);
  digitalWrite(26, HIGH);
  pinMode(25, OUTPUT);
  digitalWrite(25, LOW);
  //---------------
  g_SensorCap.setup(g_PinSensor, g_LapseInterval, g_CallibrationMin,  g_CallibrationMax);
  // if you want see callibration recomended values keep this three lines.
  // in other case remove
  unsigned int numRead=100;           // number of read for callibration
  unsigned int LapseDelayMillis=3000; //Time lapse between readings
  //g_SensorCap.debugCalibration(numRead,LapseDelayMillis);
  for(uint8_t t = 4; t > 0; t--) {
        Serial.printf("[SETUP] WAIT %d...\n", t);
        Serial.flush();
        delay(1000);
    }
  wifiMulti.addAP(ssid, password);
  //Wire.begin(SDA_PIN, SCL_PIN, I2C_MASTER); 
}

void loop(){
  if(wifiMulti.run() == WL_CONNECTED){
    if(Reply()){
      Serial.println("recieved an update");
      scan(G_cmd);
      delay(500);
      //Send(txtx, G_chat_id);
      delay(2000);
    }
    else{
      Serial.println("No updates in chat, waiting...\n");
      delay(1000);
    }
  }
  //return 0;
}
