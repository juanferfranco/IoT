#include "ESP8266.h"
#include <RFduinoGZLL.h>

#define SSID        "FRANCO_HIG"
#define PASSWORD    "funcholandia"
#define HOST_NAME   "192.168.1.4"
#define HOST_PORT   (8125)

void initESP(void);
char buf[20] ={0};

ESP8266 wifi(Serial);

void setup(void)
{
    pinMode(2, OUTPUT);  //red
    digitalWrite(2,HIGH);
    //pinMode(3, OUTPUT);  //greem
    //pinMode(4, OUTPUT); // blue
    wifi.initUart(9600);
    digitalWrite(2,LOW);
    delay(500);
    digitalWrite(2,HIGH);
    delay(1000);
    wifi.restart();
    RFduinoGZLL.begin(DEVICE0);
    initESP();
}
 
void loop(void)
{
    static uint32_t counter = 0;
    uint8_t buffer[128] = {0};
    bool flag = true;
    
    while(flag){
      if (wifi.createTCP(HOST_NAME, HOST_PORT)) {
          RFduinoGZLL.sendToHost("create tcp ok\r\n");
          flag = false;
          delay(100);
      } else {
          RFduinoGZLL.sendToHost("create tcp err\r\n");
          digitalWrite(2,LOW);
          delay(500);
          digitalWrite(2,HIGH);
          delay(1000);          
          wifi.restart();
          initESP();
      }
    }
    
    
    String data = "Counter: " + String(counter,DEC);
    RFduinoGZLL.sendToHost(data);
    data.toCharArray(buf, 20);
    int dataLenght = data.length();
    buf[dataLenght] = 0x1E;
    wifi.send((const uint8_t*)buf, (uint32_t)data.length() + 1);
    delay(100);
    
    
//    uint32_t len = wifi.recv(buffer, sizeof(buffer), 10000);
//    if (len > 0) {
//        RFduinoGZLL.sendToHost("Received:[");
//        for(uint32_t i = 0; i < len; i++) {
//            RFduinoGZLL.sendToHost((char)buffer[i]);
//        }
//        RFduinoGZLL.sendToHost("]\r\n");
//    }
    
    if (wifi.releaseTCP()) {
        RFduinoGZLL.sendToHost("release tcp ok\r\n");
        delay(100);
    } else {
        RFduinoGZLL.sendToHost("release tcp err\r\n");
    }
    counter++;
    if(counter < 0 ) counter = 0;
}

void initESP(void){
    RFduinoGZLL.sendToHost("setup begin\r\n");
    
    RFduinoGZLL.sendToHost("FW Version:");
    RFduinoGZLL.sendToHost(wifi.getVersion().c_str());
      
    if (wifi.setOprToStation()) {
        RFduinoGZLL.sendToHost("to station ok\r\n");
    } else {
        RFduinoGZLL.sendToHost("to station err\r\n");
    }
 
    if (wifi.joinAP(SSID, PASSWORD)) {
        RFduinoGZLL.sendToHost("Join AP success\r\n");
        RFduinoGZLL.sendToHost("IP:");
        RFduinoGZLL.sendToHost( wifi.getLocalIP().c_str());       
    } else {
        RFduinoGZLL.sendToHost("Join AP failure\r\n");
    }
    
    if (wifi.disableMUX()) {
        RFduinoGZLL.sendToHost("single ok\r\n");
    } else {
        RFduinoGZLL.sendToHost("single err\r\n");
    }
    
    RFduinoGZLL.sendToHost("setup end\r\n");

}
     
