#include <SPI.h>
#include <SD.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 9
#define LED_T 4
#define buzzer 5
#define SoMOTOR1 6
#define SoMOTOR2 7
const int chipSelect = 8;
const String VIP_UID = "F3 B3 A6 2A"; 
MFRC522 mfrc522(SS_PIN, RST_PIN);
File dataFile;
String readValue = "";

void setup() {
  Serial.begin(9600);
  while (!Serial);

  SPI.begin();
  mfrc522.PCD_Init();

  pinMode(buzzer, OUTPUT);
  pinMode(LED_T, OUTPUT);
  pinMode(SoMOTOR1, OUTPUT);
  pinMode(SoMOTOR2, OUTPUT);
  digitalWrite(SoMOTOR1,  HIGH);  // Activate motor relay (assumes relay is LOW active)
  digitalWrite(SoMOTOR2,HIGH);  // Activate motor relay (assumes relay is LOW active)

  if (!SD.begin(chipSelect)) {
    Serial.println("SD card initialization failed!");
    return;
  }
  Serial.println("SD card initialized successfully!");
}

void loop() {
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    readUID();
    Serial.println("Scanned UID: " + readValue);

    bool isVIP = (VIP_UID == readValue);
    bool isFound = checkUIDInFile("datalog.txt");
    bool isBlacklisted = checkUIDInFile("datalist.txt");

    handleAccess(isVIP, isFound, isBlacklisted);

    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
  }

  delay(1000);
}

void readUID() {
  readValue = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    readValue += (mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
    readValue += String(mfrc522.uid.uidByte[i], HEX);
    if (i < mfrc522.uid.size - 1) readValue += " ";
  }
  readValue.toUpperCase();
}

bool checkUIDInFile(const String filename) {
  bool found = false;
  dataFile = SD.open(filename, FILE_READ);
  if (dataFile) {
    while (dataFile.available()) {
      String line = dataFile.readStringUntil('\n');
      line.trim();
      line.toUpperCase();
      if (line == readValue) {
        found = true;
        break;
      }
    }
    dataFile.close();
  } else {
    Serial.print("Error opening ");
    Serial.print(filename);
    Serial.println(" for reading!");
  }
  return found;
}

void handleAccess(bool isVIP, bool isFound, bool isBlacklisted) {
  if (isVIP) {
    Serial.println("Welcome to Team Sharks!");
      digitalWrite(LED_T, HIGH);
    digitalWrite(SoMOTOR1, LOW);  // Activate motor relay (assumes relay is LOW active)
    digitalWrite(SoMOTOR2, LOW);  // Activate motor relay (assumes relay is LOW active)
    delay(1500);
    digitalWrite(LED_T, LOW);
    digitalWrite(SoMOTOR1, HIGH);  // Deactivate motor relay
    digitalWrite(SoMOTOR2, HIGH);  // Deactivate motor relay
  } else if (isFound && isBlacklisted) {
    digitalWrite(buzzer, HIGH);
    delay(1000);
    digitalWrite(buzzer, LOW);
    Serial.println("Access denied (blacklist).");
  } else if (isFound && !isBlacklisted) {
    Serial.println("Access granted.");
    digitalWrite(LED_T, HIGH);
    digitalWrite(SoMOTOR2, LOW);  // Activate motor relay (assumes relay is LOW active)
    delay(1500);
    digitalWrite(LED_T, LOW);
    digitalWrite(SoMOTOR2, HIGH);  // Deactivate motor relay
  } 
  else {
    digitalWrite(buzzer, HIGH);
    delay(1000);
    digitalWrite(buzzer, LOW);
    Serial.println("Access denied (not found).");
  }
}
