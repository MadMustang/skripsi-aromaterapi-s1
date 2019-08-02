#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include "FS.h"

// AP Mode SSID and Password
const char* ssid = "NodeMCU";  // Enter SSID here
const char* password = "12345678";  //Enter Password here

/* Put IP Address details */
IPAddress local_ip(192,168,4,1);
IPAddress gateway(192,168,4,1);
IPAddress subnet(255,255,255,0);

ESP8266WebServer server(80);

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

void handle_OnConnect() {

  /*
  StaticJsonDocument<256> json;
  json["serverName"] = "api.jamban.com";
  json["accessToken"] = "128du9as8du12eoue8da98h123ueh9h98";

  String output;
  serializeJson(json, output);

  */

  File configFile = SPIFFS.open("/config.json", "r");

  //StaticJsonDocument<256> output;
  //DeserializationError error = deserializeJson(output, configFile);

  StaticJsonDocument<256> json;
  DeserializationError error = deserializeJson(json, configFile);
  if (error) {
    Serial.println("Failed to parse config file");
//    return false;
  }

  String output;
  serializeJson(json, output);

  server.send(200, "application/json", output);
}


void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);

  // AP Mode start
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  WiFi.softAP(ssid, password);
  delay(100);

  // Initialize LED
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  if (!SPIFFS.begin()) {
    Serial.println("Failed to mount file system");
    return;
  }

  // Handle requests
  server.on("/", handle_OnConnect);
  server.onNotFound(handle_NotFound);

  // Start server
  server.begin();

}

void loop() {
  // put your main code here, to run repeatedly:
  server.handleClient();

}
