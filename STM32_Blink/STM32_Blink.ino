/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.

  This example code is in the public domain.
 */

// Pin 13 has an LED connected on most Arduino boards.
// Pin 11 has the LED on Teensy 2.0
// Pin 6  has the LED on Teensy++ 2.0
// Pin 13 has the LED on Teensy 3.0
// give it a name:
uint8_t led_pins[] = {1,2,3,4};
uint8_t power_enable_pin = 12; //Self power enable. Keep HIGH to stay powered

// the setup routine runs once when you press reset:
void setup() {
  // initialize the digital pin as an output.
  for (int i = 0; i < 4; i++)
    pinMode(led_pins[i], OUTPUT);

  pinMode(power_enable_pin, OUTPUT);
  digitalWrite(power_enable_pin, HIGH);
}

// the loop routine runs over and over again forever:
void loop() {
  //static uint8_t state = 0;
  digitalWrite(led_pins[0], HIGH);   // turn the LED on (HIGH is the voltage level)
  digitalWrite(led_pins[1], LOW);
  delay(1000);               // wait for a second
  digitalWrite(led_pins[0], LOW);    // turn the LED off by making the voltage LOW
  digitalWrite(led_pins[1], HIGH);
  delay(1000);               // wait for a second

  if (millis() > 10000)
    digitalWrite(power_enable_pin, LOW); //cut power
}
