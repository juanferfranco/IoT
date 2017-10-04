#include <ESP8266WiFi.h>

const char* ssid     = "UPBWiFi";
const char* password = "";
const char* host = "maker.ifttt.com";
const char* privateKey = "YOUR_KEY";
const char* event = "YOUR_NOTIFICATION";

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
  if (Serial.available()) {
    char data = (char)Serial.read();
    if (data == 'i') {
      Serial.println("");
      Serial.println("WiFi connected");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
    }
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
}


