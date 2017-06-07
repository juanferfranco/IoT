#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <MFRC522.h>

#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include <EEPROM.h>


#define USE_SERIAL Serial
ESP8266WiFiMulti WiFiMulti;

#define RST_PIN  5 // RST-PIN for RC522 - RFID - SPI - Modul GPIO5 
#define SS_PIN  4 // SDA-PIN for RC522 - RFID - SPI - Modul GPIO4 

MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance

#define LEDCARD 15


#define INITPAYLOAD 38  // WARNING!!!!!!!! Adjust this constant to point at the end of strReq (just after '/')
char strReq[64] = "http://ensayoerror.pythonanywhere.com/";


void setup() {

  pinMode(LEDCARD, OUTPUT);
  SPI.begin();      // Init SPI bus
  mfrc522.PCD_Init();   // Init MFRC522
  USE_SERIAL.begin(115200);
  USE_SERIAL.println();
  USE_SERIAL.println();
  USE_SERIAL.println();
  for (uint8_t t = 4; t > 0; t--) {
    USE_SERIAL.printf("[SETUP] WAIT %d...\n", t);
    USE_SERIAL.flush();
    delay(1000);
  }
  setupDevice();
}

void loop() {
  char strTmp[15];

  if (USE_SERIAL.available() > 0) {
    uint8_t data = USE_SERIAL.read();
    if ( data == 'n') {
      WiFi.disconnect();
      ESP.reset();
      delay(2000);
    }
    if (data == 'f') {
      USE_SERIAL.println(ESP.getFreeHeap());
    }
    if (data == 'c') {
      USE_SERIAL.println(F("Reconnecting"));
      WiFiManager wifiManager;
      wifiManager.autoConnect("RFIDTIC");
    }
  }

  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  strReq[INITPAYLOAD] = NULL;

  for (uint8_t i = 0; i < mfrc522.uid.size; i++) {
    sprintf(strTmp, "%02X", mfrc522.uid.uidByte[i]);
    strcat(strReq, strTmp);
  }
  // Halt PICC
  mfrc522.PICC_HaltA();
  // Stop encryption on PCD
  mfrc522.PCD_StopCrypto1();

  USE_SERIAL.println("TEST 2");

  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {
    HTTPClient http;

    USE_SERIAL.println("Sending UID");
    http.begin(strReq);
    USE_SERIAL.print("[HTTP] GET...\n");
    int httpCode = http.GET();
    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);


      // file found at server
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        USE_SERIAL.println(payload);
      }
    } else {
      USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
    if (httpCode == HTTP_CODE_OK) {
      digitalWrite(LEDCARD, HIGH);
      delay(500);
      digitalWrite(LEDCARD, LOW);
      delay(500);
    }
  }
  else{
    USE_SERIAL.println("WiFi disconnected");
  }
}

void setupDevice() {
  char deviceName[15] = {0};
  WiFiManager wifiManager;
  EEPROM.begin(16);

  // Is there a device name in EEPROM[0] ? then load wifiManager's device name
  uint8_t value = EEPROM.read(0);
  if (value == '1') {
    USE_SERIAL.println(F("Read EEROM data"));
    uint8_t address = 0;
    while (true) {
      value = EEPROM.read(address + 1);
      if (value != 0) {
        deviceName[address] = value;
        address++;
      }
      else break;
    }
    deviceName[address] = 0;
    wifiManager.initDeviceName(deviceName);
  }

  wifiManager.autoConnect("RFIDTIC");
  USE_SERIAL.println(wifiManager.getDeviceName());
  
  if (wifiManager.hasDeviceName() == true) {
    USE_SERIAL.println(F("Write EEROM data"));
    EEPROM.write(0, '1'); // Indicate that there is name stored
    char* pname = wifiManager.getDeviceName();
    uint8_t address = 0;
    while (pname[address] != 0) {
      EEPROM.write(address + 1, pname[address]);
      address++;
    }
    EEPROM.write(address+1, 0);
    EEPROM.commit();
    delay(100);
  }
}

