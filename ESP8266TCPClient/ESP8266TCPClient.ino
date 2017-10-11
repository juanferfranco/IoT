#include <ESP8266WiFi.h>

const char* ssid = "TP-LINK_DE02";
const char* password = "upb2000qwerty";
const uint16_t port = 23;
const char * host = "192.168.0.102"; // ip or dns


#define WAITCONNECT 0
#define CONNECTED 1

uint8_t state = WAITCONNECT;
WiFiClient clientRef;

#define TXBUFFERSIZE 20
uint8_t txBuffer[TXBUFFERSIZE];
size_t dataCounter = 0;


void setup() {
  Serial.begin(115200); // initialize serial communication
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
}


void loop() {
  uint8_t data;

  switch (state)
  {
    case WAITCONNECT:
      if (Serial.available()) {
        data = (char)Serial.read();
        if (data == 'i') {
          Serial.println("");
          Serial.println("WiFi connected");
          Serial.println("IP address: ");
          Serial.println(WiFi.localIP());
        }
        if (data == 'c') {
          Serial.println("CONNECTING...");
          if (!clientRef.connect(host, port)) {
            Serial.println("connection failed");
            Serial.println("wait 5 sec...");
            delay(5000);
          }
          else {
            state = CONNECTED;
            dataCounter = 0;
            Serial.println("CONNECTED");
          }
        }
      }

      break;

    case CONNECTED:
      if (Serial.available()) {
        data = (uint8_t)Serial.read();
        if ((data != '\n') && (dataCounter < TXBUFFERSIZE)) {
          txBuffer[dataCounter] = data;
          dataCounter = (dataCounter + 1) % TXBUFFERSIZE;
        }
        if (data == '\n') {
          clientRef.write((const uint8_t *)txBuffer, dataCounter);
          dataCounter = 0;
        }
        if (data == '*') {
          Serial.println("DISCONNECTED");
          clientRef.stop();
          state = WAITCONNECT;
          return;
        }
      }
      if (clientRef.available()) {
        while (clientRef.available()) {
          Serial.write(clientRef.read());
        }
      }
      if (!clientRef.connected()) {
        clientRef.stop();
        Serial.println("DISCONNECTED FROM SERVER");
        state = WAITCONNECT;
      }
      break;

  }
}



