#include <Arduino.h>
#include <ArduinoJson.h>
//--------------- ESP32 Libraries
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiMulti.h>
long lastMsg = 0;
char msg[50];
int value = 0;
//---------------------------------
#include <PubSubClient.h>
const char* mqtt_server = "192.168.0.104";

WiFiClient espClient;
PubSubClient client(espClient);

#include "DHTesp.h"
#define DHTpin 14    //D15 of ESP32 DevKit  
DHTesp dht;
//---------------------------------

#define SensorPin 35          //pH meter Analog output to Arduino Analog Input 0
unsigned long int avgValue;  //Store the average value of the sensor feedback
float b;
int buf[10],temp;

//const char* ssid = "Epics";
//const char* password = "expo1234";
//const char* ssid = "who is the hottest man";
//const char* password = "praveenk";
const char* ssid = "TP-Link_81BC";
const char* password = "09870987";


const char* fingerprint = "BB DC 45 2A 07 E3 4A 71 33 40 32 DA BE 81 F7 72 6F 4A 2B 6B";
const char* BotToken = "784890187:AAGuBQx0ZY_Vo02Dwc35N54NNnEN4kxxSxY";
const char* host = "api.telegram.org";
const int ssl_port = 443;

//boolean first_check = true;
//String G_cmd;
//long G_chat_id;
//long prv_update_id = 0, update_id = 0;

//sensor values
//int w_temp, humi, temperature, ph;

int ph_read(){
  for(int i=0;i<10;i++)       //Get 10 sample value from the sensor for smooth the value
  { 
    buf[i]=analogRead(SensorPin);
    delay(10);
  }
  for(int i=0;i<9;i++)        //sort the analog from small to large
  {
    for(int j=i+1;j<10;j++)
    {
      if(buf[i]>buf[j])
      {
        temp=buf[i];
        buf[i]=buf[j];
        buf[j]=temp;
      }
    }
  }
  avgValue=0;
  for(int i=2;i<8;i++)                      //take the average value of 6 center sample
    avgValue+=buf[i];
  float phValue=(float)avgValue*5.0/1024/6; //convert the analog into millivolt
  phValue=3.5*phValue;
  phValue=phValue-32.0;
  Serial.println("exit ph");
  return phValue;
  return 5;
}

int dht_read_temperature(){
  //return dht.toFahrenheit(dht.getTemperature());
  TempAndHumidity newValues = dht.getTempAndHumidity();
  return newValues.humidity;
}

  int dht_read_humidity(){
  return dht.getHumidity();
  return 5;
}

int water_temperature(){
  
  return 5;
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("esp32/output");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
 
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
  //update_id = int(messageTemp );
  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
  // Changes the output state according to the message
  /*
  if (String(topic) == "esp32/output") {
    Serial.print("Changing output to ");
    if(messageTemp == "on"){
      Serial.println("on");
      digitalWrite(ledPin, HIGH);
    }
    else if(messageTemp == "off"){
      Serial.println("off");
      digitalWrite(ledPin, LOW);
    }*/
      TempAndHumidity newValues = dht.getTempAndHumidity();
      
      char humiString[8];// = (char*)malloc(8);
      //String myString = String(newValues.humidity);
      //myString.toCharArray(humiString, 8);
      //humi = String(dht_read_humidity());
      dtostrf((double)newValues.humidity, 1, 2, humiString);
      client.publish("esp32/tele/humidity", humiString);
      Serial.print("humidity:        ");
      Serial.println(humiString);
      //free(humiString);
      
      delay(300);
      
      char tempString[8];// = (char*)malloc(8);
      dtostrf((double)newValues.temperature, 1, 2, tempString);
      client.publish("esp32/tele/temperature", tempString);
      Serial.print("temperature:        ");
      Serial.println(tempString);
      ///free(tempString);

      delay(300);
      
      char w_tempString[8];// = (char*)malloc(8);
      dtostrf((double)newValues.temperature-2.0, 1, 2, w_tempString);
      client.publish("esp32/tele/temp_water", w_tempString);
      Serial.print("water_temperature:        ");
      Serial.println(w_tempString);
      //free(w_tempString);

      delay(300);
      
      char phString[8];// = (char*)malloc(8);
      dtostrf((double)ph_read(), 1, 2, phString);
      client.publish("esp32/tele/phvalue", phString);
      Serial.print("ph:        ");
      Serial.println(phString);
      //free(tempString);
      delay(300);
}

void setup(){
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  dht.setup(DHTpin, DHTesp::DHT11); //for DHT11 Connect DHT sensor to GPIO 17
}

void loop(){
  if(WiFi.status() == WL_CONNECTED){
    if (!client.connected()) {
      reconnect();
    }
    client.loop();

    long noww = millis();
    if (noww - lastMsg > 60000) {
      lastMsg = noww;
      Serial.println("loop");
      TempAndHumidity newValues = dht.getTempAndHumidity();
      
      char humiString[8];
      //String myString = String(newValues.humidity);
      //myString.toCharArray(humiString, 8);
      //humi = String(dht_read_humidity());
      dtostrf((double)newValues.humidity, 1, 2, humiString);
      client.publish("esp32/humidity", humiString);
      Serial.print("humidity:        ");
      Serial.println(humiString);
      //free(humiString);
      
      delay(300);
      
      char tempString[8];// = (char*)malloc(8);
      dtostrf((double)newValues.temperature, 1, 2, tempString);
      client.publish("esp32/temperature", tempString);
      Serial.print("temperature:        ");
      Serial.println(tempString);
      //free(tempString);

      delay(300);
      
      char w_tempString[8];/// = (char*)malloc(8);
      dtostrf((double)newValues.temperature-2.0, 1, 2, w_tempString);
      client.publish("esp32/temp_water", w_tempString);
      Serial.print("water_temperature:        ");
      Serial.println(w_tempString);
      //free(w_tempString);

      delay(300);
      
      char phString[8];// = (char*)malloc(8);
      dtostrf((double)ph_read(), 1, 2, phString);
      client.publish("esp32/phvalue", phString);
      Serial.print("ph:        ");
      Serial.println(phString);
     // free(tempString);
      delay(300);
      //(void)water_temperature();
    }
    }
  //return 0;
}
