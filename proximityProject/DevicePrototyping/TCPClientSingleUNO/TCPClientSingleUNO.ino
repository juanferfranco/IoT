#include "ESP8266.h"

#include <SoftwareSerial.h>

#define SSID        "UPBWiFi"
#define PASSWORD    ""
#define HOST_NAME   "10.9.197.142"
#define HOST_PORT   (5000)

SoftwareSerial mySerial(3, 2); /* RX:D3, TX:D2 */
ESP8266 wifi(mySerial);

void initESP(void);

char buf[20] ={0};


void setup(void)
{
    wifi.restart();
    delay(3000);
    Serial.begin(9600);
    initESP();
}
 
void loop(void)
{
    static uint32_t counter = 0;
    uint8_t buffer[128] = {0};
    
    if (wifi.createTCP(HOST_NAME, HOST_PORT)) {
        Serial.print("create tcp ok\r\n");
    } else {
        Serial.print("create tcp err\r\n");
        wifi.restart();
        delay(3000);
        initESP();
    }

    
    String data = "Counter: " + String(counter,DEC);
    Serial.println(data);
    data.toCharArray(buf, 20);
    int dataLenght = data.length();
    buf[dataLenght] = 0x1E;
    wifi.send((const uint8_t*)buf, (uint32_t)data.length() + 1);
    
    
    uint32_t len = wifi.recv(buffer, sizeof(buffer), 10000);
    if (len > 0) {
        Serial.print("Received:[");
        for(uint32_t i = 0; i < len; i++) {
            Serial.print((char)buffer[i]);
        }
        Serial.print("]\r\n");
    }
    
    if (wifi.releaseTCP()) {
        Serial.print("release tcp ok\r\n");
    } else {
        Serial.print("release tcp err\r\n");
    }

    counter++;
    if(counter < 0 ) counter = 0;
    delay(100);
    
}

void initESP(void){
    Serial.print("setup begin\r\n");
    
    Serial.print("FW Version:");
    Serial.println(wifi.getVersion().c_str());
      
    if (wifi.setOprToStation()) {
        Serial.print("to station ok\r\n");
    } else {
        Serial.print("to station err\r\n");
    }
 
    if (wifi.joinAP(SSID, PASSWORD)) {
        Serial.print("Join AP success\r\n");
        Serial.print("IP:");
        Serial.println( wifi.getLocalIP().c_str());       
    } else {
        Serial.print("Join AP failure\r\n");
    }
    
    if (wifi.disableMUX()) {
        Serial.print("single ok\r\n");
    } else {
        Serial.print("single err\r\n");
    }
    
    Serial.print("setup end\r\n");

}
     
