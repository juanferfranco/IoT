#include <ESP8266WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
/* WARNING change the source code of MFRC522 library to remove
  Soft Reset part of the init code and replace it with a hard reset.
*/

#define RST_PIN  D1 // RST-PIN for RC522 - RFID - SPI - Modul GPIO5: 5 
#define SS_PIN  D2 // SDA-PIN for RC522 - RFID - SPI - Modul GPIO4: 4
#define LED_RFID D8 // LED RFID read 

const char* ssid = "TP-LINK_DE02";
const char* password = "upb2000qwerty";
const uint16_t port = 3000;
const char * host = "192.168.0.104"; // ip or dns

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

uint8_t state = TOCONNECT;
WiFiClient clientRef;

unsigned long previousMillis = 0;
const long interval = 1000;

MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance


#define SERIAL_DEBUG Serial1

void readSensorRFID();
void readServerCommands();
void sendStatusToServer();

void setup() {
  pinMode(LED_WIFI, OUTPUT);
  pinMode(LED_CONNECTED, OUTPUT);
  pinMode(DOORSENSORPORT, INPUT);
  pinMode(ACTUADOR, OUTPUT);
  digitalWrite(ACTUADOR, LOW);

  digitalWrite(LED_WIFI, LOW);
  digitalWrite(LED_CONNECTED, LOW);
  SERIAL_DEBUG.begin(115200); // initialize serial communication

  SERIAL_DEBUG.println();
  SERIAL_DEBUG.println("Init SPI ");
  SPI.begin();      // Init SPI bus
  SERIAL_DEBUG.println("SPI done ");
  SERIAL_DEBUG.println("Init RFID sensor ");
  mfrc522.PCD_Init();   // Init MFRC522
  SERIAL_DEBUG.println("Init RFID sensor done ");

  pinMode(LED_RFID, OUTPUT);


  SERIAL_DEBUG.print("Connecting to ");
  SERIAL_DEBUG.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    SERIAL_DEBUG.print(".");
  }
  digitalWrite(LED_WIFI, HIGH);
  clientRef.setNoDelay(true);

}

void loop() {
  readSensorRFID();
  sendStatusToServer();
  readServerCommands();
}

void readSensorRFID() {
  //SERIAL_DEBUG.println("Enter readSensorRFID()");
  //SERIAL_DEBUG.println(millis());

  
  mfrc522.PCD_Init();   // Init MFRC522

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

      digitalWrite(RST_PIN,LOW);
    }
  }
  //SERIAL_DEBUG.println("Exit readSensorRFID()");
  //SERIAL_DEBUG.println(millis());
}
void sendStatusToServer() {
  switch (state)
  {
    case TOCONNECT:
      if (!clientRef.connect(host, port)) {
        SERIAL_DEBUG.println("connection failed");
        SERIAL_DEBUG.println("wait 5 sec...");
        delay(5000);
      }
      else {
        digitalWrite(LED_CONNECTED, HIGH);
        SERIAL_DEBUG.println("Connected");
        state = CONNECTED;
        previousMillis = millis();
      }
      break;

    case CONNECTED:

      uint8_t windowSensorState = digitalRead(DOORSENSORPORT);
      //uint8_t windowSensorState = LOW;

      unsigned long currentMillis = millis();
      if ((currentMillis - previousMillis >= interval)) {

        previousMillis = currentMillis;
        if (windowSensorState == HIGH) message[SENSORSTATE] = 'o';
        else message[SENSORSTATE] = 'c';


        if (clientRef.connected()) {

          unsigned long millis1 = millis();
          //SERIAL_DEBUG.print("Enter write: ");
          //SERIAL_DEBUG.println(millis());

          clientRef.write((const uint8_t *)message, MESSAGESIZE);

          unsigned long millis2 = millis();

          SERIAL_DEBUG.print("write time: ");
          SERIAL_DEBUG.println(millis2 - millis1);
          if ((millis2 - millis1) > 2000) {
            SERIAL_DEBUG.println("Server connection timeout");
            clientRef.stop();
            state = TOCONNECT;
            digitalWrite(LED_CONNECTED, LOW);
            SERIAL_DEBUG.println("Disconnected");
          }


          //SERIAL_DEBUG.println("Send data to server");
        }
        else {
          clientRef.stop();
          state = TOCONNECT;
          digitalWrite(LED_CONNECTED, LOW);
          SERIAL_DEBUG.println("Disconnected");

        }

      }




      break;
  }
}

void readServerCommands() {
  //SERIAL_DEBUG.println("Enter readServerCommands()");
  //SERIAL_DEBUG.println(millis());
  if (clientRef.available()) {
    while (clientRef.available())
      if (clientRef.read() == 'o') {
        digitalWrite(ACTUADOR, HIGH);
      }
      else {
        digitalWrite(ACTUADOR, LOW);
      }
  }
  //SERIAL_DEBUG.println("Exit readServerCommands()");
  //SERIAL_DEBUG.println(millis());
}

