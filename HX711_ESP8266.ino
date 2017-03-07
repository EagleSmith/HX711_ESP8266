#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsServer.h>
#include <Hash.h>

ESP8266WiFiMulti WiFiMulti;

WebSocketsServer webSocket = WebSocketsServer(81);


#define CLK 14
#define DAT 13

const char* ssid     = "GLASS";
const char* password = "MITMEDIALAB";

unsigned long count;

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {

    switch(type) {
        case WStype_DISCONNECTED:
            Serial.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED:
            {
                IPAddress ip = webSocket.remoteIP(num);
                Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        
        // send message to client
        webSocket.sendTXT(num, "Connected");
            }
            break;
        case WStype_TEXT:
            Serial.printf("[%u] get Text: %s\n", num, payload);

            // send message to client
            // webSocket.sendTXT(num, "message here");

            // send data to all connected clients
            //String loadDataStr = String(count);
            //webSocket.broadcastTXT(loadDataStr);
            break;
        case WStype_BIN:
            Serial.printf("[%u] get binary lenght: %u\n", num, lenght);
            hexdump(payload, lenght);

            // send message to client
            // webSocket.sendBIN(num, payload, lenght);
            break;
    }

}
 
void setup() {
  pinMode(CLK, OUTPUT);
  pinMode(DAT, INPUT);

  resetHX711();
 
  Serial.begin(115200);
  delay(100);

  Serial.setDebugOutput(true);

   for(uint8_t t = 4; t > 0; t--) {
        Serial.printf("[SETUP] BOOT WAIT %d...\n", t);
        Serial.flush();
        delay(1000);
    }

 
    WiFiMulti.addAP("SSID", "passpasspass");

    while(WiFiMulti.run() != WL_CONNECTED) {
        delay(100);
    }

    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
    
 
}


void resetHX711() {
   digitalWrite(CLK, HIGH);
   delayMicroseconds(70);
   digitalWrite(CLK, LOW);
}

 
void loop() {
  unsigned char i;
  int data;
  count = 0;

  while (digitalRead(DAT)==HIGH) { }
  
  for (i=0; i<24; ++i) {
    digitalWrite(CLK, HIGH);
    count = count << 1;
    data = digitalRead(DAT);
    digitalWrite(CLK, LOW);
    delayMicroseconds(1);  
    if (data==HIGH) count++; 
  }
  digitalWrite(CLK, HIGH);
  count = count ^ 0x800000;
  digitalWrite(CLK, LOW);  
  Serial.println(count);
  String loadDataStr = String(count);
  webSocket.broadcastTXT(loadDataStr);
            
  webSocket.loop(); 
}
