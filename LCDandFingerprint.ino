/*
 * R307 Fingerprint Voting System with LCD
 * 
 * Wiring:
 * R307:
 *   VCC -> 3.3V, GND -> GND, TX -> Pin 2, RX -> Pin 3
 * 
 * LCD (16x2):
 *   RS -> Pin 13, E -> Pin 12
 *   D4-D7 -> Pins 11,10,9,8
 *   VSS,RW,K -> GND, VDD,A -> 5V, V0 -> GND
 * 
 * Buttons (one side to pin, other to GND):
 *   ENROLL     -> A0
 *   DELETE     -> A1
 *   VOTE       -> A2 (Verify finger then vote)
 *   UP         -> A3 (Navigate ID up)
 *   DOWN       -> A4 (Navigate ID down)
 *   RESULTS    -> A5 (Show results & winner)
 *   CANDIDATE1 -> Pin 4 (Vote for Candidate 1)
 *   CANDIDATE2 -> Pin 5 (Vote for Candidate 2)
 *   CANDIDATE3 -> Pin 6 (Vote for Candidate 3)
 *   CANDIDATE4 -> Pin 7 (Vote for Candidate 4)
 */

#include <SoftwareSerial.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>

LiquidCrystal lcd(13, 12, 11, 10, 9, 8);
SoftwareSerial fingerSerial(2, 3);

// Button pins
#define BTN_ENROLL      A0
#define BTN_DELETE      A1
#define BTN_VOTE        A2
#define BTN_UP          A3
#define BTN_DOWN        A4
#define BTN_RESULTS     A5
#define BTN_CANDIDATE1  4
#define BTN_CANDIDATE2  5
#define BTN_CANDIDATE3  6
#define BTN_CANDIDATE4  7

// Command codes
#define GET_IMAGE           0x01
#define IMAGE_TO_TZ         0x02
#define SEARCH              0x04
#define CREATE_MODEL        0x05
#define STORE_MODEL         0x06
#define DELETE_MODEL        0x0C
#define VERIFY_PASSWORD     0x13
#define GET_TEMPLATE_COUNT  0x1D

#define HEADER_H            0xEF
#define HEADER_L            0x01
#define PACKAGE_ID          0x01

// EEPROM addresses
#define EEPROM_VOTE1        0
#define EEPROM_VOTE2        1
#define EEPROM_VOTE3        2
#define EEPROM_VOTE4        3
#define EEPROM_VOTED_START  10
#define MAX_FINGERPRINTS    25

uint8_t packet[128];
int totalFingerprints = 0;
int vote1 = 0, vote2 = 0, vote3 = 0, vote4 = 0;

void setup() {
  Serial.begin(9600);
  
  pinMode(BTN_ENROLL, INPUT_PULLUP);
  pinMode(BTN_DELETE, INPUT_PULLUP);
  pinMode(BTN_VOTE, INPUT_PULLUP);
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_CANDIDATE1, INPUT_PULLUP);
  pinMode(BTN_CANDIDATE2, INPUT_PULLUP);
  pinMode(BTN_CANDIDATE3, INPUT_PULLUP);
  pinMode(BTN_CANDIDATE4, INPUT_PULLUP);
  pinMode(BTN_RESULTS, INPUT_PULLUP);
  
  lcd.begin(16, 2);
  delay(100);
  lcd.clear();
  delay(50);
  
  lcd.setCursor(0, 0);
  lcd.print("Fingerprint");
  lcd.setCursor(0, 1);
  lcd.print("Voting System");
  delay(2000);
  
  // Check for system reset (hold RESULTS at startup)
  lcd.clear();
  lcd.print("Hold RESULTS");
  lcd.setCursor(0, 1);
  lcd.print("to reset system");
  delay(1000);
  
  bool resetRequested = true;
  for (int i = 0; i < 20; i++) {
    if (digitalRead(BTN_RESULTS) == HIGH) {
      resetRequested = false;
      break;
    }
    delay(500);
  }
  
  if (resetRequested) {
    resetSystem();
  }
  
  // Load votes from EEPROM
  vote1 = EEPROM.read(EEPROM_VOTE1);
  vote2 = EEPROM.read(EEPROM_VOTE2);
  vote3 = EEPROM.read(EEPROM_VOTE3);
  vote4 = EEPROM.read(EEPROM_VOTE4);
  if (vote1 == 0xFF) vote1 = 0;
  if (vote2 == 0xFF) vote2 = 0;
  if (vote3 == 0xFF) vote3 = 0;
  if (vote4 == 0xFF) vote4 = 0;
  
  lcd.clear();
  lcd.print("Testing sensor");
  
  bool sensorOK = false;
  fingerSerial.begin(57600);
  delay(900);
  
  for (int i = 0; i < 6; i++) {
    lcd.setCursor(0, 1);
    lcd.print("Attempt ");
    lcd.print(i + 1);
    lcd.print("     ");
    if (verifyPassword()) {
      sensorOK = true;
      lcd.setCursor(0, 1);
      lcd.print("Sensor OK!      ");
      delay(1000);
      break;
    }
    delay(300);
  }
  
    
  if (sensorOK) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sensor OK!");
    Serial.println("\n✓ Sensor connected successfully!");
    delay(1000);
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sensor Error!");
    lcd.setCursor(0, 1);
    lcd.print("Check wiring");
    
    Serial.println("\n✗ SENSOR NOT RESPONDING!");
    Serial.println("\nTroubleshooting:");
    Serial.println("1. Swap TX/RX wires physically");
    Serial.println("2. Check 3.3V power (measure with multimeter)");
    Serial.println("3. Check if sensor LED is on");
    Serial.println("4. Try powering from external 3.3V source");
    Serial.println("5. Check for loose connections");
    
    while(1);
  }
  
  totalFingerprints = getTemplateCount();
  showMainMenu();
}

void loop() {
  if (digitalRead(BTN_ENROLL) == LOW) {
    delay(200);
    enrollFingerprint();
    showMainMenu();
    while(digitalRead(BTN_ENROLL) == LOW) delay(10);
  }
  
  if (digitalRead(BTN_DELETE) == LOW) {
    delay(200);
    deleteByID();
    showMainMenu();
    while(digitalRead(BTN_DELETE) == LOW) delay(10);
  }
  
  if (digitalRead(BTN_VOTE) == LOW) {
    delay(200);
    verifyAndVote();
    showMainMenu();
    while(digitalRead(BTN_VOTE) == LOW) delay(10);
  }
  
  if (digitalRead(BTN_RESULTS) == LOW) {
    delay(200);
    showResults();
    showMainMenu();
    while(digitalRead(BTN_RESULTS) == LOW) delay(10);
  }
  
  delay(50);
}

void showMainMenu() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enrolled: ");
  lcd.print(totalFingerprints);
  lcd.setCursor(0, 1);
  lcd.print("E-D-V-R Ready");
}

void resetSystem() {
  lcd.clear();
  lcd.print("System Reset");
  lcd.setCursor(0, 1);
  lcd.print("Please wait...");
  
  EEPROM.write(EEPROM_VOTE1, 0);
  EEPROM.write(EEPROM_VOTE2, 0);
  EEPROM.write(EEPROM_VOTE3, 0);
  EEPROM.write(EEPROM_VOTE4, 0);
  
  for (int i = 0; i < MAX_FINGERPRINTS; i++) {
    EEPROM.write(EEPROM_VOTED_START + i, 0xFF);
  }
  
  vote1 = 0;
  vote2 = 0;
  vote3 = 0;
  vote4 = 0;
  
  delay(2000);
  lcd.clear();
  lcd.print("Reset Complete");
  lcd.setCursor(0, 1);
  lcd.print("Restarting...");
  delay(2000);
}


//vote
void verifyAndVote() {
  lcd.clear();
  lcd.print("Place finger");
  
  while (fingerSerial.available()) fingerSerial.read();
  
  if (!waitForFinger(15)) {
    lcd.clear();
    lcd.print("Timeout!");
    delay(1500);
    return;
  }
  
  lcd.clear();
  lcd.print("Verifying...");
  delay(800);
  
  if (!imageToTemplate(1)) {
    lcd.clear();
    lcd.print("Scan error!");
    delay(1000);
    return;
  }
  
  lcd.clear();
  lcd.print("Searching...");
  
  while (fingerSerial.available()) fingerSerial.read();
  
  uint8_t searchData[] = {0x01, 0x00, 0x00, 0x00, 0x7F};
  sendCommand(SEARCH, searchData, 5);
  delay(1000);
  
  uint8_t response[32];
  int len = readResponse(response, 3000);
  
  if (len >= 16 && response[9] == 0x00) {
    int matchID = (response[10] << 8) | response[11];
    int confidence = (response[12] << 8) | response[13];
    
    // Check if already voted
    bool alreadyVoted = true;
    for (int i = 0; i < MAX_FINGERPRINTS; i++) {
      if (EEPROM.read(EEPROM_VOTED_START + i) == matchID) {
        alreadyVoted = false;
        break;
      }
    }
    
    if (alreadyVoted) {
      lcd.clear();
      lcd.print("Already Voted!");
      lcd.setCursor(0, 1);
      lcd.print("ID: ");
      lcd.print(matchID);
      delay(3000);
      return;
    }
    
    lcd.clear();
    lcd.print("Authorized!");
    lcd.setCursor(0, 1);
    lcd.print("ID:");
    lcd.print(matchID);
    lcd.print(" C:");
    lcd.print(confidence);
    delay(2000);
    
    castVote(matchID);
    
  } else {
    lcd.clear();
    lcd.print("Not Authorized");
    lcd.setCursor(0, 1);
    lcd.print("Enroll first!");
    delay(2000);
  }
}

//cast vote
void castVote(int voterID) {
  lcd.clear();
  lcd.print("Vote:");
  lcd.setCursor(0, 1);
  lcd.print("C1 C2 C3 C4");
  
  unsigned long startTime = millis();
  while (millis() - startTime < 15000) {
    
    if (digitalRead(BTN_CANDIDATE1) == LOW) {
      vote1++;
      EEPROM.write(EEPROM_VOTE1, vote1);
      markAsVoted(voterID);
      
      lcd.clear();
      lcd.print("Vote Recorded!");
      lcd.setCursor(0, 1);
      return;
    }
    
    if (digitalRead(BTN_CANDIDATE2) == LOW) {
      vote2++;
      EEPROM.write(EEPROM_VOTE2, vote2);
      markAsVoted(voterID);
      
      lcd.clear();
      lcd.print("Vote Recorded!");
      lcd.setCursor(0, 1);
      return;
    }
    
    if (digitalRead(BTN_CANDIDATE3) == LOW) {
      vote3++;
      EEPROM.write(EEPROM_VOTE3, vote3);
      markAsVoted(voterID);
      
      lcd.clear();
      lcd.print("Vote Recorded!");
      lcd.setCursor(0, 1);
      return;
    }
    
    if (digitalRead(BTN_CANDIDATE4) == LOW) {
      vote4++;
      EEPROM.write(EEPROM_VOTE4, vote4);
      markAsVoted(voterID);
      
      lcd.clear();
      lcd.print("Vote Recorded!");
      lcd.setCursor(0, 1);
      return;
    }
    
    delay(50);
  }
  
  lcd.clear();
  delay(2000);
}

void markAsVoted(int id) {
  // Mark voter as having voted by removing them from the "not voted" list
  for (int i = 0; i < MAX_FINGERPRINTS; i++) {
    if (EEPROM.read(EEPROM_VOTED_START + i) == id) {
      EEPROM.write(EEPROM_VOTED_START + i, 0xFF);
      Serial.print("Marked ID ");
      Serial.print(id);
      Serial.println(" as voted");
      break;
    }
  }
}
//***results button***
void showResults() {
  lcd.clear();
  lcd.print("Results:");
  delay(2000);
  
  // Show each candidate
  lcd.clear();
  lcd.print("Candidate 1:");
  lcd.setCursor(0, 1);
  lcd.print(vote1);
  lcd.print(" votes");
  delay(3000);
  
  lcd.clear();
  lcd.print("Candidate 2:");
  lcd.setCursor(0, 1);
  lcd.print(vote2);
  lcd.print(" votes");
  delay(3000);
  
  lcd.clear();
  lcd.print("Candidate 3:");
  lcd.setCursor(0, 1);
  lcd.print(vote3);
  lcd.print(" votes");
  delay(3000);
  
  lcd.clear();
  lcd.print("Candidate 4:");
  lcd.setCursor(0, 1);
  lcd.print(vote4);
  lcd.print(" votes");
  delay(3000);
  
  // Show summary
  int totalVotes = vote1 + vote2 + vote3 + vote4;
  
  if (totalVotes == 0) {
    lcd.clear();
    lcd.print("No votes yet!");
    delay(2000);
    return;
  }
  
  lcd.clear();
  lcd.print("Total: ");
  lcd.print(totalVotes);
  lcd.print("/");
  lcd.print(totalFingerprints);
  delay(3000);
  
  // Determine winner
  lcd.clear();
  int maxVotes = max(max(vote1, vote2), max(vote3, vote4));
  int winners = 0;
  if (vote1 == maxVotes) winners++;
  if (vote2 == maxVotes) winners++;
  if (vote3 == maxVotes) winners++;
  if (vote4 == maxVotes) winners++;
  
  if (winners > 1) {
    lcd.print("Tie!");
    lcd.setCursor(0, 1);
    lcd.print("No clear winner");
  } else {
    lcd.print("Winner:");
    lcd.setCursor(0, 1);
    if (vote1 == maxVotes) lcd.print("Candidate 1");
    else if (vote2 == maxVotes) lcd.print("Candidate 2");
    else if (vote3 == maxVotes) lcd.print("Candidate 3");
    else if (vote4 == maxVotes) lcd.print("Candidate 4");
  }
  delay(4000);
}
//***Enroll FingerPrint***
void enrollFingerprint() {
  int id = 1;
  
  lcd.clear();
  lcd.print("Select ID:");
  
  while (true) {
    lcd.setCursor(0, 1);
    lcd.print("ID: ");
    lcd.print(id);
    lcd.print("    ");
    //UP button
    if (digitalRead(BTN_UP) == LOW) {
      id++;
      if (id > MAX_FINGERPRINTS) id = 1;
      delay(200);
    }
    //DOWN button

    if (digitalRead(BTN_DOWN) == LOW) {
      id--;
      if (id < 1) id = MAX_FINGERPRINTS;
      delay(200);
    
    }
    
    if (digitalRead(BTN_ENROLL) == LOW) {
      delay(200);
      break;
    }
    
    if (digitalRead(BTN_DELETE) == LOW) {
      return;
    }
  }
  
  lcd.clear();
  lcd.print("Enrolling ID:");
  lcd.print(id);
  lcd.setCursor(0, 1);
  lcd.print("Place finger");
  delay(1000);
  
  int attempts = 0;
  bool success = false;
  
  while (attempts < 3 && !success) {
    if (waitForFinger(10)) {
      lcd.clear();
      lcd.print("Hold steady...");
      delay(800);
      
      if (imageToTemplate(1)) {
        success = true;
      } else {
        attempts++;
        lcd.clear();
        lcd.print("Try ");
        lcd.print(attempts + 1);
        lcd.print(" of 3");
        delay(1200);
        
        while (checkFinger()) delay(100);
        delay(300);
        
        if (attempts < 3) {
          lcd.clear();
          lcd.print("Place finger");
          delay(800);
        }
      }
    } else {
      lcd.clear();
      lcd.print("Timeout!");
      delay(1500);
      return;
    }
  }
  
  
  lcd.clear();
  lcd.print("Image OK!");
  lcd.setCursor(0, 1);
  lcd.print("Remove finger");
  delay(1500);

  //???
  int removeAttempts = 0;
  while (checkFinger() && removeAttempts < 50) {
    delay(200);
    removeAttempts++;
  }
  
  if (removeAttempts >= 50) {
    lcd.clear();
    lcd.print("Remove finger!");
    delay(1500);
    return;
  }
  
  delay(1500);
  
  while (fingerSerial.available()) fingerSerial.read();
  
  lcd.clear();
  lcd.print("Place SAME");
  lcd.setCursor(0, 1);
  lcd.print("finger again");
  delay(1500);
  
  attempts = 0;
  success = false;
  
  while (attempts < 3 && !success) {
    if (waitForFinger(15)) {
      lcd.clear();
      lcd.print("Hold steady...");
      delay(1000);
      
      if (imageToTemplate(2)) {
        success = true;
      } else {
        attempts++;
        lcd.clear();
        lcd.print("Try ");
        lcd.print(attempts + 1);
        lcd.print(" of 3");
        delay(1500);
        
        while (checkFinger()) delay(100);
        delay(800);
        
        while (fingerSerial.available()) fingerSerial.read();
        
        if (attempts < 3) {
          lcd.clear();
          lcd.print("Place SAME");
          lcd.setCursor(0, 1);
          lcd.print("finger again");
          delay(1000);
        }
      }
    } else {
      lcd.clear();
      lcd.print("Timeout!");
      delay(1000);
      return;
    }
  }
  
  if (!success) {
    lcd.clear();
    lcd.print("Failed!");
    delay(800);
    return;
  }
  
  lcd.clear();
  lcd.print("Processing...");
  
  if (!createModel()) {
    lcd.clear();
    lcd.print("Mismatch!");
    delay(1500);
    return;
  }
  
  lcd.clear();
  lcd.print("Storing...");
  
  if (!storeModel(id)) {
    lcd.clear();
    lcd.print("Store failed!");
    delay(1500);
    return;
  }
  
  for (int i = 0; i < MAX_FINGERPRINTS; i++) {
    if (EEPROM.read(EEPROM_VOTED_START + i) == 0xFF) {
      EEPROM.write(EEPROM_VOTED_START + i, id);
      break;
    }
  }
  
  lcd.clear();
  lcd.print("SUCCESS!");
  lcd.setCursor(0, 1);
  lcd.print("ID: ");
  lcd.print(id);
  delay(1000);
  
  totalFingerprints = getTemplateCount();
}
//***Delete Function***
void deleteByID() {
  int id = 1;
  
  lcd.clear();
  lcd.print("Delete ID:");
  
  while (true) {
    lcd.setCursor(0, 1);
    lcd.print("ID: ");
    lcd.print(id);
    lcd.print("    ");
    
    if (digitalRead(BTN_UP) == LOW) {
      id++;
      if (id > MAX_FINGERPRINTS) id = 1;
      delay(200);
    }
    
    if (digitalRead(BTN_DOWN) == LOW) {
      id--;
      if (id < 1) id = MAX_FINGERPRINTS;
      delay(200);
    }
    
    if (digitalRead(BTN_DELETE) == LOW) {
      delay(200);
      break;
    }
    
    if (digitalRead(BTN_ENROLL) == LOW) {
      return;
    }
  }
  
  lcd.clear();
  lcd.print("Deleting ID:");
  lcd.print(id);
  delay(1000);
  
  uint8_t deleteData[] = {(uint8_t)(id >> 8), (uint8_t)(id & 0xFF), 0x00, 0x01};
  sendCommand(DELETE_MODEL, deleteData, 4);
  delay(200);
  
  uint8_t response[32];
  int len = readResponse(response, 1000);
  
  if (len >= 12 && response[9] == 0x00) {
    for (int i = 0; i < MAX_FINGERPRINTS; i++) {
      if (EEPROM.read(EEPROM_VOTED_START + i) == id) {
        EEPROM.write(EEPROM_VOTED_START + i, 0xFF);
        break;
      }
    }
    
    lcd.clear();
    lcd.print("Deleted!");
    lcd.setCursor(0, 1);
    lcd.print("ID: ");
    lcd.print(id);
    delay(2000);
  } 
  
  totalFingerprints = getTemplateCount();
}

bool waitForFinger(int timeoutSeconds) {
  int timeout = timeoutSeconds * 10;
  
  while (timeout > 0) {
    while (fingerSerial.available()) fingerSerial.read();
    
    sendCommand(GET_IMAGE, NULL, 0);
    delay(200);
    
    uint8_t response[32];
    int len = readResponse(response, 1000);
    
    if (len >= 12) {
      if (response[9] == 0x00) {
        return true;
      } else if (response[9] == 0x02) {
        timeout--;
        continue;
      }
    }
    timeout--;
  }
  return false;
}

bool checkFinger() {
  while (fingerSerial.available()) fingerSerial.read();
  
  sendCommand(GET_IMAGE, NULL, 0);
  delay(100);
  
  uint8_t response[32];
  int len = readResponse(response, 500);
  
  return (len >= 12 && response[9] == 0x00);
}

bool imageToTemplate(uint8_t bufferID) {
  uint8_t data[] = {bufferID};
  
  while (fingerSerial.available()) fingerSerial.read();
  
  sendCommand(IMAGE_TO_TZ, data, 1);
  delay(1100);
  
  uint8_t response[64];
  int len = readResponse(response, 5000);
  
  if (len >= 12) {
    uint8_t confirmCode = response[9];
    
    if (confirmCode != 0x00) {
      lcd.clear();
      
      switch(confirmCode) {
        case 0x06:
          lcd.print("Too messy!");
          lcd.setCursor(0, 1);
          lcd.print("Clean sensor");
          break;
        case 0x07:
          lcd.print("Few features");
          lcd.setCursor(0, 1);
          lcd.print("Press harder");
          break;
        default:
          lcd.print("Error: 0x");
          lcd.print(confirmCode, HEX);
          break;
      }
      delay(2500);
    }
    
    return (confirmCode == 0x00);
  }
  
  return false;
}

bool createModel() {
  sendCommand(CREATE_MODEL, NULL, 0);
  delay(200);
  
  uint8_t response[32];
  int len = readResponse(response, 1000);
  
  return (len >= 12 && response[9] == 0x00);
}

bool storeModel(int id) {
  uint8_t data[] = {0x01, (uint8_t)(id >> 8), (uint8_t)(id & 0xFF)};
  sendCommand(STORE_MODEL, data, 3);
  delay(200);
  
  uint8_t response[32];
  int len = readResponse(response, 1000);
  
  return (len >= 12 && response[9] == 0x00);
}

bool verifyPassword() {
  uint8_t data[] = {0x00, 0x00, 0x00, 0x00};
  sendCommand(VERIFY_PASSWORD, data, 4);
  delay(500);
  
  uint8_t response[32];
  int len = readResponse(response, 2000);
  
  lcd.setCursor(0, 1);
  lcd.print(len);
  lcd.print(" bytes  ");
  delay(300);
  
  return (len >= 12 && response[9] == 0x00);
}

int getTemplateCount() {
  sendCommand(GET_TEMPLATE_COUNT, NULL, 0);
  delay(100);
  
  uint8_t response[32];
  int len = readResponse(response, 1000);
  
  if (len >= 14 && response[9] == 0x00) {
    return (response[10] << 8) | response[11];
  }
  return 0;
}

void sendCommand(uint8_t cmd, uint8_t *data, int dataLen) {
  while (fingerSerial.available()) fingerSerial.read();
  
  int idx = 0;
  packet[idx++] = HEADER_H;
  packet[idx++] = HEADER_L;
  packet[idx++] = 0xFF;
  packet[idx++] = 0xFF;
  packet[idx++] = 0xFF;
  packet[idx++] = 0xFF;
  packet[idx++] = PACKAGE_ID;
  
  int length = dataLen + 3;
  packet[idx++] = (length >> 8) & 0xFF;
  packet[idx++] = length & 0xFF;
  packet[idx++] = cmd;
  
  for (int i = 0; i < dataLen; i++) {
    packet[idx++] = data[i];
  }
  
  uint16_t checksum = PACKAGE_ID + ((length >> 8) & 0xFF) + (length & 0xFF) + cmd;
  for (int i = 0; i < dataLen; i++) {
    checksum += data[i];
  }
  
  packet[idx++] = (checksum >> 8) & 0xFF;
  packet[idx++] = checksum & 0xFF;
  
  fingerSerial.write(packet, idx);
}

int readResponse(uint8_t *buffer, int timeout) {
  unsigned long startTime = millis();
  int idx = 0;
  
  while (millis() - startTime < timeout) {
    if (fingerSerial.available()) {
      buffer[idx++] = fingerSerial.read();
      if (idx >= 32) break;
    }
  }
  return idx;
}