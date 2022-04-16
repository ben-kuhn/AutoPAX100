/*
 AutoPAX100

This sketch reads the analog value from the Band pin on the ACC connector of the Xiegu G90
transceiver and automatically switches the LPFX7 LPF to the correct band.  The sketch should
also automatically disable and bypass the PAX100 amplifier and LPF when an unsupported band
is selected.

A 16 column 2 row display shows the status of the amp

 The circuit:
  - Power is provided to the 5v pin via an LM7805 regulator with filtering capacitors
  - G90 Aux port Pin 2 (Red) connected to analog pin 0.
  - G90 Aux port Pin 7 (Violet) connected to ground
  - Bypass SPST Power Relay connected to Digital Pin 2
    - Provides 12v to the amplifier
    - A SPDT relay connects to the output of Digital Pin 2.
      The NC side of the relay routes the RF signal around the amp and filter directly to the output connector
      The NO side of the signal routes the RF signal in to the PAX100 and LPFX7
  - 10-15M LPF select switch connectied via solid-state relay to digital pin 3
  - 17-20M LPF select switch connectied via solid-state relay to digital pin 4
  - 40M LPF select switch connectied via solid-state relay to digital pin 5
  - 80M LPF select switch connectied via solid-state relay to digital pin 6
  - Output from the 10M enable LED is connected via level shifter to analog pin 0
  - Output from the 20M enable LED is connected via level shifter to analog pin 1
  - Output from the 40M enable LED is connected via level shifter to analog pin 2
  - Output from the 80M enable LED is connected via level shifter to analog pin 3
  - TX/RX signal connected via level shifter to analog pin 3
  - LCD RS pin to digital pin 7
  - LCD Enable pin to digital pin 8
  - LCD D4 pin to digital pin 9
  - LCD D5 pin to digital pin 10
  - LCD D6 pin to digital pin 11
  - LCD D7 pin to digital pin 12
  - LCD R/W pin to ground
  - 10K trimpot:
    - ends to +5V and ground
    - wiper to LCD VO pin (pin 3)

  created 29 Jan. 2021
  by Ben Kuhn, KU0HN

*/
// include the library code:
#include <LiquidCrystal.h>

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 7, en = 8, d4 = 9, d5 = 10, d6 = 11, d7 = 12;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Define the analog pin used to read the band pin from the radio
const int bandPin = A7;

// Define digital pins used to activate the 4 available filters
const int activate10 = 3;
const int activate20 = 4;
const int activate40 = 5;
const int activate80 = 6;

// Define pins used to read current filter activation status
const int read10 = A0;
const int read20 = A1;
const int read40 = A2;
const int read80 = A3;

// Define pin used for the power bypass relay and set it for output
const int ampRelay = 2;

// Define the pin used to detect +8v from the radio
const int radioOn = A4;

// Define pin used to detect the TX signal
const int txEnabled = 13;

// Variable to store the band data
int bandPinValue = 0;

// Variables to store the current LPF state
bool status10 = LOW;
bool status20 = LOW;
bool status40 = LOW;
bool status80 = LOW;

// Variable to store the Bypass Relay state
bool ampEnabled = LOW;

// Variable to store the current LPF setting
int filterState = 0;

void setup() {
  // Uncomment for serial debugging
  //Serial.begin(9600);

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);

  // Set LPF select pins to output
  pinMode(activate10, OUTPUT);
  pinMode(activate20, OUTPUT);
  pinMode(activate40, OUTPUT);
  pinMode(activate80, OUTPUT);

  // Set LPF status pins to input
  pinMode(read10, INPUT);
  pinMode(read20, INPUT);
  pinMode(read40, INPUT);
  pinMode(read80, INPUT);

  // Set the amp bypass relay control to output
  pinMode(ampRelay, OUTPUT);

  // Set the TX signal detect pin to input
  pinMode(txEnabled, INPUT);

  // Bypass the amp while we wait for things to settle down.
  bypassAmp();

  
  
}

int getRadioBand() {

  // Read band information from the radio interface
  bandPinValue = analogRead(bandPin);

  // Convert value from ADC to a meaningful band
  // See Values.txt for ADC values from testing.  Giving a bit (10) each direction in case a low or high
  // battery voltage condition or something causes voltage to sag or be a bit high.
    if ((bandPinValue > 40) && (bandPinValue < 60)) {
      return 160;
    }
    else if ((bandPinValue > 90) && (bandPinValue < 110)) {
      return 80;
    }
    else if ((bandPinValue > 140) && (bandPinValue < 160)) {
      return 60;
    }
    else if ((bandPinValue > 190) && (bandPinValue < 210)) {
      return 40;
    }
    else if ((bandPinValue > 240) && (bandPinValue < 260)) {
      return 30;
    }
    else if ((bandPinValue > 290) && (bandPinValue < 310)) {
      return 20;
    }
    else if ((bandPinValue > 340) && (bandPinValue < 360)) {
      return 17; 
    }
    else if ((bandPinValue > 390) && (bandPinValue < 410)) {
      return 15;
    }
    else if ((bandPinValue > 440) && (bandPinValue < 515)) {
      //There is a voltage split in the middle of 12 meters (annoyingly) so we are combining 10 and 12
      return 10;
    }
    else {
      //return 0 for error if nothing else matches
      return 0;
    }
}

int isFilterActive() {

  // Read the output from the LPF to see what filter, if any, are active
  if (digitalRead(read10) == HIGH) {
    return 10;
  }
  else if (digitalRead(read20) == HIGH) {
    return 20;
  }
  else if (digitalRead(read40) == HIGH) {
    return 40;
  }
  else if (digitalRead(read80) == HIGH) {
    return 80;
  }
  // Return 0 if the filter is off
  else {
    return 0;
  }
}

void bypassAmp() {

  // Cut power to the amp and LPF, and bypass radio output direct to the antenna
  digitalWrite(ampRelay, LOW);
  // Set the LPF state to 0 so we know to toggle it next time the band changes.
  filterState = 0;
}

void enableAmp() {

  // Cut power to the amp and LPF, and bypass radio output direct to the antenna
  digitalWrite(ampRelay, HIGH);
}

void setBand(int bandSet) {

  // Activate the appropriate LPF for the given band
  if (bandSet == 80) {
    if (filterState != 80) {
      digitalWrite(activate80, HIGH);
      //We just need a momentary pulse to latch the relay so wait 10ms and go low again
      delay(10);
      digitalWrite(activate80, LOW);
      filterState = 80;
    } 
  }
  else if (bandSet == 40) {
    if (filterState != 40) {
      digitalWrite(activate40, HIGH);
      //We just need a momentary pulse to latch the relay so wait 10ms and go low again
      delay(10);
      digitalWrite(activate40, LOW);
      filterState = 40;
    } 
  }
  else if (bandSet == 20) {
    if (filterState != 20) {
      digitalWrite(activate20, HIGH);
      //We just need a momentary pulse to latch the relay so wait 10ms and go low again
      delay(10);
      digitalWrite(activate20, LOW); 
      filterState = 20;
    }  
  }
  else {
    if (filterState != 10) {
      digitalWrite(activate10, HIGH);
      //We just need a momentary pulse to latch the relay so wait 10ms and go low again
      delay(10);
      digitalWrite(activate10, LOW); 
      filterState = 10;
    }
  }
}

void displayBand(int dispBand) {
  //Display the current band on the top right of the LCD display
  if (dispBand == 10) {
    // First Row
    lcd.home();
    lcd.print("10/12 Meters      ");
    // Second Row
    lcd.setCursor(5,1);
    lcd.print("Active");
  }
  else if ((dispBand == 15) || (dispBand == 17) || (dispBand == 20) || (dispBand == 40) || (dispBand == 80)){
    lcd.home();
    lcd.print(dispBand);
    lcd.print(" Meters      ");
    // Second Row
    lcd.setCursor(5,1);
    lcd.print("Active");
  }
  else {
    lcd.home();
    lcd.print(dispBand);
    lcd.print(" Meters     ");
    // Display Bypass on second row of display
    lcd.setCursor(5,1);
    lcd.print("Bypass");
  }
}

void loop() {
//digitalWrite(activate10, HIGH);
  
  // Read band value from the radio
  int band = getRadioBand();
  
  // Serial Debugging
  // Serial.println(band);

  // Keep the AGC Calm 
  delay(2);
  
  // Set filter and amplifier status based on band value from the radio
  if ((band == 10) || (band == 15)) { // 10, 12, and 15 meters share a filter
    //if (isFilterActive() != 10) {
      setBand(10);
    //}
    enableAmp();
    displayBand(band);
  }
  else if ((band == 17) || (band == 20)) { // 17 and 20 meters share a filter
    if (isFilterActive() != 20) {
      setBand(20);
    }
    enableAmp();
    displayBand(band);
  }
  else if (band == 30) { // No filter for 30 meters, bypass amp.
    displayBand(band);
    bypassAmp();
  }
  else if (band == 40) { // 40 Meters
    if (isFilterActive() != 40) {
      setBand(40);
    }
    enableAmp();
    displayBand(band);
  }
  else if (band == 60) { // No filter for 60 meters, bypass amp.
    displayBand(band);
    bypassAmp();
  }
  else if (band == 80) { // 80 Meters
    if (isFilterActive() != 80) {
      setBand(80);
    }
    enableAmp();
    displayBand(band);
  }
  else if (band == 160) { // No filter for 160 meters, bypass amp.
    displayBand(band);
    bypassAmp();
    // Display Bypass on second row of display
  }
  else { //Any other value is a communication error with the rig.  Bypass amp for safety and display an error message.
    lcd.home();
    lcd.print(" Rig Comm Error ");
    // Display Bypass on second row of display
    lcd.setCursor(5,1);
    lcd.print("Bypass"); 
    bypassAmp();    
  }
  
  
}
