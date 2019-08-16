#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <SparkFun_Si7021_Breakout_Library.h>
#include <Wire.h>
#include <ArduinoJson.h>

HTTPClient http;
Weather sensor;
TaskHandle_t Task1;
TaskHandle_t Task2;
WiFiServer server(80);
void senddt(void *pvParameter) {
  while(true) {
    StaticJsonDocument <300> sensordt;
    sensordt["temp"] = round(sensor.getTemp()*10)/10;
    sensordt["humidity"] = round(sensor.getRH()*10)/10;
    char data[300];
    serializeJson(sensordt,data);
    http.begin("192.168.1.104",190,"/postjson");
    http.addHeader("Content-Type","application/json");
    Serial.println("Printing from: " + xPortGetCoreID());
    int httpstatus = http.POST(data);
    if(httpstatus == -1) {
      Serial.println("Sent failed!");
      vTaskDelay(1000);
    }
    http.end();
    vTaskDelay(2000);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(2,OUTPUT);
  WiFi.begin("TP-LINK","0949321325");
  while(WiFi.status() != WL_CONNECTED) {
    Serial.println("Waiting for connection");
    delay(400);
  }
  digitalWrite(2,1);
  sensor.begin();
  server.begin();
  delay(1);
  xTaskCreatePinnedToCore(senddt,"Task2",10000,NULL,1,&Task2,0);
  Serial.println(WiFi.localIP());
}

void loop() {
  WiFiClient client = server.available();
  Serial.println("Printing from: "+xPortGetCoreID());
  if(client) {
    while(client.connected()) {
      if(client.available()) {
        char c = client.read();
        Serial.print(c);
        if(c == '\n') {
          client.println("OK");
          break;
        }
      }
    }
  }
  client.stop();
  vTaskDelay(500);
}