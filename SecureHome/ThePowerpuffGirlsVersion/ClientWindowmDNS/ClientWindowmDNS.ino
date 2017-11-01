#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>

const char* ssid = "TP-LINK_DE02";
const char* password = "upb2000qwerty";
uint16_t port = 3000;
IPAddress host(192,168,0,104); // ip or dns

#define MESSAGESIZE 10
#define SENSORTYPE 0
#define SENSORSTATE 1
#define UID_BYTE7 2
#define UID_BYTE6 3
#define UID_BYTE5 4
#define UID_BYTE4 5
#define UID_BYTE3 6
#define UID_BYTE2 7
#define UID_BYTE1 8
#define UID_BYTE0 9

uint8_t message[MESSAGESIZE] = {'w', 'c', 0, 0, 0, 0, 0, 0, 0, 0};

#define LED_WIFI D0
#define LED_CONNECTED D1
#define WINDOWSENSORPORT D2

#define TOCONNECT 0
#define CONNECTED 1
#define CONFIRMATION 2
#define CONFIRMATIONTIMEOUT 5000

uint8_t state = TOCONNECT;
WiFiClient clientRef;



unsigned long previousMillis = 0;
const long interval = 1000;


void setup() {
  pinMode(LED_WIFI, OUTPUT);
  pinMode(LED_CONNECTED, OUTPUT);
  pinMode(WINDOWSENSORPORT, INPUT);
  digitalWrite(LED_WIFI, LOW);
  digitalWrite(LED_CONNECTED, LOW);
  Serial.begin(115200); // initialize serial communication
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  digitalWrite(LED_WIFI, HIGH);

  // Start mDNS query
  if (!MDNS.begin("espWindow")) {
    Serial.println("Error setting up MDNS responder!");
  }
  Serial.println("mDNS responder started");
  MDNS.addService("espWindow", "tcp", 3000); // Announce esp tcp service on port 8080

  Serial.println("Sending mDNS query");
  int n = MDNS.queryService("espServer", "tcp"); // Send out query for esp tcp services
  Serial.println("mDNS query done");

  while (n == 0) {
    Serial.println("no services found");
    delay(2000);
  }
  Serial.print("Host: ");
  Serial.print(MDNS.hostname(0));
  Serial.print(" (");
  Serial.print(MDNS.IP(0));
  Serial.print(":");
  Serial.print(MDNS.port(0));
  Serial.println(")");
  Serial.println();

  host = MDNS.IP(0);
  port = MDNS.port(0);
}


void loop() {
  unsigned long currentMillis;
  uint8_t windowSensorState;

  MDNS.update();

  switch (state)
  {
    case TOCONNECT:
      if (!clientRef.connect(host, port)) {
        Serial.println("connection failed");
        Serial.println("wait 5 sec...");
        delay(5000);
      }
      else {
        digitalWrite(LED_CONNECTED, HIGH);
        state = CONNECTED;
        previousMillis = millis();
      }
      break;

    case CONNECTED:

      windowSensorState = digitalRead(WINDOWSENSORPORT);
      currentMillis = millis();

      if ((currentMillis - previousMillis) >= interval) {
        previousMillis = currentMillis;
        if (windowSensorState == HIGH) message[SENSORSTATE] = 'o';
        else message[SENSORSTATE] = 'c';


        if (clientRef.connected()) {
          clientRef.write((const uint8_t *)message, MESSAGESIZE);
        }
        else {
          clientRef.stop();
          state = TOCONNECT;
          digitalWrite(LED_CONNECTED, LOW);
        }


        state = CONFIRMATION;
        previousMillis = millis();


      }
      break;

    case CONFIRMATION:
      currentMillis  = millis();
      if ((currentMillis - previousMillis) >= CONFIRMATIONTIMEOUT) {
        clientRef.stop();
        state = TOCONNECT;
        digitalWrite(LED_CONNECTED, LOW);
        Serial.println("TIMEOUT");
      }
      else {

        if (clientRef.available()) {

          Serial.println("SERVER");
          if (clientRef.read() == 'l') {
            state = CONNECTED;
            previousMillis = millis();
          }
        }
      }
      break;
  }
}

