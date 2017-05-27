#include <RFduinoGZLL.h>

device_t role = DEVICE0;
#define SSID        "FRANCO_HIG"
#define PASSWORD    ""
#define HOST_NAME   "192.168.1.9"
#define HOST_PORT   (8125)

void initESP(void);
char buf[20] ={0};


void setup(void)
{
  RFduinoGZLL.begin(role);
  Serial.begin(9600);
  wifiRestart();
  delay(3000);
  initESP();
}
 
void loop(void)
{
    static uint32_t counter = 0;
    uint8_t buffer[128] = {0};
    
    if (wifiCreateTCP(HOST_NAME, HOST_PORT)) {
         RFduinoGZLL.sendToHost("create tcp ok\r\n");
    } else {
         RFduinoGZLL.sendToHost("create tcp err\r\n");
        wifiRestart();
        delay(3000);
        initESP();
    }

    
    String data = "Counter: " + String(counter,DEC);
     RFduinoGZLL.sendToHost(data);
    data.toCharArray(buf, 20);
    int dataLenght = data.length();
    buf[dataLenght] = 0x1E;
    wifiSend((const uint8_t*)buf, (uint32_t)data.length() + 1);
    
    
    uint32_t len = wifiRecv(buffer, sizeof(buffer), 10000);
    if (len > 0) {
         RFduinoGZLL.sendToHost("Received:[");
        for(uint32_t i = 0; i < len; i++) {
             RFduinoGZLL.sendToHost((char)buffer[i]);
        }
         RFduinoGZLL.sendToHost("]\r\n");
    }
    
    if (wifiReleaseTCP()) {
         RFduinoGZLL.sendToHost("release tcp ok\r\n");
    } else {
         RFduinoGZLL.sendToHost("release tcp err\r\n");
    }

    counter++;
    if(counter < 0 ) counter = 0;
    delay(100);
    
}

void initESP(void){
    RFduinoGZLL.sendToHost("setup begin\r\n");
    
    RFduinoGZLL.sendToHost("FW Version:");
    RFduinoGZLL.sendToHost(wifiGetLocalIP().c_str());
      
    if (wifiSetOprToStation()) {
        RFduinoGZLL.sendToHost("to station ok\r\n");
    } else {
        RFduinoGZLL.sendToHost("to station err\r\n");
    }
 
    if (wifiJoinAP(SSID, PASSWORD)) {
        RFduinoGZLL.sendToHost("Join AP success\r\n");
        RFduinoGZLL.sendToHost("IP:");
        RFduinoGZLL.sendToHost( wifiGetLocalIP().c_str());       
    } else {
        RFduinoGZLL.sendToHost("Join AP failure\r\n");
    }
    
    if (wifiDisableMUX()) {
        RFduinoGZLL.sendToHost("single ok\r\n");
    } else {
        RFduinoGZLL.sendToHost("single err\r\n");
    }
    RFduinoGZLL.sendToHost("setup end\r\n");
}

bool wifiReleaseTCP(void)
{
    return eATCIPCLOSESingle();
}


uint32_t recvPkg(uint8_t *buffer, uint32_t buffer_size, uint32_t *data_len, uint32_t timeout, uint8_t *coming_mux_id)
{
    String data;
    char a;
    int32_t index_PIPDcomma = -1;
    int32_t index_colon = -1; /* : */
    int32_t index_comma = -1; /* , */
    int32_t len = -1;
    int8_t id = -1;
    bool has_data = false;
    uint32_t ret;
    unsigned long start;
    uint32_t i;
    
    if (buffer == NULL) {
        return 0;
    }
    
    start = millis();
    while (millis() - start < timeout) {
        if(Serial.available() > 0) {
            a = Serial.read();
            data += a;
        }
        
        index_PIPDcomma = data.indexOf("+IPD,");
        if (index_PIPDcomma != -1) {
            index_colon = data.indexOf(':', index_PIPDcomma + 5);
            if (index_colon != -1) {
                index_comma = data.indexOf(',', index_PIPDcomma + 5);
                /* +IPD,id,len:data */
                if (index_comma != -1 && index_comma < index_colon) { 
                    id = data.substring(index_PIPDcomma + 5, index_comma).toInt();
                    if (id < 0 || id > 4) {
                        return 0;
                    }
                    len = data.substring(index_comma + 1, index_colon).toInt();
                    if (len <= 0) {
                        return 0;
                    }
                } else { /* +IPD,len:data */
                    len = data.substring(index_PIPDcomma + 5, index_colon).toInt();
                    if (len <= 0) {
                        return 0;
                    }
                }
                has_data = true;
                break;
            }
        }
    }
    
    if (has_data) {
        i = 0;
        ret = len > buffer_size ? buffer_size : len;
        start = millis();
        while (millis() - start < 3000) {
            while(Serial.available() > 0 && i < ret) {
                a = Serial.read();
                buffer[i++] = a;
            }
            if (i == ret) {
                rx_empty();
                if (data_len) {
                    *data_len = len;    
                }
                if (index_comma != -1 && coming_mux_id) {
                    *coming_mux_id = id;
                }
                return ret;
            }
        }
    }
    return 0;
}


uint32_t wifiRecv(uint8_t *buffer, uint32_t buffer_size, uint32_t timeout)
{
    return recvPkg(buffer, buffer_size, NULL, timeout, NULL);
}

bool wifiSend(const uint8_t *buffer, uint32_t len)
{
    return sATCIPSENDSingle(buffer, len);
}

bool wifiCreateTCP(String addr, uint32_t port)
{
    return sATCIPSTARTSingle("TCP", addr, port);
}

String wifiGetLocalIP(void)
{
    String list;
    eATCIFSR(list);
    return list;
}

bool wifiDisableMUX(void)
{
    return sATCIPMUX(0);
}


bool wifiJoinAP(String ssid, String pwd)
{
    return sATCWJAP(ssid, pwd);
}

bool wifiRestart(void)
{
    unsigned long start;
    if (eATRST()) {
        delay(2000);
        start = millis();
        while (millis() - start < 3000) {
            if (eAT()) {
                delay(1500); /* Waiting for stable */
                return true;
            }
            delay(100);
        }
    }
    return false;
}

String wifiGetVersion(void)
{
    String version;
    eATGMR(version);
    return version;
}

bool wifiSetOprToStation(void)
{
    uint8_t mode;
    if (!qATCWMODE(&mode)) {
        return false;
    }
    if (mode == 1) {
        return true;
    } else {
        if (sATCWMODE(1) && wifiRestart()) {
            return true;
        } else {
            return false;
        }
    }
}


void rx_empty(void) 
{
    while(Serial.available() > 0) {
        Serial.read();
    }
}

bool recvFind(String target, uint32_t timeout=1000)
{
    String data_tmp;
    data_tmp = recvString(target, timeout);
    if (data_tmp.indexOf(target) != -1) {
        return true;
    }
    return false;
}

String recvString(String target, uint32_t timeout)
{
    String data;
    char a;
    unsigned long start = millis();
    while (millis() - start < timeout) {
        while(Serial.available() > 0) {
            a = Serial.read();
      if(a == '\0') continue;
            data += a;
        }
        if (data.indexOf(target) != -1) {
            break;
        }   
    }
    return data;
}

String recvString(String target1, String target2, uint32_t timeout=1000)
{
    String data;
    char a;
    unsigned long start = millis();
    while (millis() - start < timeout) {
        while(Serial.available() > 0) {
            a = Serial.read();
      if(a == '\0') continue;
            data += a;
        }
        if (data.indexOf(target1) != -1) {
            break;
        } else if (data.indexOf(target2) != -1) {
            break;
        }
    }
    return data;
}

String recvString(String target1, String target2, String target3, uint32_t timeout=1000)
{
    String data;
    char a;
    unsigned long start = millis();
    while (millis() - start < timeout) {
        while(Serial.available() > 0) {
            a = Serial.read();
      if(a == '\0') continue;
            data += a;
        }
        if (data.indexOf(target1) != -1) {
            break;
        } else if (data.indexOf(target2) != -1) {
            break;
        } else if (data.indexOf(target3) != -1) {
            break;
        }
    }
    return data;
}

bool recvFindAndFilter(String target, String begin, String end, String &data, uint32_t timeout=1000)
{
    String data_tmp;
    data_tmp = recvString(target, timeout);
    if (data_tmp.indexOf(target) != -1) {
        int32_t index1 = data_tmp.indexOf(begin);
        int32_t index2 = data_tmp.indexOf(end);
        if (index1 != -1 && index2 != -1) {
            index1 += begin.length();
            data = data_tmp.substring(index1, index2);
            return true;
        }
    }
    data = "";
    return false;
}


bool eAT(void)
{
    rx_empty();
    Serial.println("AT");
    return recvFind("OK");
}

bool eATRST(void) 
{
    rx_empty();
    Serial.println("AT+RST");
    return recvFind("OK");
}

bool eATGMR(String &version)
{
    rx_empty();
    Serial.println("AT+GMR");
    return recvFindAndFilter("OK", "\r\r\n", "\r\n\r\nOK", version); 
}

bool qATCWMODE(uint8_t *mode) 
{
    String str_mode;
    bool ret;
    if (!mode) {
        return false;
    }
    rx_empty();
    Serial.println("AT+CWMODE?");
    ret = recvFindAndFilter("OK", "+CWMODE:", "\r\n\r\nOK", str_mode); 
    if (ret) {
        *mode = (uint8_t)str_mode.toInt();
        return true;
    } else {
        return false;
    }
}


bool sATCWMODE(uint8_t mode)
{
    String data;
    rx_empty();
    Serial.print("AT+CWMODE=");
    Serial.println(mode);
    
    data = recvString("OK", "no change");
    if (data.indexOf("OK") != -1 || data.indexOf("no change") != -1) {
        return true;
    }
    return false;
}

bool sATCWJAP(String ssid, String pwd)
{
    String data;
    rx_empty();
    Serial.print("AT+CWJAP=\"");
    Serial.print(ssid);
    Serial.print("\",\"");
    Serial.print(pwd);
    Serial.println("\"");
    
    data = recvString("OK", "FAIL", 10000);
    if (data.indexOf("OK") != -1) {
        return true;
    }
    return false;
}

bool sATCIPMUX(uint8_t mode)
{
    String data;
    rx_empty();
    Serial.print("AT+CIPMUX=");
    Serial.println(mode);
    
    data = recvString("OK", "Link is builded");
    if (data.indexOf("OK") != -1) {
        return true;
    }
    return false;
}

bool eATCIFSR(String &list)
{
    rx_empty();
    Serial.println("AT+CIFSR");
    return recvFindAndFilter("OK", "\r\r\n", "\r\n\r\nOK", list);
}

bool sATCIPSTARTSingle(String type, String addr, uint32_t port)
{
    String data;
    rx_empty();
    Serial.print("AT+CIPSTART=\"");
    Serial.print(type);
    Serial.print("\",\"");
    Serial.print(addr);
    Serial.print("\",");
    Serial.println(port);
    
    data = recvString("OK", "ERROR", "ALREADY CONNECT", 10000);
    if (data.indexOf("OK") != -1 || data.indexOf("ALREADY CONNECT") != -1) {
        return true;
    }
    return false;
}

bool sATCIPSENDSingle(const uint8_t *buffer, uint32_t len)
{
    rx_empty();
    Serial.print("AT+CIPSEND=");
    Serial.println(len);
    if (recvFind(">", 5000)) {
        rx_empty();
        for (uint32_t i = 0; i < len; i++) {
            Serial.write(buffer[i]);
        }
        return recvFind("SEND OK", 10000);
    }
    return false;
}

bool eATCIPCLOSESingle(void)
{
    rx_empty();
    Serial.println("AT+CIPCLOSE");
    return recvFind("OK", 5000);
}

