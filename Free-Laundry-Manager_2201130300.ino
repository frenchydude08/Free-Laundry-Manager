/**
   ----------------------------------------------------------------------------
   ESP32 SAMPLE_Keypad
   ----------------------------------------------------------------------------

   ----------------------------------------------------------------------------
*/

#include <Preferences.h>       // Read write to FLASH
#include <MFRC522.h>           // RC522 RFID
#include <LiquidCrystal_I2C.h> // LCD
#include <Keypad_I2C.h>        // ADDED for keypad via I2C
#include <Keypad.h>            // keypad
#include <SPI.h>               // SPI
#include <Wire.h>              // I2C stuff
//#include <Servo.h>             // servo ctrl stuff
//#include <SoftwareSerial.h>    // used with SIM900


// ----------------------------------------------------------------------------
// Definition of macros
// ----------------------------------------------------------------------------

#define MAXPWD 4


// ----------------------------------------------------------------------------
// Definition of global constants
// ----------------------------------------------------------------------------

MFRC522 mfrc522(4, 5);              // MFRC522 mfrc522(SS_PIN, RST_PIN)
LiquidCrystal_I2C lcd(0x27, 20, 4); // Set the LCD address to 0x27 for a 16 chars and 2 line display
//SoftwareSerial SIM900(10, 11);    // SoftwareSerial SIM900(Rx, Tx)
//Servo myServo;                    // create servo object to control a servo

// Set Pins for LED, servo, buzzer and wipe button
constexpr uint8_t greenLed = 15;    // LED Green
constexpr uint8_t blueLed = 2;      // LED Blue
constexpr uint8_t redLed = 14;      // LED Red
constexpr uint8_t BuzzerPin = 17;   // Buzzer pin
constexpr uint8_t wipeB = 3;        // Button pin for WipeMode
//constexpr uint8_t ServoPin = 9;   // Servo pin

boolean NormalMode = true;          // boolean to change modes
boolean programMode = false;        // initialize programming mode to false
boolean RFIDMode = true;            // boolean to change modes

boolean match = false;              // initialize card match to false
boolean replaceMaster = false;      // initialize Master replace to false
uint8_t successRead;                // Variable to keep if we have Successful Read from Reader
uint8_t count;
uint8_t higherTag;                  // Variable to keep track of the number of tags saved in FLASH
uint8_t rights;                     /*
  Rights : regroupe plusieurs informations booléennes sur 8 bytes
  bytes order : 0b12345678
  bit 1: non utilisé
  bit 2: non utilisé
  bit 3: non utilisé
  bit 4: non utilisé
  bit 5: non utilisé
  bit 6: 1 = Tag erasable, 0 = Tag write protected
  bit 7: 1 = Tag enabled, 0 = Tag desabled
  bit 8: 1 = Admin Tag,  0 = Tag is not an Admin
*/

uint32_t uID;                       // Stores scanned ID read from RFID Module
uint32_t masterTag;                 // Stores master card ID read from FLASH
uint32_t storedTag;                 // Stores an ID read from FLASH
String password;                    // Variable to store password from KEYPAD
String masterPass;                  // Variable to store master password
String storedPass;                  // Variable to get password from FLASH
uint8_t storedRights;               // Variable to get Tag-rights from FLASH

String keyChoice = "";              // Variable to store incoming keys
uint8_t i = 0;                      // Variable used for counter
uint8_t b = 255;                    // Variable used for ref to tags
uint8_t tagok = 255;                // Variable used for selected tag
char nomParam[15];                  // Used to manipulate prior to set Preferences key

// defining how many rows and columns our keypad have
const byte rows = 4;
const byte columns = 4;
// Keypad pin map
char hexaKeys[rows][columns] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'F', '0', 'E', 'D'}
};
// Initializing pins for keypad
byte row_pins[rows] = {7, 6, 5, 4};
byte column_pins[columns] = {3, 2, 1, 0};

// Create instance for I2C keypad
int i2caddress = 0x20; // via PCF8574, pin A0 - A2 ke Ground.
Keypad_I2C newKey = Keypad_I2C( makeKeymap(hexaKeys), row_pins, column_pins, rows, columns, i2caddress );

Preferences sauvegardeStudios; //initiate the sauvegardeStudios instance


// ----------------------------------------------------------------------------
// DEBUG
// ----------------------------------------------------------------------------

///////////////////////////////////////// Display Debug infos   ///////////////////////////////////
void debugModes() {
  Serial.print("DEBUG-MODES_");
  Serial.print("NormalMode is: ");
  Serial.print(NormalMode);
  Serial.print(" / programMode is: ");
  Serial.print(programMode);
  Serial.print(" / RFIDMode is: ");
  Serial.println(RFIDMode);
}

void debugVariables(uint8_t b) {
  Serial.print("DEBUG-VARIA_");
  Serial.print("higherTag=");
  Serial.print(higherTag);
  Serial.print(" / b=");
  Serial.print(b);
  Serial.print(" / uID=");
  Serial.print(uID);
  Serial.print(" / password=");
  Serial.print(password);
  Serial.print(" / rights=");
  Serial.println(rights);
}

void debugMasterVariables() {
  Serial.print("DEBUG-MAVAR_");
  Serial.print("masterTag=");
  Serial.print(masterTag);
  Serial.print(" / masterPass=");
  Serial.println(masterPass);
}

void debugTagFlash(uint8_t b) {
  Serial.print("DEBUG-FLASH_");
  Serial.print("higherTag=");
  Serial.print(higherTag);
  Serial.print(" / b=");
  Serial.print(b);
  Serial.print(" / storedTag=");
  Serial.print(storedTag);
  Serial.print(" / storedPass=");
  Serial.print(storedPass);
  Serial.print(" / storedRights=");
  Serial.println(storedRights);
}


// ----------------------------------------------------------------------------
// LEDs
// ----------------------------------------------------------------------------

///////////////////////////////////////// Cycle Leds (Program Mode) ///////////////////////////////////
void cycleLeds() {
  digitalWrite(redLed, LOW);        // Make sure red LED is off
  digitalWrite(greenLed, HIGH);     // Make sure green LED is on
  digitalWrite(blueLed, LOW);       // Make sure blue LED is off
  delay(200);
  digitalWrite(redLed, LOW);        // Make sure red LED is off
  digitalWrite(greenLed, LOW);      // Make sure green LED is off
  digitalWrite(blueLed, HIGH);      // Make sure blue LED is on
  delay(200);
  digitalWrite(redLed, HIGH);       // Make sure red LED is on
  digitalWrite(greenLed, LOW);      // Make sure green LED is off
  digitalWrite(blueLed, LOW);       // Make sure blue LED is off
  delay(200);
  digitalWrite(redLed, LOW);        // Make sure red LED is on
  digitalWrite(greenLed, LOW);      // Make sure green LED is off
  digitalWrite(blueLed, HIGH);      // Make sure blue LED is off
}

///////////////////////////////////////// Blink LED's For Indication   ///////////////////////////////////
void BlinkLEDS(int led) {
  digitalWrite(blueLed, LOW);       // Make sure blue LED is off
  digitalWrite(redLed, LOW);        // Make sure red LED is off
  digitalWrite(greenLed, LOW);      // Make sure green LED is off
  digitalWrite(BuzzerPin, HIGH);
  delay(200);
  digitalWrite(led, HIGH);          // Make sure blue LED is on
  digitalWrite(BuzzerPin, LOW);
  delay(200);
  digitalWrite(led, LOW);           // Make sure blue LED is off
  digitalWrite(BuzzerPin, HIGH);
  delay(200);
  digitalWrite(led, HIGH);          // Make sure blue LED is on
  digitalWrite(BuzzerPin, LOW);
  delay(200);
  digitalWrite(led, LOW);           // Make sure blue LED is off
  digitalWrite(BuzzerPin, HIGH);
  delay(200);
  digitalWrite(led, HIGH);          // Make sure blue LED is on
  digitalWrite(BuzzerPin, LOW);
  delay(200);
}

//////////////////////////////////////// Normal Mode Led  ///////////////////////////////////
void normalModeOn () {
  digitalWrite(blueLed, HIGH);     // Blue LED ON and ready to read card
  digitalWrite(redLed, LOW);       // Make sure Red LED is off
  digitalWrite(greenLed, LOW);     // Make sure Green LED is off
}


// ----------------------------------------------------------------------------
// LCD
// ----------------------------------------------------------------------------

////////////////////// Print Info on LCD   ///////////////////////////////////
void ShowOnLCD() {
  if (RFIDMode == false) {
    lcd.clear();
    lcd.setCursor(0, 2);
    lcd.print("Enter Password      ");
    //lcd.setCursor(0, 3);
  }
  else if (RFIDMode == true) {
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("   Access Control   ");
    lcd.setCursor(0, 2);
    lcd.print("     Scan a Tag     ");
  }
}


// ----------------------------------------------------------------------------
// RELAYS handeling
// ----------------------------------------------------------------------------

/////////////////////////////////////////  Access Granted    ///////////////////////////////////
void granted () {
  digitalWrite(blueLed, LOW);       // Turn off blue LED
  digitalWrite(redLed, LOW);        // Turn off red LED
  digitalWrite(greenLed, HIGH);     // Turn on green LED
  if (RFIDMode == false) {
    //OUT myServo.write(90);
    //OUT delay(3000);
    //OUT myServo.write(10);
  }
  delay(1000);
  digitalWrite(blueLed, HIGH);
  digitalWrite(greenLed, LOW);
}

///////////////////////////////////////// Access Denied  ///////////////////////////////////
void denied() {
  digitalWrite(greenLed, LOW);     // Make sure green LED is off
  digitalWrite(blueLed, LOW);      // Make sure blue LED is off
  digitalWrite(redLed, HIGH);      // Turn on red LED
  digitalWrite(BuzzerPin, HIGH);
  delay(1000);
  digitalWrite(BuzzerPin, LOW);
  digitalWrite(blueLed, HIGH);
  digitalWrite(redLed, LOW);
}


// ----------------------------------------------------------------------------
// WIPE and reset to factory defaults
// ----------------------------------------------------------------------------

/////////////////// Counter to check if reset/wipe button is pressed or not   /////////////////////
bool monitorWipeButton(uint32_t interval) {
  unsigned long currentMillis = millis();                 // grab current time
  while (millis() - currentMillis < interval)  {
    int timeSpent = (millis() - currentMillis) / 1000;
    delay(1000);
    Serial.print(timeSpent);
    Serial.print(" ");
    lcd.setCursor(10, 1);
    lcd.print(timeSpent);
    // check on every half a second
    if (((uint32_t)millis() % 1000) == 0) {
      if (digitalRead(wipeB) != LOW) {
        return false;
      }
    }
  }
  return true;
}

///////////////////////////// Wipe Code   /////////////////////////////////
// If the Button (wipeB) Pressed while setup run (powered on) it wipes sauvegardeStudios in FLASH
void wipeAll() {
  if (digitalRead(wipeB) == LOW) {                // when button pressed pin should get low, button connected to ground
    digitalWrite(redLed, HIGH);                   // Red Led stays on to inform user we are going to wipe
    Serial.print("!! RESET BUTTON PRESSED !!");
    lcd.setCursor(0, 0);
    lcd.print(" Button Pressed     ");
    digitalWrite(BuzzerPin, HIGH);
    delay(1000);
    digitalWrite(BuzzerPin, LOW);
    lcd.clear();
    Serial.println(" THIS WILL REMOVE ALL RECORDS !!");
    lcd.setCursor(0, 0);
    lcd.print("This will remove");
    lcd.setCursor(0, 1);
    lcd.print("all records");
    delay(2000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("You have 10 ");
    lcd.setCursor(0, 1);
    lcd.print("secs to Cancel");
    delay(2000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Unpress to cancel   ");
    lcd.setCursor(0, 1);
    lcd.print("Counting: ");
    bool buttonState = monitorWipeButton(10000);            // Give user enough time to cancel operation
    if (buttonState == true && digitalRead(wipeB) == LOW) { // If button still be pressed, wipe FLASH
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Wiping FLASH...");
      sauvegardeStudios.clear();                            // Erase content of
      sauvegardeStudios.putUInt("higherTag", 0);            // Set the number of existing tags to zero
      sauvegardeStudios.end();                              // Close sauvegardeStudios
      delay(3000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Wiping Done");
      Serial.println("Wiping Done.");
      cycleLeds();                                          // visualize a successful wipe
      delay(3000);
    }
    else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Wiping Cancelled");        // Show some feedback that the wipe button did not pressed for 10 seconds
      Serial.println("Wiping Cancelled.");
      digitalWrite(redLed, LOW);
      delay(2000);
    }
  }
}


// ----------------------------------------------------------------------------
// KEYPAD actions
// ----------------------------------------------------------------------------

////////////////////// Get password from user keypad  ///////////////////////////////////
bool getKeyPWD() {
  Serial.println("Waiting for Password..");
  Serial.print("password Keyed: ");
  lcd.setCursor(0, 1);
  lcd.print("  Enter Password:   ");
  lcd.setCursor(0, 2);
  lcd.print("                    ");
  lcd.setCursor(0, 3);
  lcd.print("                    ");
  lcd.setCursor(8, 2);
  int k = 0;
  password = "";                                      // Clear password input value
  while (k < 4)
  {
    char key = newKey.getKey();
    if (key)
    {
      Serial.print(key);
      lcd.print("*");
      password += key;                                // append new character to input password string
      k++;
    }
  }
  Serial.print(", so password is: ");
  Serial.println(password);
  return 1;
}

////////////////////// Get CHOICE from user keypad  ///////////////////////////////////
bool getKeyChoice() {
  //lcd.setCursor(0, 2);
  //lcd.print("Select you choice:  ");
  lcd.setCursor(0, 3);
  lcd.print("Waiting for choice..");
  Serial.println("Waiting for user choice..");
  Serial.print("Keyed: ");
  int k = 0;
  keyChoice = "";
  while (k < 1)
  {
    char key = newKey.getKey();
    if (key)
    {
      Serial.println(key);
      //lcd.print(key);
      keyChoice += key;
      Serial.println(keyChoice);
      k++;
    }
  }
  return 1;
}

///////////////////////////////////////// Get VALUE from user keypad  ///////////////////////////////////
// Will have to write a fonction to acquire a value from the user


// ----------------------------------------------------------------------------
// RFID-RC522
// ----------------------------------------------------------------------------

/////////////////////// Check if RFID Reader is correctly initialized or not /////////////////////
void ShowReaderDetails() {
  // Get the MFRC522 software version
  byte v = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
  Serial.println("Initializing MFRC522..");
  Serial.print("MFRC522 version: ");
  Serial.println(v);
  // When 0x00 or 0xFF is returned, communication probably failed
  if ((v == 0x00) || (v == 0xFF)) {
    lcd.setCursor(0, 1);
    lcd.print("RC-522 Com. Failure ");
    lcd.setCursor(0, 2);
    lcd.print(" Check SPI RC-522   ");
    digitalWrite(BuzzerPin, HIGH);
    delay(2000);
    // Visualize system is halted
    digitalWrite(greenLed, LOW);  // Make sure green LED is off
    digitalWrite(blueLed, LOW);   // Make sure blue LED is off
    digitalWrite(redLed, HIGH);   // Turn on red LED
    digitalWrite(BuzzerPin, LOW);
    while (true); // do not go further
  }
}

///////////////////////////////////////// Get Tag's UID ///////////////////////////////////
int getID() {
  // Getting ready for Reading Tags
  if ( ! mfrc522.PICC_IsNewCardPresent()) { //If a new Tag placed to RFID reader continue
    return 0;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) {   //Since a Tag placed get Serial and continue
    return 0;
  }
  // There are Mifare Tags which have 4 byte or 7 byte UID care if you use 7 byte Tag
  // I think we should assume every Tag as they have 4 byte UID
  // Until we support 7 byte Tags
  uID = *(uint32_t *)mfrc522.uid.uidByte;
  Serial.print("Dec.: ");
  Serial.print(uID);
  Serial.print(" / Hex.: ");
  Serial.print(uID, HEX);
  mfrc522.PICC_HaltA(); // Stop reading
  Serial.println(" (Stopped reading).");
  return 1;
}


// ----------------------------------------------------------------------------
// Tags tests
// ----------------------------------------------------------------------------

////////////////////// Check uID IF is masterTag   ///////////////////////////////////
// Check to see if the ID passed is the master programing card
bool isMaster(uint32_t uID) {
  Serial.println();
  Serial.print(">>> DOING function isMaster.. with uID [");
  Serial.print(uID);
  Serial.println("]");
  b = 0;
  readID(b);
  if (uID == masterTag) {
    Serial.println("DONE, return true, IS MASTER. <<<");
    return true;
  }
  else
    Serial.println("DONE, return FALSE, TAG IS NOT MASTER. <<<");
  return false;
}

////////////////////// Check to see if password is matching   ///////////////////////////////////
// Check to see if the password is matching
bool matchPass( uint8_t b ) {
  Serial.println();
  Serial.print(">>> DOING function matchPass");
  RFIDMode = false;
  getKeyPWD();
  delay(200);
  debugModes();
  sprintf(nomParam, "Stu%dPWD", b);                      // ...
  storedPass = sauvegardeStudios.getString(nomParam);
  if ( storedPass == password)
  {
    Serial.println();
    Serial.println("!!! GOOD PASSWORD !!! End of matchPass <<<");
    i = 0;
    return 1;
  }
  else   // If password is not matched
  {
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("   Wrong Password   ");
    digitalWrite(BuzzerPin, HIGH);
    delay(1000);
    digitalWrite(BuzzerPin, LOW);
    Serial.println();
    Serial.println("!!! WRONG PASSWORD !!!  End of matchPass <<<");
    i = 0;
    return 0;
  }
}

////////////////////// Find ID in FLASH   ///////////////////////////////////
bool findID(uint32_t uID) {
  Serial.println();
  Serial.print(">>> DOING function findID.. with uID [");
  Serial.print(uID);
  Serial.println("]");
  count = higherTag;                                              // set count to the value of higherTag
  for ( uint8_t b = 1; b <= count; b++ ) {                        // Loop once for each FLASH entry
    Serial.println(b);
    readID(b);                                                    // Read an ID from FLASH
    if (uID == storedTag) {
      Serial.print("TRUE! Tag uID FOUND in FLASH for b = ");
      Serial.println(b);
      tagok = b;
      Serial.print("tagok = ");
      Serial.println(tagok);
      return true;
      break;                                                      // Stop looking we found it
    }
    else {                                                        // If not, return false
    }
  }
  Serial.println("FALSE! Tag uID NOT found in FLASH.");
  debugVariables(b);
  return false;
}

////////////////////// Find Slot of existing tag in FLASH  ///////////////////////////////////
uint8_t findIDSLOT(uint32_t uID) {
  Serial.println();
  Serial.println(">>> DOING function findIDSLOT..");
  count = higherTag;                                                      // set count to the value of higherTag
  for ( uint8_t b = 1; b <= count; b++ ) {                                // Loop once for each FLASH entry
    readID(b);                                                            // Read an ID from FLASH, it is stored in storedTag
    sprintf(nomParam, "Stu%dUID", b);
    if (*(uint32_t *)storedTag == sauvegardeStudios.getUInt(nomParam)) {  // Check to see if the storedTag read from FLASH
      // is the same as the find[] ID card passed
      return b;                                                           // The slot number of the card
    }
  }
}

////////////////////// Check Tag enable/disable state   ///////////////////////////////////
// Checks if Tag has been set to enable, or if it has been disabled.
bool chkTagState(uint8_t b) {
  Serial.println();
  Serial.println(">>> DOING function chkTagState..");
  sprintf(nomParam, "Stu%dEx", b);
  storedRights = sauvegardeStudios.getUInt(nomParam);
  debugTagFlash(b);
  debugVariables(b);
  return (bitRead(storedRights, 1));
}

////////////////////// Find an Empty Erasabled Slot   ///////////////////////////////////
// Find an empty erasable slot in Flash (to be used to write an new tag).
bool findEmpty() {
  Serial.println();
  Serial.println(">>> DOING function findEmpty..");
  readHigherTag();                                                // Read value of higherTag from FLASH
  count = higherTag;                                              // set count to the value of higherTag
  for ( b = 1; b <= count; b++ ) {                                // Loop once for each FLASH entry
    Serial.print(b);
    readID(b);                                                    // Read an ID from FLASH
    Serial.print("rights = "); Serial.println(rights);
    //if (*(uint8_t *)rights == 0b00000000 ) {
    if (1 - bitRead(rights, 2)) {
      Serial.print("EMPTY SLOT FOUND in FLASH for b = ");
      Serial.println(b);
      tagok = b;
      return true;
      break;                                                      // Stop looking we found an empty erasable slot
    }
    else {                                                        // If not, return false
    }
  }
  Serial.println("FALSE! NO EMPTY SLOT found in FLASH.");         // Did not find an empty erasable slot,
  return false;                                                   // return false.
}


// ----------------------------------------------------------------------------
// FLASH handeling
// ----------------------------------------------------------------------------

//////////////////////////////////////// Read higherTag from FLASH //////////////////////////////
void readHigherTag() {
  Serial.println();
  Serial.println(">>> DOING function readHigherTag..");
  higherTag = sauvegardeStudios.getUInt("higherTag");    // Get the value of higher tag from FLASH
  Serial.print("higherTag LOADED, is [");
  Serial.print(higherTag);
  Serial.println("]");
}

//////////////////////////////////////// Read ID from FLASH //////////////////////////////
void readID( uint8_t b ) {
  Serial.println();
  Serial.print(">>> DOING function readID.. for Tag [");
  Serial.print(b);
  Serial.println("]");
  if ( b == 0 ) {                                          // 0 is Master,
    masterTag = sauvegardeStudios.getUInt("Stu0UID");      // Read Master Tag uID
    masterPass = sauvegardeStudios.getString("Stu0PWD");   // Read Master Tag password
    debugTagFlash(b);
    debugVariables(b);
  }
  else {                                                   // Any other b value is a normal Tag user
    sprintf(nomParam, "Stu%dUID", b);                      // Prepare the read address
    storedTag = sauvegardeStudios.getUInt(nomParam);       // Assign values read from FLASH to array
    sprintf(nomParam, "Stu%dPWD", b);                      // ...
    storedPass = sauvegardeStudios.getString(nomParam);
    sprintf(nomParam, "Stu%dEx", b);
    rights = sauvegardeStudios.getUInt(nomParam);
    debugTagFlash(b);
    debugVariables(b);
  }
  Serial.println("readID COMPLETED <<<");
}

////////////////////// WRITE ID in FLASH   ///////////////////////////////////
bool writeTag ( uint8_t b, uint32_t uID, String password ) {
  Serial.println();
  Serial.println(">>> DOING function writeTag..");
  sprintf(nomParam, "Stu%dUID", b);
  sauvegardeStudios.putUInt(nomParam, uID);
  storedTag = sauvegardeStudios.getUInt(nomParam);
  debugVariables(b);
  Serial.print("Write tag [");
  Serial.print(b);
  Serial.print("] with uID [");
  Serial.print(uID);
  Serial.print("] to address [");
  Serial.print(nomParam);
  Serial.println("] in FLASH mem.");
  Serial.print("For verification, storedTag is: ");
  Serial.println(storedTag);
  Serial.println();
  if (storedTag != uID) {
    Serial.println("!! uID NOT SAVED in FLASH !!");
    return 0;
  }
  sprintf(nomParam, "Stu%dPWD", b);
  sauvegardeStudios.putString(nomParam, password);    //store the password of user b into sauvegardeStudios
  storedPass = sauvegardeStudios.getString(nomParam);
  debugVariables(b);
  Serial.print("Write tag [");
  Serial.print(b);
  Serial.print("] with password [");
  Serial.print(password);
  Serial.print("] to address [");
  Serial.print(nomParam);
  Serial.println("] in FLASH mem.");
  Serial.print("For verification, storedPass is: ");
  Serial.println(storedPass);
  if (storedPass != password) {
    Serial.println("!! password NOT SAVED in FLASH !!");
    return 0;
  }
  return 1;
}

////////////////////// Write Tag RIGHTS in FLASH   ///////////////////////////////////
bool writeRights( uint8_t b, uint8_t rights) {
  Serial.println();
  Serial.println(">>> DOING function writeRights..");
  sprintf(nomParam, "Stu%dEx", b);                          // Loads nomParam with proper Stu%dEx value based on the value of b
  sauvegardeStudios.putUInt(nomParam, rights);              // Store rights of user b into sauvegardeStudios
  storedRights = sauvegardeStudios.getUInt(nomParam);       // Load rights just saved for verification
  Serial.println();
  Serial.print(" / rights=");
  Serial.println(rights);
  Serial.print(" / storedRights=");
  Serial.println(storedRights);
  debugVariables(b);
  debugTagFlash(b);
}


// ----------------------------------------------------------------------------
// Tags handeling
// ----------------------------------------------------------------------------

///////////////////////////////////////// ADD NEW TAG and PASSWORD to FLASH   ///////////////////////////////////
bool addTag( uint8_t b, uint32_t uID ) {
  Serial.println();
  Serial.println(">>> DOING function addTag..");
  if ( !findEmpty() ) {                                    // Before we write to the FLASH, check to see if we have an erasable slot!
    readHigherTag();                                       // If no erasable slot, read value of higherTag from FLASH
    Serial.println();
    Serial.println("FINISHED doing higherTag");
    higherTag = higherTag + 1;                             // Add 1 to higherTag
    Serial.print("going to save new value of higherTag to FLASH");
    sauvegardeStudios.putUInt("higherTag", higherTag);     // Write the new value of higherTag to FLASH
    Serial.print("higherTag="); Serial.println(higherTag);
    b = higherTag;                                         // Set b to new higherTag
  }
  b = tagok;
  Serial.print("b="); Serial.println(b);
  getKeyPWD();                                             // Get password from user
  writeTag( b, uID, password );                            // Write tag ID and password to b address
  rights = 6;
  writeRights(b, rights);
  debugVariables(b);                                       // Display debug values
  lcd.setCursor(0, 2);
  lcd.print("     Tag Added      ");
  delay(1000);
  return 1;
}

///////////////////////////////////////// CRUSH TAG in FLASH   ///////////////////////////////////
bool deleteTag(uint8_t b) {
  Serial.println();
  Serial.println(">>> DOING function deleteTag..");
  if ( !findID(uID) ) {                                 // Before we delete from the FLASH, check to see if we have this card!
    BlinkLEDS(redLed);                                  // If not
    lcd.setCursor(0, 1);
    lcd.print(" Failed!            ");
    lcd.setCursor(0, 2);
    lcd.print("wrong ID or bad mem.");
    delay(2000);
    return 0;
  }
  else {                                              // If the Tag is known (and we are in programMode) then we are going to erase it
    Serial.print("value of b is: "); Serial.println(b);
    Serial.println("This will erase the Tag, press 'D' to procede with delete. Press any other key to cancel.");
    lcd.setCursor(0, 0);
    lcd.print(" Delete this Tag?   ");
    lcd.setCursor(0, 1);
    lcd.print("Press 'D' to delete,");
    lcd.setCursor(0, 2);
    lcd.print("Other key to cancel.");
    debugVariables(b);                                    // Display debug values
    while ( !getKeyChoice() );
    debugVariables(b);                                    // Display debug values
    if ( keyChoice == "D" ) {                             // If user confirm delete w/ 'D'
      rights = 0;                                         // Remove the tag by 'not write protected' + desabled + not admin (000), Prepared rights w required value (all set to 0)
      sprintf(nomParam, "Stu%dEx", b);                    // Loads nomParam with proper Stu%dEx value based on the value of b
      sauvegardeStudios.putUInt(nomParam, rights);        // Store rights of user b into sauvegardeStudios
      storedRights = sauvegardeStudios.getUInt(nomParam); // Load rights just saved for verification
      debugVariables(b);                                  // Display debug values
      Serial.print("value of b is: "); Serial.println(b);
      Serial.println("Do remove tag");
      Serial.print("saved rights for tag "); Serial.print(b); Serial.print(" is : "); Serial.println(storedRights);
      BlinkLEDS(blueLed);
      lcd.setCursor(0, 0);
      lcd.print("                    ");
      lcd.setCursor(0, 1);
      lcd.print("    Tag removed     ");
      lcd.setCursor(0, 2);
      lcd.print("                    ");
      lcd.setCursor(0, 3);
      lcd.print("                    ");
      delay(1500);
      return 1;
    }
    else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("                    ");
      lcd.setCursor(0, 1);
      lcd.print(" Delete cancelled.  ");
      lcd.setCursor(0, 2);
      lcd.print("                    ");
      lcd.setCursor(0, 3);
      lcd.print("                    ");
      delay(2000);
      return 0;
    }
  }
}


///////////////////////////////////////// Setup ///////////////////////////////////
//*********************************************************************************
void setup() {
  Serial.begin(115200);
  Serial.println("****************************************************************************");
  Serial.println("****************************************************************************");
  // Pin Configuration
  pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(blueLed, OUTPUT);
  pinMode(BuzzerPin, OUTPUT);
  pinMode(wipeB, INPUT_PULLUP);                               // Enable pin pull up resistor
  // Make sure leds are off
  digitalWrite(redLed, LOW);
  digitalWrite(greenLed, LOW);
  digitalWrite(blueLed, LOW);
  //Protocol Configuration
  lcd.begin();                                                // initialize the LCD
  lcd.backlight();
  SPI.begin();                                                // Initialize SPI protocol used by MFRC522 Hardware
  mfrc522.PCD_Init();                                         // Initialize RC522 Hardware
  ShowReaderDetails();                                        // Show details of RC522 Card Reader details
  Serial.println("MFRC522 initialized.");
  Serial.println();
  sauvegardeStudios.begin("my-app", false);                   // Opens storage space "my-app" for read and write
  newKey.begin();                                             // Initialize Keypad_I2C

  //myServo.attach(ServoPin);                                 // attaches the servo on pin 8 to the servo object
  //myServo.write(10);                                        // Initial Position
  // Arduino communicates with SIM900 GSM shield at a baud rate of 19200
  // Make sure that corresponds to the baud rate of your module
  //SIM900.begin(19200);
  // If you set Antenna Gain to Max it will increase reading distance
  //mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_max);

  wipeAll();                                                  // Check to see if Wipe-System is activated or not at boot time

  // Check if master card defined, if not let user choose a master card
  // This also useful to just redefine the Master Card
  // You can keep other FLASH records just write other than 143 to FLASH address Stu0Sta
  // FLASH address Stu0Sta should hold magical number which is '143'
  readHigherTag();
  debugVariables(b);
  Serial.println();
  b = 0;
  readID(b);
  debugMasterVariables();
  debugTagFlash(b);
  Serial.println();
  debugModes();
  Serial.println();
  Serial.println("Check if Master Tag defined..");            // Check if Master tag is defined
  if (sauvegardeStudios.getType("Stu0UID") == PT_INVALID) {   // If no value in Stu0UID, there is no Master defined
    Serial.println("NO MASTER FOUND !");
    Serial.println();
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("No Master Card ");
    lcd.setCursor(0, 2);
    lcd.print("Defined");
    delay(2000);
    lcd.setCursor(0, 1);
    lcd.print("   Scan a Tag to    ");
    lcd.setCursor(0, 2);
    lcd.print("  Define as Master  ");
    Serial.println("Waiting for scan of new Master tag..");   // Then we need to scan a new tag to define as Master
    do {
      successRead = getID();                                  // sets successRead to 1 when we get read from reader otherwise 0
      // Visualize Master Card need to be defined
      digitalWrite(blueLed, HIGH);
      digitalWrite(BuzzerPin, HIGH);
      delay(200);
      digitalWrite(BuzzerPin, LOW);
      digitalWrite(blueLed, LOW);
      delay(200);
    }
    while (!successRead);                                            // Program will not go further while not get a successful read
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("   Master Defined   ");
    Serial.println("Master defined. Enter password for Master! ");
    delay(2000);
    b = 0;
    getKeyPWD();                                                     // Get password from user
    sauvegardeStudios.begin("my-app", false);                        // Opens storage space "my-app" for read and write
    writeTag( b, uID, password );                                    // Write scanned Tag's uID and password to FLASH
    Serial.print("Master password defined.");
    Serial.println();
    //readHigherTag();                                                 // Loads the number of tags in FLASH for later use
    count = higherTag;
    debugVariables(b);
    NormalMode = false;
  }
  Serial.println();
  Serial.println("MASTER FOUND !");                                  // If Master already in FLASH
  b = 0;
  readID(b);
  debugMasterVariables();
  ShowOnLCD();                                                       // Print on the LCD
  cycleLeds();                                                       // Everything ready lets give user some feedback by cycling leds
  // AT command to set SIM900 to SMS mode
  //OUT SIM900.print("AT + CMGF = 1\r");
  delay(100);
  // Set module to send SMS data to serial out upon receipt
  //OUT SIM900.print("AT + CNMI = 2, 2, 0, 0, 0\r");
  readHigherTag();                                                   // Loads the number of tags in FLASH for later use
  sauvegardeStudios.end();                                           // Close sauvegardeStudios
  //programMode = true;
  delay(100);
  debugVariables(b);
  debugModes();
  NormalMode = true;
}


///////////////////////////////////////// Main Loop ///////////////////////////////////
//*************************************************************************************
void loop ()
{
  sauvegardeStudios.begin("my-app", false);                              // Opens storage space "my-app" for read and write
  // System will first check the mode
  if (NormalMode == false)                            //*** LOOK FOR MASTER and messages only, if NormalMode false
  {
    Serial.println("---- Now looking for MASTER tag and MESSAGES.. ----");
    // Function to receive message
    //OUT RxSmsMsg();
    // Function to get tag's UID
    getID();
    if ( isMaster(uID) )                                                 // If Scanned tag is master tag
    {
      NormalMode = true;                                                 // Go back to RFID mode
      RFIDMode = true;
      debugModes();
      cycleLeds();
    }
  }
  ////////// If NormalMode == true and RFIDMode == true
  else if (NormalMode == true && RFIDMode == true)                       //T200
  { //*** LOOK FOR ALL TAGS and messages if RFID Mode is true
    Serial.println();
    Serial.println("-------------------------------------------------");
    Serial.println("---- Now looking for ALL TAGS and MESSAGES.. ----");
    do
    {
      // Function to receive message
      //OUT RxSmsMsg();
      if (NormalMode == false)
      {
        break;
      }
      successRead = getID();                                             // sets successRead to 1 when we get read from reader otherwise 0
      if (programMode)
      {
        cycleLeds();                                                     // Program Mode cycles through Red Green Blue waiting to read a new card
      }
      else
      {
        normalModeOn();                                                  // Normal mode, blue Power LED is on, all others are off
      }
    }
    while ( !successRead );                                              //T240 the program will not go further while you are not getting a successful read
    //delay(500);
    Serial.println(">>> successRead received");
    debugModes();
    if (programMode && RFIDMode == true)                                 //T250
    {
      Serial.println("programMode && RFIDMode == true");
      if ( isMaster(uID) )                                               //T260
      { //*** When in program mode check First if master card scanned again to exit program mode
        Serial.println("Is Master: exiting Program Mode.");
        Serial.println();
        Serial.println("successRead received");
        debugModes();
        lcd.setCursor(0, 1);
        lcd.print("                    ");
        lcd.setCursor(0, 2);
        lcd.print("Leaving Program Mode");
        lcd.setCursor(0, 3);
        lcd.print("                    ");
        digitalWrite(BuzzerPin, HIGH);
        delay(1500);
        digitalWrite(BuzzerPin, LOW);
        ShowOnLCD();
        programMode = false;
        debugModes();
        return;
      }
      else
      {
        Serial.println("Is not Master: check if known tag,");
        Serial.println();
        Serial.println(b);
        Serial.println();
        debugVariables(b);
        if ( findID(uID) )                                        //T261
        { //*** If scanned card is known delete it
          Serial.println("Tag is known: deleting tag,");
          lcd.setCursor(0, 1);
          lcd.print(" Already there      ");
          b = tagok;
          deleteTag(b);
        }
        else                                                      //*** If scanned card is not known add it
        {
          Serial.println("Tag is new: adding new tag to FLASH,");
          lcd.setCursor(0, 1);
          lcd.print("New Tag, adding...  ");
          lcd.setCursor(0, 2);
          lcd.print("                    ");
          delay(2000);
          addTag( b, uID );
        }
        lcd.setCursor(0, 0);
        lcd.print("--- PROGRAM MODE ---");
        lcd.setCursor(0, 1);
        lcd.print("   Scan a Tag to    ");
        lcd.setCursor(0, 2);
        lcd.print("    ADD / REMOVE    ");
        lcd.setCursor(0, 3);
        lcd.print("  (Master to exit)  ");
      }
    }
    else
    {
      if ( isMaster(uID) )                                  //T270
      { //*** If scanned card's ID matches Master Card's ID
        //programMode = true;
        b = 0;
        lcd.setCursor(0, 0);
        lcd.print("     Master Tag     ");
        debugModes();
        Serial.println();
        if ( matchPass(b) )                                 //T280
        { //*** and if password is correct
          programMode = true;                               //*** enter program mode
        }
        else
        {
          programMode = false;
        }
        RFIDMode = true;
        ShowOnLCD();
        if (programMode == true)
        {
          Serial.print("*** NOW IN PROGRAM MODE *** (higherTag=");
          Serial.print(higherTag);
          Serial.println(")");
          lcd.clear();
          //lcd.home();
          lcd.setCursor(0, 0);
          lcd.print("--- PROGRAM MODE ---");
          lcd.setCursor(0, 2);
          lcd.print("   I have ");
          lcd.print(higherTag);
          lcd.print(" Tags");
          digitalWrite(BuzzerPin, HIGH);
          delay(2000);
          digitalWrite(BuzzerPin, LOW);
          lcd.setCursor(0, 1);
          lcd.print("   Scan a Tag to    ");
          lcd.setCursor(0, 2);
          lcd.print("    ADD / REMOVE    ");
          lcd.setCursor(0, 3);
          lcd.print("  (Master to exit)  ");
          Serial.print("Scan a tag to ADD / REMOVE. Scan MASTER to EXIT");
        }
      }
      else
      {
        if ( findID(uID) )                                            //T271
        { //*** If uID found in FLASH,
          Serial.println("Tag FOUND in FLASH, ");
          b = tagok;
          readID(b);                                                  // Load Tag values from FLASH
          if ( chkTagState(b))                                        //T272
          { //*** Check if Tag is enabled
            RFIDMode = false;                                         // If tag is a valid enabled tag, Make RFID mode false, and proceed with identification
            debugModes();
            ShowOnLCD();
            lcd.clear();
            lcd.setCursor(0, 1);
            lcd.print("      Good Tag      ");
            lcd.setCursor(0, 2);
            lcd.print("proceeding w/ passwd");
            delay(2000);
          }
          else                                                        //*** If tag is a disabled tag, ACCESS DENIED
          {
            Serial.println("DISABLED tag, cannot proceed.");
            lcd.clear();
            lcd.setCursor(0, 1);
            lcd.print("       DENIED       ");
            lcd.setCursor(0, 2);
            lcd.print("    DISABLED Tag    ");
            delay(2000);
            //OUT send_message("Someone Tried with a disabled tag");
            ShowOnLCD();
          }
        }
        else                                                          //*** If uID is not in FLASH, ACCESS DENIED
        {
          Serial.println("Tag NOT FOUND in FLASH, ACCESS DENIED !!");
          lcd.clear();
          lcd.setCursor(0, 1);
          lcd.print("       DENIED       ");
          lcd.setCursor(0, 2);
          lcd.print("     UNKNOWN Tag    ");
          delay(2000);
          denied();
          //OUT send_message("Someone Tried with the wrong tag");
          ShowOnLCD();
        }
      }
    }
  }
  ////////// If NormalMode && RFID mode are both false, jump straight to the end, system is in HALT mode.
  else if (NormalMode == true && RFIDMode == false)               //T300
  { //*** ASK FOR PASSWORD if NormalMode and no RFID
    getKeyPWD();
    if (storedPass == password)                                   //*** If PASSWORD MATCHED
    {
      Serial.println();
      Serial.println("!! GOOD PASSWORD !!");
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("   Pass Accepted    ");
      lcd.setCursor(0, 2);
      lcd.print(" loading wash-menu..");
      delay(1000);
      granted();
      //OUT send_message("Door Opened \n If it wasn't you, type 'close'");
    }
    else                                                          //*** If PASSWORD NOT MATCHED
    {
      Serial.println();
      Serial.println("WRONG PASSWORD");
      debugVariables(b);
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("   Wrong Password   ");
      delay(1000);
      denied();
      //OUT send_message("Someone Tried with the wrong tag");
    }
    RFIDMode = true;                                              // Make RFID mode true
    debugModes();
    ShowOnLCD();
  }
  i = 0;
  sauvegardeStudios.end();                                        // Close sauvegardeStudios
}


///////////////////////////// Receiving the message//////////////////////////////
/*void RxSmsMsg()
  {
  char incoming_char = 0; //Variable to save incoming SMS characters
  String incomingData;   // for storing incoming serial data
  if (SIM900.available() > 0)
  {
    incomingData = SIM900.readString(); // Get the incoming data.
    delay(10);
  }
  // if received command is to open the door
  if (incomingData.indexOf("open") >= 0)
  {
    myServo.write(90);
    NormalMode = true;
    RFIDMode == true;
    send_message("Opened");
    delay(10000);
    myServo.write(0);
  }
  // if received command is to halt the system
  if (incomingData.indexOf("close") >= 0)
  {
    NormalMode = false;
    RFIDMode == false;
    send_message("Closed");
  }
  incomingData = "";
  }
  //////////// Send the message////////////////////////
  void send_message(String message)
  {
  SIM900.println("AT+CMGF=1");    //Set the GSM Module in Text Mode
  delay(100);
  SIM900.println("AT+CMGS=\"+XXXXXXXXXXXX\""); // Replace it with your mobile number
  delay(100);
  SIM900.println(message);   // The SMS text you want to send
  delay(100);
  SIM900.println((char)26);  // ASCII code of CTRL+Z
  delay(100);
  SIM900.println();
  }
*/
