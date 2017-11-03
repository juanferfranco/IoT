#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>

// libraries for WiFiManager
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager


//how many clients should be able to telnet to this ESP8266
#define MAX_SRV_CLIENTS 3
const char* host = "maker.ifttt.com";
const char* evAlarmTriggered = "correo";
const char* evAlarmArmed = "alarmaArmed";
const char* privateKey = "lmQacBAEY1SyfUdlXxS378N4bwSYOPGHsEYxp7MVjCT";

byte mac[6];
WiFiServer server(3000);
WiFiClient serverClients[MAX_SRV_CLIENTS];

#define LED_WIFI D0
#define LED_WINDOW D1
#define LED_DOOR D2
#define LED_ALARM D3
#define TRIGGER_WIFIMANAGER D5


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
uint8_t IFTTTCont = 0;

void processDataFromClients();
void checkNewClients();

#define ALARM_DISARMED 0
#define ALARM_ARMED 1
uint8_t alarmState =  ALARM_DISARMED;

#define APPROVEDKEYS 2
uint8_t approvedKeys[APPROVEDKEYS][4] = {{0xB6, 0x5A, 0x11, 0x9E}, {0xFF, 0xFF, 0xFF, 0xFF}};

uint8_t message[MESSAGESIZE] = {0};

uint8_t verifyKey();

void setup() {
  pinMode(TRIGGER_WIFIMANAGER, INPUT);
  pinMode(LED_WIFI, OUTPUT);
  pinMode(LED_WINDOW, OUTPUT);
  pinMode(LED_DOOR, OUTPUT);
  pinMode(LED_ALARM, OUTPUT);
  digitalWrite(LED_WIFI, LOW);
  digitalWrite(LED_WINDOW, LOW);
  digitalWrite(LED_DOOR, LOW);
  digitalWrite(LED_ALARM, LOW);

  Serial.begin(115200);

  WiFiManager wifiManager;

  if (digitalRead(TRIGGER_WIFIMANAGER) == LOW) {
    if (!wifiManager.startConfigPortal("Server AutoConnectAP")) {
      Serial.println("failed to connect and hit timeout");
      delay(3000);
      //reset and try again, or maybe put it to deep sleep
      ESP.reset();
      delay(5000);
    }
  }
  else {
    wifiManager.autoConnect("Server AutoConnectAP");
  }


  Serial.print("\nConnecting to ");
  Serial.println(WiFi.SSID());
  digitalWrite(LED_WIFI, HIGH);

  //start UART and the server
  server.begin();
  server.setNoDelay(true);
  Serial.print("Ready! Use 'telnet ");
  Serial.print(WiFi.localIP());
  Serial.println(" 3000' to connect");
  WiFi.macAddress(mac);
  for (uint8_t i = 0; i < 6 ; i++) {
    Serial.print(mac[i], HEX);
    Serial.print(':');
  }
  Serial.println();

  // Start mDNS responder
  if (!MDNS.begin("espServer")) {
    Serial.println("Error setting up MDNS responder!");
  }
  Serial.println("mDNS responder started");
  MDNS.addService("espServer", "tcp", 3000); // Announce esp tcp service on port 8080

}

void loop() {
  MDNS.update();
  checkNewClients();
  processDataFromClients();
  sendDataToClients();
}

void processDataFromClients() {
  for (uint8_t i = 0; i < MAX_SRV_CLIENTS; i++) {
    if (serverClients[i] && serverClients[i].connected()) {
      if (serverClients[i].available() == MESSAGESIZE) {
        while (serverClients[i].available()) {
          for (uint8_t j  = 0; j < MESSAGESIZE; j++) {
            message[j] = serverClients[i].read();
          }
          Serial.write(message[SENSORTYPE]);
          Serial.println();

          /*
            Serial.print("SENSOR TYPE: ");
            Serial.write(message[SENSORTYPE]);
            Serial.println();
            Serial.print("SENSOR STATE: ");
            Serial.write(message[SENSORSTATE]);
            Serial.println();
            Serial.print("UID: ");
            Serial.print(message[UID_BYTE3], HEX);
            Serial.print(message[UID_BYTE2], HEX);
            Serial.print(message[UID_BYTE1], HEX);
            Serial.println(message[UID_BYTE0], HEX);
          */

          switch (alarmState) {
            case ALARM_DISARMED:
              if (verifyKey() == 1) {
                digitalWrite(LED_ALARM, HIGH);
                sendAlert(evAlarmArmed);
                serverClients[i].write('o'); // inverted logic o: secure c: unsecure
                alarmState = ALARM_ARMED;
                Serial.println("Door is secured");
                IFTTTCont = 0;
              }
              break;
            case ALARM_ARMED:
              if ((message[SENSORSTATE] == 'o') && (IFTTTCont == 0)) {
                sendAlert(evAlarmTriggered);
                IFTTTCont = 1;
                Serial.println("Alarm is triggered");
              }
              if (verifyKey() == 1) {
                serverClients[i].write('c');
                digitalWrite(LED_ALARM, LOW);
                alarmState = ALARM_DISARMED;
                Serial.println("Door is unsecured");
              }
              break;
          }
          // ACK message to client
          serverClients[i].write('l');
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

void sendAlert(const char* event) {
  Serial.print("connecting to ");
  Serial.println(host);
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }
  String url = "/trigger/";
  url += event;
  url += "/with/key/";
  url += privateKey;
  Serial.print("Requesting URL: ");
  Serial.println(url);
  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");

  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }
  while (client.available()) {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  Serial.println();
  Serial.println("closing connection");
}

uint8_t verifyKey() {

  uint8_t keyApproved = 0;

  for (uint8_t key = 0; key < APPROVEDKEYS; key++) {
    if (!(message[UID_BYTE3] == approvedKeys[key][0])) {
      continue;
    }
    if (!(message[UID_BYTE2] == approvedKeys[key][1])) {
      continue;
    }
    if (!(message[UID_BYTE1] == approvedKeys[key][2])) {
      continue;
    }
    if (!(message[UID_BYTE0] == approvedKeys[key][3])) {
      continue;
    }
    keyApproved = 1;
    return keyApproved;
  }
}


