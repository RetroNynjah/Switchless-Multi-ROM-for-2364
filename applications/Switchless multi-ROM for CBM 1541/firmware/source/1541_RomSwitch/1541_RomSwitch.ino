/*
#########################################################################################
#                                                                                       #
#  ROM switcher sketch for CBM 1541 (24-pin ROM)                                        #
#  To be used with the Retroninja 2364 switchless multi-ROM                             #
#                                                                                       #
#  Version 1.1                                                                          #
#  https://github.com/retronynjah                                                       #
#                                                                                       #
#########################################################################################
*/

#include <EEPROM.h>

// searchString is the command that is used for switching ROM.
// It is passed by the 1541 in reverse order so the variable should be reversed too.
// The command should be preceded by a ROM number between 1 and 4 when used.
// The below reversed searchString in hex ascii is MORNR@ which is specified like this on the C64: 1@RNROM, 2@RNROM and so on.
byte searchString[] = {0x4D,0x4F,0x52,0x4E,0x52,0x40};

// pin defitions
int resetPin = 11;
int clockPin = 13;
int ledPin = A0;

int commandLength = sizeof(searchString);
int bytesCorrect = 0;
volatile bool state;


void pciSetup(byte pin)
{
    // enable Pin Change Interrupt for requested pin
    *digitalPinToPCMSK(pin) |= bit (digitalPinToPCMSKbit(pin));  // enable pin
    PCIFR  |= bit (digitalPinToPCICRbit(pin)); // clear any outstanding interrupt
    PCICR  |= bit (digitalPinToPCICRbit(pin)); // enable interrupt for the group
}



ISR (PCINT0_vect) // handle pin change interrupt for D8 to D13 here
{    
       // read state of pin 13
       state = PINB & B100000;
}
 

void cleareeprom(){
  for (int i = 0 ; i < EEPROM.length() ; i++) {
    EEPROM.write(i, 0);
  }
}


void resetdrive(){
  pinMode(resetPin, OUTPUT); // reset pin
  digitalWrite(resetPin, LOW);
  delay(50);
  digitalWrite(resetPin, HIGH);
  delay(50);
  digitalWrite(resetPin, LOW);
  pinMode(resetPin, INPUT);

}


void switchrom(int romnumber){

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


  int savedROM = EEPROM.read(0);
  if (savedROM != romnumber){
    EEPROM.write(0, romnumber);
  }

  resetdrive();

  // blink LED to indicate selected image
  for (int x = 0; x <= romnumber; x++){
    digitalWrite (ledPin, HIGH);
    delay(30);
    digitalWrite (ledPin, LOW);
    delay(250);
  }
  delay (200);
}



// function for debugging using onboard LED
void blinknumber(int num) {
  int hundreds = num / 100;
  int tens = num % 100 / 10;
  int singular = num % 10;
  
  for (int x = 1; x <= 10; x++){
    digitalWrite(ledPin, HIGH);
    delay(50);
    digitalWrite(ledPin, LOW);
    delay(50);
  }
  delay(2000);

  for (int x = 1; x <= hundreds; x++){
    digitalWrite(ledPin, HIGH);
    delay(50);
    digitalWrite(ledPin, LOW);
    delay(350);
  }
  delay(2000);

  for (int x = 1; x <= tens; x++){
    digitalWrite(ledPin, HIGH);
    delay(50);
    digitalWrite(ledPin, LOW);
    delay(350);
  }
  delay(2000);

  for (int x = 1; x <= singular; x++){
    digitalWrite(ledPin, HIGH);
    delay(50);
    digitalWrite(ledPin, LOW);
    delay(350);
  }
}



void setup() {

  // set data pins as inputs
  DDRD = B00000000;
  
  pinMode(clockPin, INPUT); // clock pin
  pinMode(resetPin, INPUT); // reset pin. keep as input while not performing reset.
  
  pinMode(8, OUTPUT); // eprom A13
  pinMode(9, OUTPUT); // eprom A14
  pinMode(10, OUTPUT); // eprom A15
  pinMode(A1, OUTPUT); // eprom A16
  pinMode(A2, OUTPUT); // eprom A17
  pinMode(A3, OUTPUT); // eprom A18

  pinMode (ledPin, OUTPUT); //LED
    
  // retrieve last used ROM from ATmega EEPROM and switch ROM using ROM address pins A13-A16
  int lastROM = EEPROM.read(0);
  if (lastROM > 8){
    cleareeprom();
    lastROM = 0;
  }
  switchrom(lastROM);
  
  // enable pin change interrupt on pin D13(PB5) connected to R/W (pin 34) on 6502
  pciSetup(clockPin); 
}


void loop() {
  if (state == HIGH){
    state=LOW;
    byte byteCurr = PIND;
      
    if (bytesCorrect == commandLength){
      // we have our search string. This byte must be the ROM number
      // valid numbers are 1-8 (ASCII 49-56)
      if ((byteCurr >= 49)&&(byteCurr<=56)){
        // rom number within valid range. Switch rom
        switchrom(byteCurr - 49);
      }
      else{
        bytesCorrect=0;
      }
      //blinknumber(byteCurr);
    }

    // we don't have full search string yet. check if current byte is what we are looking for
    if (byteCurr == searchString[bytesCorrect]){
      // increase bytesCorrect to check for next character
      bytesCorrect++;
    }
    else {
      bytesCorrect = 0;
    }
  }
}
