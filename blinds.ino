#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include <Stepper_28BYJ_48.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SinricPro.h>
#include <SinricProBlinds.h>
#include <FS.H>
#include <ArduinoJson.h>

#include "index.h"

#define APP_KEY    ""
#define APP_SECRET ""
#define DEVICE_ID  ""

#define SSID       ""
#define PASS       ""

#define HOSTNAME   ""

#define BAUD_RATE  9600

struct Config {
  long current_position;
  long max_position;
};

SinricProBlinds &rollo = SinricPro[DEVICE_ID];
Stepper_28BYJ_48 stepper(D1, D3, D2, D4);
AsyncWebServer server(80);
bool ccw = false;
int globalPercentage;
bool initLoop = true, power =  false;
String action;
int path;
Config config;

bool onRangeValue(const String &deviceId, int &position) {
  Serial.printf("Device %s set position to %d\r\n", deviceId.c_str(), position);
  action = "auto";
  path = config.max_position - ((config.max_position / 100) * position);
  return true;
}

bool onAdjustRangeValue(const String &deviceId, int &positionDelta) {
  Serial.printf("Device %s position changed about %i\r\n", deviceId.c_str(), positionDelta);
  return true;
}

bool onPowerState(const String &deviceId, bool &state) {
  if (!state) power = true;
  return true;
}

void setupSinricPro() {
  rollo.onRangeValue(onRangeValue);
  rollo.onAdjustRangeValue(onAdjustRangeValue);
  rollo.onPowerState(onPowerState);

  SinricPro.onConnected([] { Serial.printf("[SinricPro]: Connected\r\n"); });
  SinricPro.onDisconnected([] { Serial.printf("[SinricPro]: Disconnected\r\n"); });
  SinricPro.begin(APP_KEY, APP_SECRET);
};

void setupWiFi() {
  WiFi.hostname(HOSTNAME);
  WiFi.begin(SSID, PASS);
  Serial.printf("[WiFi]: Connecting to %s", SSID);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.printf(".");
    delay(250);
  }
  Serial.printf("connected\r\n");
}

void setupOTA() {
  ArduinoOTA.setHostname(HOSTNAME);
  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
}

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

void setup_ws()
{
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/html", index_html);
  });
  server.on("/up", HTTP_GET, [] (AsyncWebServerRequest * request) {
    path = -1;
    action = "manual";
    Serial.print("Manual UP\r\n");
    request->send(200, "text/plain", "ok");
  });
  server.on("/down", HTTP_GET, [] (AsyncWebServerRequest * request) {
    path = 1;
    action = "manual";
    Serial.print("Manual DOWN\r\n");
    request->send(200, "text/plain", "ok");
  });
  server.on("/stop", HTTP_GET, [] (AsyncWebServerRequest * request) {
    path = 0;
    action = "manual";
    Serial.print("Manual STOP\r\n");
    request->send(200, "text/plain", "ok");
  });
  server.on("/save_up", HTTP_GET, [] (AsyncWebServerRequest * request) {
    config.current_position = 0;
    path = 0;
    action = "manual";
    Serial.print("SAVE UP\r\n");
    request->send(200, "text/plain", "ok");
  });
  server.on("/save_down", HTTP_GET, [] (AsyncWebServerRequest * request) {
    config.max_position = config.current_position;
    path = 0;
    action = "manual";
    Serial.print("SAVE DOWN\r\n");
    request->send(200, "text/plain", "ok");
  });
  server.on("/save_config", HTTP_GET, [] (AsyncWebServerRequest * request) {
    save_config();
    Serial.print("SAVE TO SPIFFS\r\n");
    request->send(200, "text/plain", "ok");
  });
  server.onNotFound(notFound);
  server.begin();
}

void load_config()
{
  SPIFFS.begin();
  File settings = SPIFFS.open("/settings.json", "r");
  if(!settings)
  {
    save_config();
  }
  StaticJsonDocument<512> doc;
  deserializeJson(doc, settings);
  config.current_position = doc["current_position"];
  config.max_position = doc["max_position"];
  settings.close();
}

void save_config()
{
  File settings = SPIFFS.open("/settings.json", "w");
  StaticJsonDocument<256> doc;
  doc["current_position"] = config.current_position;
  doc["max_position"] = config.max_position;
  serializeJson(doc,settings);
  settings.close();
}

void stopPowerToCoils() {
  digitalWrite(D1, LOW);
  digitalWrite(D2, LOW);
  digitalWrite(D3, LOW);
  digitalWrite(D4, LOW);
}

void setup() {
  Serial.begin(BAUD_RATE);
  setupWiFi();
  setupSinricPro();
  setupOTA();
  setup_ws();
  load_config();
}

void update_mode()
{
  if (action == "auto")
  {
    if (config.current_position > path)
    {
      stepper.step(ccw ? -1 : 1);
      config.current_position = config.current_position - 1;
    } else if (config.current_position < path)
    {
      stepper.step(ccw ? 1 : -1);
      config.current_position = config.current_position + 1;
    } else
    {
      path = 0;
      action = "";
      power = true;
    }
  }
  if (action == "manual" && path != 0)
  {
    stepper.step(ccw ?  path : -path);
    config.current_position = config.current_position + path;
  }
}

void loop() {

  SinricPro.handle();
  ArduinoOTA.handle();
  update_mode();

  if (power)
  {
    power = false;
    stopPowerToCoils();
  }

  if (initLoop)
  {
    initLoop = false;
    stopPowerToCoils();
  }
}
