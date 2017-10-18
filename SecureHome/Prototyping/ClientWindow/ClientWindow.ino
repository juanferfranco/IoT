#include <ESP8266WiFi.h>

const char* ssid = "TP-LINK_DE02";
const char* password = "upb2000qwerty";
const uint16_t port = 3000;
const char * host = "192.168.0.102"; // ip or dns

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
}


void loop() {
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

      uint8_t windowSensorState = digitalRead(WINDOWSENSORPORT);
      unsigned long currentMillis = millis();

      if ((currentMillis - previousMillis >= interval)) {
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
      }
      break;
  }
}

