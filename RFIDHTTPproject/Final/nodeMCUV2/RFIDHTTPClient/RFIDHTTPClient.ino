#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <MFRC522.h>

#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic

#define USE_SERIAL Serial
ESP8266WiFiMulti WiFiMulti;

#define RST_PIN  5 // RST-PIN for RC522 - RFID - SPI - Modul GPIO5 
#define SS_PIN  4 // SDA-PIN for RC522 - RFID - SPI - Modul GPIO4 

MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance

#define LEDCARD 15


#define INITPAYLOAD 38  // WARNING!!!!!!!! Adjust this constant to point at the end of strReq (just after '/')
char strReq[64] = "http://ensayoerror.pythonanywhere.com/";


void setup() {
  ////////////////////////////////////
  // Temporal code to test WiFiManager.
  // Use this code to clear the wifi credentials.

  WiFi.disconnect();
  delay(1000);
  ///////////////////////////////////

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
  WiFiManager wifiManager;
  //WiFiMulti.addAP("UPBWiFi");
  wifiManager.autoConnect("RFIDTIC");

  USE_SERIAL.println("TEST");
}

void loop() {
  char strTmp[15];

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
    if(httpCode == HTTP_CODE_OK){
      digitalWrite(LEDCARD, HIGH);
      delay(500);
      digitalWrite(LEDCARD, LOW);
      delay(500);
    }
  }
}

