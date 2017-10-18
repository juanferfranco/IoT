/*
  WiFiTelnetToSerial - Example Transparent UART to Telnet Server for esp8266

  Copyright (c) 2015 Hristo Gochkov. All rights reserved.
  This file is part of the ESP8266WiFi library for Arduino environment.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include <ESP8266WiFi.h>

//how many clients should be able to telnet to this ESP8266
#define MAX_SRV_CLIENTS 3
const char* ssid = "TP-LINK_DE02";
const char* password = "upb2000qwerty";

WiFiServer server(3000);
WiFiClient serverClients[MAX_SRV_CLIENTS];

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

void processDataFromClients();
void checkNewClients();

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.print("\nConnecting to ");
  Serial.println(ssid);
  uint8_t i = 0;
  while (WiFi.status() != WL_CONNECTED && i++ < 20) delay(500);
  if (i == 21) {
    Serial.print("Could not connect to"); Serial.println(ssid);
    while (1) delay(500);
  }
  //start UART and the server
  server.begin();
  server.setNoDelay(true);
  Serial.print("Ready! Use 'telnet ");
  Serial.print(WiFi.localIP());
  Serial.println(" 23' to connect");
}

void loop() {
  checkNewClients();
  processDataFromClients();
}

void processDataFromClients() {
  for (uint8_t i = 0; i < MAX_SRV_CLIENTS; i++) {
    if (serverClients[i] && serverClients[i].connected()) {
      if (serverClients[i].available() == MESSAGESIZE) {
        while (serverClients[i].available()) {
          uint8_t message[MESSAGESIZE] = {0};
          for (uint8_t j  = 0; j < MESSAGESIZE; j++) {
            message[j] = serverClients[i].read();
          }
          
          Serial.print("SENSOR TYPE: ");
          Serial.println(message[SENSORTYPE]);
          Serial.print("SENSOR STATE: ");
          Serial.println(message[SENSORSTATE]);
          Serial.print("UID: ");
          Serial.print(message[UID_BYTE7]);
          Serial.print(message[UID_BYTE6]);
          Serial.print(message[UID_BYTE5]);
          Serial.print(message[UID_BYTE4]);
          Serial.print(message[UID_BYTE3]);
          Serial.print(message[UID_BYTE2]);
          Serial.print(message[UID_BYTE1]);
          Serial.println(message[UID_BYTE0]);
        }
      }
    }
  }
}

void checkNewClients() {
  //check if there are any new clients
  if (server.hasClient()) {
    for (uint8_t i = 0; i < MAX_SRV_CLIENTS; i++) {
      //find free/disconnected spot
      if (!serverClients[i] || !serverClients[i].connected()) {
        if (serverClients[i]) serverClients[i].stop();
        serverClients[i] = server.available();
        Serial.print("New client: "); Serial.print(i);
        continue;
      }
    }
    //no free/disconnected spot so reject
    WiFiClient serverClient = server.available();
    serverClient.stop();
  }

}

void sendDataToClients() {
  //check UART for data
  if (Serial.available()) {
    size_t len = Serial.available();
    uint8_t sbuf[len];
    Serial.readBytes(sbuf, len);
    //push UART data to all connected telnet clients
    for (uint8_t i = 0; i < MAX_SRV_CLIENTS; i++) {
      if (serverClients[i] && serverClients[i].connected()) {
        serverClients[i].write(sbuf, len);
        delay(1);
      }
    }
  }
}

