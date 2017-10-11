#include <ESP8266WiFi.h>

const char* ssid = "TP-LINK_DE02";
const char* password = "upb2000qwerty";

WiFiServer server(3000);

void setup() {
  Serial.begin(115200); // initialize serial communication
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  server.begin();
  server.setNoDelay(true);
  Serial.print("Ready! Listening");
  Serial.print(WiFi.localIP());
  Serial.println(" 3000' to connect");
}


void loop() {
  // listen for incoming clients
  WiFiClient client = server.available();

  if (Serial.available()) {
    char data = (char)Serial.read();
    if (data == 'i') {
      Serial.println("");
      Serial.println("WiFi connected");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
      return;
    }
  }


  if (client) {
    if (client.connected()) {
      Serial.println("Connected to client");
      client.println("Hello From Server");
    }

    // close the connection:
    client.stop();
  }
}
