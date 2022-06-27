/*
#########################################################################################
#                                                                                       #
#  Kernal switcher sketch for C64 longboard (24-pin ROM)                                #
#  To be used with the Retroninja 2364 switchless multi-ROM                             #
#                                                                                       #
#  Version 1.3                                                                          #
#  https://github.com/retronynjah                                                       #
#                                                                                       #
#########################################################################################
*/

#include <EEPROM.h>

// searchString is the command that the kernalselector firmware sends to the ROM switcher.
// Searchstring below is RNROM64#
// The command will be followed by a single byte ROM number between 1 (0x01) and 15 (0x0f)
byte searchString[] = {0x52,0x4E,0x52,0x4F,0x4D,0x36,0x34,0x23}; // "RNROM64#"

// pin defitions
int resetPin = 11;
int restorePin = 12;
int clockPin = 13;
int ledPin = A0;

int savedROM;
int commandLength = sizeof(searchString);
int bytesCorrect = 0;
volatile bool clockstate;
bool restorestate;
bool restoreholding = false;
bool inmenu = false;
long restorepressed;

void pciSetup(byte pin)
{
    // enable Pin Change Interrupt for requested pin
    *digitalPinToPCMSK(pin) |= bit (digitalPinToPCMSKbit(pin));  // enable pin
    PCIFR |= bit (digitalPinToPCICRbit(pin)); // clear any outstanding interrupt
    PCICR |= bit (digitalPinToPCICRbit(pin)); // enable interrupt for the group
}


ISR (PCINT0_vect) // handle pin change interrupt for D8 to D13 here
{    
  // read state of clock pin (D13)
  clockstate = PINB & B100000;
}
 

void cleareeprom(){
  for (int i = 0 ; i < EEPROM.length() ; i++) {
    EEPROM.write(i, 0);
  }
}


void switchrom(int romnumber, bool doreset){

  if (doreset){
    // hold reset 
    digitalWrite(resetPin, LOW);
    delay(100);
    pinMode(resetPin, OUTPUT);
    digitalWrite(resetPin, LOW);
  }
 
  // switch eprompin A13 (D8)
  if (romnumber & B0001){
    digitalWrite(8, HIGH);
  }  
  else {
    digitalWrite(8, LOW);
  }

  // switch eprompin A14 (D9)
  if (romnumber & B000010){
    digitalWrite(9, HIGH);
  }  
  else {
    digitalWrite(9, LOW);
  }

  // switch eprompin A15 (D10)
  if (romnumber & B000100){
    digitalWrite(10, HIGH);
  }  
  else {
    digitalWrite(10, LOW);
  }

  // switch eprompin A16 (A1)
  if (romnumber & B001000){
    digitalWrite(A1, HIGH);
  }  
  else {
    digitalWrite(A1, LOW);
  }

  // switch eprompin A17 (A2)
  if (romnumber & B010000){
    digitalWrite(A2, HIGH);
  }  
  else {
    digitalWrite(A2, LOW);
  }

  // switch eprompin A18 (A3)
  if (romnumber & B100000){
    digitalWrite(A3, HIGH);
  }  
  else {
    digitalWrite(A3, LOW);
  }

  // if switching to a new kernal that isn't the menu kernal - save kernal to EEPROM address 0
  if ((romnumber != savedROM) && (romnumber != 0)){
    EEPROM.write(0, romnumber);
    savedROM = romnumber;
  }
  
  if (romnumber != 0){
    inmenu = false;
  }
  
  if (doreset){
    // Release reset
    delay(200);
    pinMode(resetPin, INPUT);
    // Give system some time to reset before entering loop.
    // Not needed if LED blink is enabled below
    //delay(500);
  }

  // blink LED ROM number of times
  for (int x = 1; x <= romnumber; x++){
    digitalWrite (ledPin, HIGH);
    delay(70);
    digitalWrite (ledPin, LOW);
    delay(250);
  }
}


void setup() {

  // set data pins 0..7 as inputs
  DDRD = B00000000;

  // Keep reset active (low) during setup
  digitalWrite(resetPin, LOW);
  pinMode(resetPin, OUTPUT);
  pinMode(clockPin, INPUT);
  
  pinMode(8, OUTPUT); // eprom A13
  pinMode(9, OUTPUT); // eprom A14
  pinMode(10, OUTPUT); // eprom A15
  pinMode (A1, OUTPUT); // eprom A16
  pinMode (A2, OUTPUT); // eprom A17
  pinMode (A3, OUTPUT); // eprom A18
  pinMode (ledPin, OUTPUT); // LED

  pinMode (restorePin, INPUT);
  digitalWrite(ledPin, LOW);
  
  // retrieve last used ROM from ATmega EEPROM and switch ROM using ROM address pins A13-A18
  savedROM = EEPROM.read(0);
  if (savedROM > 15){
    savedROM = 0;
  }
  switchrom(savedROM, false);
  
  // Release reset
  pinMode(resetPin, INPUT);
  delay(500); // Give system some time to finish reset before entering loop

  // Enable pin change interrupt on pin D13(PB5) connected to R/W (pin 38) on 6510/8500
  pciSetup(clockPin);
  
}


void loop() {
  // Read state of restore key
  restorestate = PINB & B010000;
  if (restorestate == LOW) {
    // restore pressed
    digitalWrite(ledPin, HIGH);
    if (!inmenu){
      if (restoreholding == false){
        restorepressed = millis();
        restoreholding = true;
      }
      if ((millis() - restorepressed) > 2000){
        // Switch to menu ROM and perform a reset
        inmenu = true;
        switchrom(0, true);
      }
    }
  }
  else{
    digitalWrite(ledPin, LOW);
    restoreholding = false;
  }
    
  if (clockstate == HIGH){
    byte byteCurr = PIND;
    clockstate=LOW;
    if (bytesCorrect == commandLength){
      // We have already found our command string. This byte must be the ROM number
      // Valid numbers are 1-15
      if ((byteCurr >= 1)&&(byteCurr<=15)){
        // ROM number within valid range. Switch ROM and perform a reset
        switchrom(byteCurr, true);
      }
      else{
        bytesCorrect=0;
      }
    }

    // We don't have full command string yet. Check if current byte is what we are looking for
    if (byteCurr == searchString[bytesCorrect]){
      // Increase bytesCorrect to check for next character
      bytesCorrect++;
    }
    else {
      bytesCorrect = 0;
    }
  }

  // Check if reset pin is active (low)
  if (!(PINB & B1000)){
    // Reset is active. Other switch might be active and doing a reset or user is resetting.
    // Switch ROM to give up menu in case we're in menu and don't perform another reset.
    switchrom(savedROM, false);
  }
}
