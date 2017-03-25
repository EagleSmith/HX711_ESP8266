#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsServer.h>
#include <Hash.h>

extern "C" {
#include "user_interface.h"
}

/** NODE SPECIFIC INFO **/

//the number of this device, [1,2,3 ...]

#define DEVICENAME "LOADCELL_4"
IPAddress staticIP(192, 168, 1, 194); 
IPAddress subnet(255, 255, 255, 0); 
IPAddress gateway(192, 168, 1, 1); 

//wifi SSID and Password
#include "wifi_details.h"

/************************/

ESP8266WiFiMulti WiFiMulti;

WebSocketsServer webSocket = WebSocketsServer(81);

#define DEBUG_SERIAL 1

#define CLK 14
#define DAT 13

unsigned long count;

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {

  switch (type) {
    case WStype_DISCONNECTED:
#if DEBUG_SERIAL
      Serial.printf("[%u] Disconnected!\n", num);
#endif
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
#if DEBUG_SERIAL
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
#endif

        // send message to client
        webSocket.sendTXT(num, "Connected");
      }
      break;
    case WStype_TEXT:
#if DEBUG_SERIAL
      Serial.printf("[%u] get Text: %s\n", num, payload);
#endif
      // send message to client
      // webSocket.sendTXT(num, "message here");

      // send data to all connected clients
      //String loadDataStr = String(count);
      //webSocket.broadcastTXT(loadDataStr);
      break;
    case WStype_BIN:
#if DEBUG_SERIAL
      Serial.printf("[%u] get binary lenght: %u\n", num, lenght);
      hexdump(payload, lenght);
#endif
      // send message to client
      // webSocket.sendBIN(num, payload, lenght);
      break;
  }

}

int batteryLvl = 0;
int adcRaw = 0;

void updateBatteryLevel() {

  // read the battery level from the ESP8266 analog in pin.
  // analog read level is 10 bit 0-1023 (0V-1V).
  // our 100:22 voltage divider takes the max
  // lipo value of 4.2V and drops it to 0.758V max.
  // this means our min analog read value should be 580 (3.14V)
  // and the max analog read value should be 774 (4.2V).
  adcRaw = analogRead(A0);

  // convert battery level to percent
  batteryLvl = map(adcRaw, 577, 771 , 314, 420);
#if DEBUG_SERIAL
  Serial.print(", "); Serial.print(adcRaw); Serial.print(", "); Serial.print(batteryLvl / 100.0); Serial.print("V");
#endif
}

void setup() {

  pinMode(CLK, OUTPUT);
  pinMode(DAT, INPUT);

  resetHX711();

#if DEBUG_SERIAL
  Serial.begin(115200);
  delay(100);

  Serial.setDebugOutput(true);

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] BOOT WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }
#endif
  WiFi.mode(WIFI_STA);
  WiFi.hostname(DEVICENAME);      // DHCP Hostname (useful for finding device for static lease)
  WiFi.config(staticIP, gateway, subnet, gateway); //gateway and dns are the same 
  WiFiMulti.addAP(ssid, password);

  while (WiFiMulti.run() != WL_CONNECTED) {
    delay(100);
  }

  wifi_set_sleep_type(LIGHT_SLEEP_T);

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}


void resetHX711() {
  digitalWrite(CLK, HIGH);
  delayMicroseconds(70);
  digitalWrite(CLK, LOW);
}

#define SOCKET_BUF_LENGTH 1

unsigned long socketBuffer[SOCKET_BUF_LENGTH];

int batteryInterval = 1000;
unsigned long lastBatteryMillis = 0;

void loop() {

  static unsigned char b = 0;

  unsigned char i;
  int data;
  count = 0;

  while (digitalRead(DAT) == HIGH) { }

  for (i = 0; i < 24; ++i) {
    digitalWrite(CLK, HIGH);
    count = count << 1;
    data = digitalRead(DAT);
    digitalWrite(CLK, LOW);
    delayMicroseconds(1);
    if (data == HIGH) count++;
  }
  digitalWrite(CLK, HIGH);
  count = count ^ 0x800000;
  digitalWrite(CLK, LOW);
  socketBuffer[b++] = count;

#if DEBUG_SERIAL
  Serial.print(count);
#endif

  unsigned long ms = millis();
  if (ms - lastBatteryMillis > batteryInterval) {
    updateBatteryLevel();
    lastBatteryMillis = ms;
  }

#if DEBUG_SERIAL
  Serial.println();
#endif


  String loadDataStr = String(count) + "," + String(adcRaw) + "," + String(batteryLvl);
  if (b == SOCKET_BUF_LENGTH) {
    // WiFi.forceSleepWake();
    webSocket.broadcastTXT(loadDataStr);
    b = 0;
    // WiFi.forceSleepBegin();
  }
  delay(10);
  webSocket.loop();

}
