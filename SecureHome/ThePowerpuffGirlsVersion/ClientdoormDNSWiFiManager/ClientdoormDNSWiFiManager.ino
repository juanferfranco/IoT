#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <SPI.h>
#include <MFRC522.h>

// libraries for WiFiManager
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager

#define RST_PIN  D1 // RST-PIN for RC522 - RFID - SPI - Modul GPIO5: 5 
#define SS_PIN  D2 // SDA-PIN for RC522 - RFID - SPI - Modul GPIO4: 4
#define LED_RFID D8 // LED RFID read 
#define TRIGGER_WIFIMANAGER D4

const char* ssid = "fun";
const char* password = "upb2000qwerty";
uint16_t port = 3000;
IPAddress host(192, 168, 0, 104); // ip or dns

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

uint8_t message[MESSAGESIZE] = {'d', 'c', 0, 0, 0, 0, 0, 0, 0, 0};

#define LED_WIFI D9 //D9
#define LED_CONNECTED D10 // D10
#define DOORSENSORPORT D0
#define ACTUADOR D3


#define TOCONNECT 0
#define CONNECTED 1
#define CONFIRMATION 2
#define CONFIRMATIONTIMEOUT 5000

uint8_t state = TOCONNECT;
WiFiClient clientRef;

unsigned long previousMillis = 0;
const long interval = 1000;

MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance


void setup() {
  pinMode(TRIGGER_WIFIMANAGER, INPUT);
  pinMode(LED_WIFI, OUTPUT);
  pinMode(LED_CONNECTED, OUTPUT);
  pinMode(DOORSENSORPORT, INPUT);
  pinMode(ACTUADOR, OUTPUT);
  digitalWrite(ACTUADOR, LOW);

  digitalWrite(LED_WIFI, LOW);
  digitalWrite(LED_CONNECTED, LOW);
  Serial.begin(115200); // initialize serial communication

  Serial.println();
  Serial.println("Init SPI ");
  SPI.begin();      // Init SPI bus
  Serial.println("SPI done ");
  Serial.println("Init RFID sensor ");
  mfrc522.PCD_Init();   // Init MFRC522
  Serial.println("SPI done ");

  pinMode(LED_RFID, OUTPUT);

  WiFiManager wifiManager;

  if (digitalRead(TRIGGER_WIFIMANAGER) == LOW) {
    if (!wifiManager.startConfigPortal("Door Sensor AutoConnectAP")) {
      Serial.println("failed to connect and hit timeout");
      delay(3000);
      //reset and try again, or maybe put it to deep sleep
      ESP.reset();
      delay(5000);
    }
  }
  else {
    wifiManager.autoConnect("Door Sensor AutoConnectAP");
  }

  Serial.print("\nConnecting to ");
  Serial.println(WiFi.SSID());
  digitalWrite(LED_WIFI, HIGH);

  // Start mDNS query
  if (!MDNS.begin("espDoor")) {
    Serial.println("Error setting up MDNS responder!");
  }
  Serial.println("mDNS responder started");
  MDNS.addService("espDoor", "tcp", 3000); // Announce esp tcp service on port 8080

  int n = 0;
  while (n == 0) {
    Serial.println("Sending mDNS query");
    n = MDNS.queryService("espServer", "tcp"); // Send out query for esp tcp services
    Serial.println("mDNS query done");
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
  uint8_t dataFromServer;

  if (mfrc522.PICC_IsNewCardPresent()) {
    if (mfrc522.PICC_ReadCardSerial()) {

      message[UID_BYTE3] = mfrc522.uid.uidByte[0];
      message[UID_BYTE2] = mfrc522.uid.uidByte[1];
      message[UID_BYTE1] = mfrc522.uid.uidByte[2];
      message[UID_BYTE0] = mfrc522.uid.uidByte[3];


      mfrc522.PICC_HaltA();
      // Stop encryption on PCD
      mfrc522.PCD_StopCrypto1();
      digitalWrite(LED_RFID, HIGH);
      delay(500);
      digitalWrite(LED_RFID, LOW);
      delay(500);
    }
  }


  switch (state)
  {
    case TOCONNECT:
      Serial.println("Trying Server connection");
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

      windowSensorState = digitalRead(DOORSENSORPORT);
      //uint8_t windowSensorState = LOW;

      currentMillis = millis();
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
          dataFromServer = clientRef.read();
          state = CONNECTED;
          previousMillis = millis();
          message[UID_BYTE3] = 0;
          message[UID_BYTE2] = 0;
          message[UID_BYTE1] = 0;
          message[UID_BYTE0] = 0;

          if ( dataFromServer == 'o') {
            digitalWrite(ACTUADOR, HIGH);
          }
          if ( dataFromServer == 'c') {
            digitalWrite(ACTUADOR, LOW);
          }
        }
      }
      break;
  }
}

