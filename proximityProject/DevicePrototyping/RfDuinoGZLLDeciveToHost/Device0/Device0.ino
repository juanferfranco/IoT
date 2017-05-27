#include <RFduinoGZLL.h>

device_t role = DEVICE0;

void setup()
{
  // start the GZLL stack
  RFduinoGZLL.begin(role);
}

void loop()
{
  // send state to Host
  RFduinoGZLL.sendToHost("Hola mundo", 6);
  delay(2000);
}

