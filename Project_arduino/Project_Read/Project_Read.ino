 #include <SPI.h>
#include <SD.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 9
#define LED_PIN 5 // Pin connected to the LED
const int chipSelect = 4;
MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance
File dataFile;                      // File object to read data
String readValue = "";              // String to store read RFID value

void setup() {
  Serial.begin(9600);    // Initialize serial communication
  while (!Serial);       // Wait until serial port is opened

  SPI.begin();           // Initialize SPI bus
  mfrc522.PCD_Init();    // Initialize MFRC522 RFID reader

  pinMode(LED_PIN, OUTPUT); // Set LED pin as output

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
    // Read UID of the card
    readValue = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      readValue += (mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
      readValue += (String(mfrc522.uid.uidByte[i], HEX));
      if (i < mfrc522.uid.size - 1) readValue += " ";
    }
    Serial.println("Scanned UID: " + readValue);

    // Convert the read value to upper case
    readValue.toUpperCase();

    // Check if the value exists in the file
    dataFile = SD.open("datalod.txt", FILE_READ);
    if (dataFile) {
      bool found = false;
      String line;
      while (dataFile.available()) {
        line = dataFile.readStringUntil('\n');
        line.trim(); // Remove leading and trailing whitespace
        line.toUpperCase(); // Convert the line read from the file to upper case
        if (line == readValue) {
          found = true;
          break;
        }
      }
      dataFile.close();

      // If the value is found, turn on the LED
      if (found) {
        digitalWrite(LED_PIN, HIGH);
        delay(1000);
                digitalWrite(LED_PIN, LOW);

        
        Serial.println("Value found in file. LED turned on.");
      } else {
        digitalWrite(LED_PIN, LOW);
        Serial.println("Value not found in file. LED turned off.");
      }
    } else {
      Serial.println("Error opening file for reading!");
    }

    // Halt PICC and stop crypto
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
  }

  delay(1000);  // Wait for 1 second before scanning for another card
}
