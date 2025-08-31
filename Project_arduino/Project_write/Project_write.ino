#include <SPI.h>
#include <SD.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 9
const int chipSelect = 8;
MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance
File dataFile;                      // File object to write data

void setup() {
  Serial.begin(9600);    // Initialize serial communication
  while (!Serial);       // Wait until serial port is opened

  SPI.begin();           // Initialize SPI bus
  mfrc522.PCD_Init();    // Initialize MFRC522 RFID reader

  if (!SD.begin(chipSelect)) { // Initialize SD card
    Serial.println("SD card initialization failed!");
    return;
  }
  Serial.println("SD card initialized successfully!");
}

void loop() {
  // Reset the RFID reader to look for new cards
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  // Look for new RFID cards
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    // Print UID of the card
    Serial.print("Scanned UID: ");
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
      Serial.print(mfrc522.uid.uidByte[i], HEX);
      if (i < mfrc522.uid.size - 1) Serial.print(" ");
    }
    Serial.println();

    // Create a new file on the SD card or open an existing one
    dataFile = SD.open("datalist.txt", FILE_WRITE);
    if (dataFile) {
      delay(1000); // Corrected delay function call
      // Write UID to the SD card
      for (byte i = 0; i < mfrc522.uid.size; i++) {
        dataFile.print(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
        dataFile.print(mfrc522.uid.uidByte[i], HEX);
        if (i < mfrc522.uid.size - 1) {
          dataFile.print(" ");
        }
      }
      dataFile.println(); // Add newline after writing UID
      Serial.println("Data written to SD card!");
      // Close the file
      dataFile.close();
    } else {
      Serial.println("Error opening file for writing!");
    }
  }

  delay(1000);  // Wait for 1 second before scanning for another card
}
