#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <FS.h>
#include <WiFiClient.h>
#include <TimeLib.h>
#include <NtpClientLib.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESP8266mDNS.h>
#include <Ticker.h>
#include <ArduinoOTA.h>
#include <ArduinoJson.h>
#include <FSWebServerLib.h>
#include <Hash.h>

#include "flashy.h"

MyLED *myLED;
NeoPixelBusType ledstrip(pixelCount);
void connect_wifi()
{

  if (SPIFFS.begin())
  {
    Serial.println("SPIFFS opened!");
  }

  ESPHTTPServer.begin(&SPIFFS);
  ESPHTTPServer.on("/my", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/bak_index.html", "text/html");
  });
  ESPHTTPServer.on("/mode", HTTP_POST, [](AsyncWebServerRequest *request) {
    myLED->_handleMode(request->getParam("mode", true)->value());
    request->send(SPIFFS, "/mode.html", "text/html");
  });
}

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    ; // wait for serial attach
  ledstrip.Begin();

  myLED = new MyLED(ledstrip);
  myLED->init();
  myLED->begin();
  connect_wifi();
}

void loop()
{
  myLED->anim_loop();
  ledstrip.Show();
  ESPHTTPServer.handle();
}
