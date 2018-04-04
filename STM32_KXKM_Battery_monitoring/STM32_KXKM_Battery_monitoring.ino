/*
KXKM - ESP32 audio & battery module
STM32 coprocessor - battery monitoring function

The processor is in charge of the following tasks :
  * Keep the 3.3V enable line high on power up (self power)
  * Control the ESP32 enable line and the main load switch
  * Monitor the input voltage and shut down the whole board (3.3V, load switch, ESP32) on low battery
  * Push button monitoring :
    * on short press, display the battery level on the LED gauge
    * on long press, shut down the whole board
  * Communication with the ESP32 processor :
    * battery level reporting
    * custom battery profile input
    * push button reporting
    * warning before shut down (e.g. to prevent SD card corruption)
    * display arbitrary data on the LED gauge

Tom Magnier - 04/2018
*/

#include <AceButton.h>

// Hardware definitions
const uint8_t LED_PINS[] = {4,3,2,1};
const uint8_t POWER_ENABLE_PIN = 12; //Self power enable. Keep HIGH to stay powered
const uint8_t MAIN_OUT_ENABLE_PIN = 6; //Load switch enable line
const uint8_t ESP32_ENABLE_PIN = 7; //ESP32 enable line
const uint8_t PUSH_BUTTON_DETECT_PIN = 0; //Main On/off push button
const uint8_t BATT_TYPE_SELECTOR_PINS[] = {10,11}; //3-way selector
const uint8_t LOAD_CURRENT_SENSE_PIN = A0; //Load switch current measurement
const uint8_t BATT_VOLTAGE_SENSE_PIN = A1; //Battery voltage measurement
const uint8_t ESP32_TX_PIN = 8;

const uint8_t LED_ORDERING[] = {1,0,3,5,4,2};


ace_button::AceButton button(PUSH_BUTTON_DETECT_PIN);

void setup() {
  pinMode(POWER_ENABLE_PIN, OUTPUT);
  pinMode(ESP32_ENABLE_PIN, OUTPUT);
  pinMode(MAIN_OUT_ENABLE_PIN, OUTPUT);
  pinMode(PUSH_BUTTON_DETECT_PIN, INPUT);

  for (int i = 0; i < 2; i++)
    pinMode(BATT_TYPE_SELECTOR_PINS[i], INPUT_PULLUP);

  pinMode(ESP32_TX_PIN, INPUT); // Switch TX to High Z (shared with ESP32 programmation connector)

  analogReadResolution(10);

  ace_button::ButtonConfig* buttonConfig = button.getButtonConfig();
  buttonConfig->setEventHandler(handleButtonEvent);
  buttonConfig->setFeature(ace_button::ButtonConfig::kFeatureClick);
  buttonConfig->setFeature(ace_button::ButtonConfig::kFeatureLongPress);

  // To keep interactions consistent, a long press is required to start up the board.
  // If the MCU is still powered at the end of the delay, we can move along.
  delay(button.getButtonConfig()->getLongPressDelay());

  //TODO check battery voltage & shut down if too low

  // Power up the board
  digitalWrite(POWER_ENABLE_PIN, HIGH); //Keep 3.3V regulator enabled
  digitalWrite(MAIN_OUT_ENABLE_PIN, HIGH);
  digitalWrite(ESP32_ENABLE_PIN, HIGH);

  initLedGauge();

  //Cycle through all LEDs
  for (uint8_t i = 0; i < 6; i++)
  {
    setSingleLed(i);
    delay(50);
  }
  clearLeds();
}

void loop()
{
  button.check();

  setLedGaugePercentage((millis() % 2000) / 20);
  delay(10);
}


void handleButtonEvent(ace_button::AceButton* button, uint8_t eventType, uint8_t buttonState) {
  switch (eventType) {
    case ace_button::AceButton::kEventClicked:

      break;

    case ace_button::AceButton::kEventLongPressed:

      break;
  }
}
