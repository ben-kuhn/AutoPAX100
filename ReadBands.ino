/*
  ReadBands - Read the band output pin from the Xiegu G90 and prints to serial

  This sketch is to reverse engineer the voltages used by the band pin to create
  a future sketch to drive a Radioddity PAX100 and automatically select the correct
  band on the LPFX7 LPF.

  The circuit:
  - G90 Aux port Pin 2 (Red) connected to analog pin 0.
  - G90 Aux port Pin 7 (Violet) connected to ground

  created 29 Jan. 2021
  by Ben Kuhn
*/

// These constants won't change. They're used to give names to the pins used:
const int BandPin = A0;  // Analog input pin that the potentiometer is attached to

int BandPinValue = 0;        // value read from the radio


void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(9600);
  pinMode(BandPin, INPUT_PULLUP);
}

void loop() {
  // read the band output value from the radio:
  BandPinValue = analogRead(BandPin);

  // print the results to the Serial Monitor:
  Serial.print("Band Value = ");
  Serial.println(BandPinValue);

  // wait 2 seconds before the next loop for the analog-to-digital
  // converter to settle after the last reading:
  delay(2000);
}
