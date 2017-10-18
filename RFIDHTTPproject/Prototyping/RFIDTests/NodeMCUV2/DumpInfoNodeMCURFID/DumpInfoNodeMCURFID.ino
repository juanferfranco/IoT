#include <MFRC522.h>

#define RST_PIN  D1 // RST-PIN for RC522 - RFID - SPI - Modul GPIO5: 5 
#define SS_PIN  D2 // SDA-PIN for RC522 - RFID - SPI - Modul GPIO4: 4
#define LED_RFID D8 // LED RFID read 

MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance
void setup() {
  Serial.begin(115200); // Initialize serial communications with the PC
  SPI.begin();      // Init SPI bus
  mfrc522.PCD_Init();   // Init MFRC522
  Serial.println("Scan PICC to see UID and type...");
  pinMode(LED_RFID,OUTPUT);
}

void loop() {
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  Serial.println(F("The NUID tag is:"));
  Serial.print(F("In hex: "));
  printHex(mfrc522.uid.uidByte, mfrc522.uid.size);
  Serial.println();
  // Halt PICC
  mfrc522.PICC_HaltA();
  // Stop encryption on PCD
  mfrc522.PCD_StopCrypto1();
  digitalWrite(LED_RFID,HIGH);
  delay(500);
  digitalWrite(LED_RFID,LOW);
  delay(500);  
}

void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

