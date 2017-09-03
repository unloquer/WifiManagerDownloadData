#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

#define BUILTIN_LED 2
#include <FS.h>
#include <Hash.h>
//needed for library
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include <SPIFFSEditor.h>

//for LED status
#include <Ticker.h>
Ticker ticker;

void tick()
{
  //toggle state
  int state = digitalRead(BUILTIN_LED);  // get the current state of GPIO1 pin
  digitalWrite(BUILTIN_LED, !state);     // set pin to the opposite state
}

//gets called when WiFiManager enters configuration mode
void configModeCallback (AsyncWiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
  //entered config mode, make led toggle faster
  ticker.attach(0.2, tick);
}

AsyncWebServer server(80);
DNSServer dns;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  SPIFFS.begin();

  // respond to GET requests on URL /heap
  server.on("/log", HTTP_GET, [](AsyncWebServerRequest *request){
      //Download index.htm
      request->_tempFile = SPIFFS.open("datalog.txt", "r");
      request->send(request->_tempFile, request->_tempFile.name(), String(), true);
      //AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "log", String(), true);
    });

  //set led pin as output
  pinMode(BUILTIN_LED, OUTPUT);
  // start ticker with 0.5 because we start in AP mode and try to connect
  ticker.attach(0.6, tick);

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  AsyncWiFiManager wifiManager(&server,&dns);
  //reset settings - for testing
  wifiManager.resetSettings();

  //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  wifiManager.setAPCallback(configModeCallback);

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect()) {
    Serial.println("failed to connect and hit timeout");
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(1000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");
  ticker.detach();
  //keep LED on
  digitalWrite(BUILTIN_LED, LOW);
  server.begin();
}

void loop() {
  // put your main code here, to run repeatedly:

}
