#include <RFduinoGZLL.h>

device_t role = HOST;

void setup()
{
  Serial.begin(9600);
  // start the GZLL stack  
  RFduinoGZLL.begin(role);
}

void loop()
{
}

void RFduinoGZLL_onReceive(device_t device, int rssi, char *data, int len)
{
    Serial.write((uint8_t *)data,len);
}
