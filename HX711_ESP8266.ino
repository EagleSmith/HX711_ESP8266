#include <ESP8266WiFi.h>

#define CLK 14
#define DAT 13

const char* ssid     = "GLASS";
const char* password = "MITMEDIALAB";



// ThingSpeak Settings
const int channelID = 237570;
String writeAPIKey = "KJWUBECEYLJKL9NH"; // write API key for your ThingSpeak Channel
const char* server = "api.thingspeak.com";
const int postingInterval = 20 * 1000; // post data every 20 seconds

 
void setup() {
  Serial.begin(115200);
  delay(100);
 
  pinMode(CLK, OUTPUT);
  pinMode(DAT, INPUT);
  
  Serial.println();
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

  resetHX711();
}


void resetHX711() {
   digitalWrite(CLK, HIGH);
   delayMicroseconds(70);
   digitalWrite(CLK, LOW);
}
 
void loop() {
  unsigned long count;
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
  delay(5);
   
}
