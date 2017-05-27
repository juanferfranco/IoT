#include "ESP8266.h"
#include <RFduinoGZLL.h>

#define SSID        "DTVNET_3E281A"
#define PASSWORD    "owbmtmet"
#define HOST_NAME   "192.168.100.7"
#define HOST_PORT   (8125)

void initESP(void);
void sendZone(char* buf);
ESP8266 wifi(Serial);

/*
GZLL part
*/
#define  MAX_DEVICES  3
// RSSI total and count for each device for averaging
int rssi_total[MAX_DEVICES];
int rssi_count[MAX_DEVICES];
// device with the maximum RSSI
char closest_device = 0;

// collect samples flag
int collect_samples = 0;

char* zoneOn = "{\"zone\":1,\"state\":\"on\"}";
char* zoneOff = "{\"zone\":1,\"state\":\"off\"}";

void setup(void)
{
    pinMode(2, OUTPUT);  //red
    digitalWrite(2,HIGH);
    wifi.initUart(9600);
    digitalWrite(2,LOW);
    delay(500);
    digitalWrite(2,HIGH);
    delay(1000);
    wifi.restart();
    // start the GZLL stack
    RFduinoGZLL.begin(HOST);
    initESP();
}
 
void loop()
{
  int i;
  // reset the RSSI averaging for each device
  for (i = 0; i < MAX_DEVICES; i++)
  {
    rssi_total[i] = 0;
    rssi_count[i] = 0;
  }

  // start collecting RSSI samples
  collect_samples = 1;

  // wait one second
  delay(1000);
  
  // stop collecting RSSI samples
  collect_samples = 0;

  // calculate the RSSI avarages for each device
  int average[MAX_DEVICES];
 
  for (i = 0; i < MAX_DEVICES; i++)
  {
    // no samples received, set to the lowest RSSI
    // (also prevents divide by zero)
    if (rssi_count[i] == 0)
      average[i] = -128;
    else
      average[i] = rssi_total[i] / rssi_count[i];
  }

  if(average[0] > -85) sendZone(zoneOn);
  else if(average[0] < -90) sendZone(zoneOff);
  
  // find the device with the maximum RSSI value
  int closest = 0;
  for (i = 1; i < MAX_DEVICES; i++)
    if (average[i] > average[closest])
      closest = i;

  closest_device = closest;
}

void RFduinoGZLL_onReceive(device_t device, int rssi, char *data, int len)
{
  // ignore device if outside range
  if (device > MAX_DEVICES)
    return;
    
  // if collecting samples, update the RSSI total and count
  if (collect_samples)
  {
    rssi_total[device] += rssi;
    rssi_count[device]++;
  }
  
  // piggyback max_device on the acknowledgement sent back to the requesting Device
  RFduinoGZLL.sendToDevice(device, closest_device);
}


void sendZone(char* buf){
    bool flag = true;
    while(flag){
      if (wifi.createTCP(HOST_NAME, HOST_PORT)) {
          flag = false;
          delay(100);
      } else {
          digitalWrite(2,LOW);
          delay(500);
          digitalWrite(2,HIGH);
          delay(1000);          
          wifi.restart();
          initESP();
      }
    }
    wifi.send((const uint8_t*)buf, (uint32_t)strlen(buf));
    delay(100);
    if (wifi.releaseTCP()) {
        delay(100);
    }
}

void initESP(void){
    wifi.setOprToStation();
    wifi.joinAP(SSID, PASSWORD);
    wifi.disableMUX();
}
     
