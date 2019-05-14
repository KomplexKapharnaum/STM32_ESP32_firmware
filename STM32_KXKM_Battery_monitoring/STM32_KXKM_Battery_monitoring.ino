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
    * critical section handling (the board will stay powered if the main processor requests it)
    * display arbitrary data on the LED gauge

The processor serial port is available on the ESP32 programmation connector. RX & TX must be swapped.

Tom Magnier - 04/2018
*/

#include <AceButton.h>
#include "KXKM_STM32_energy_API.h"
#include "pin_mapping.h"
#include "board_id.h"

// Firmware version
const int FIRMWARE_VERSION = 2;

// Timing configuration
const unsigned long STARTUP_GUARD_TIME_MS = 5000; // Ignore long presses during this period after startup
const unsigned long BATT_DISPLAY_TIME_MS = 3000; // Display the battery level during this time then shut down the LEDs
const unsigned long BATT_DISPLAY_DELAY_MS = 300; // Display the battery level after this delay when the button is pushed.
const unsigned long LOAD_SWITCH_START_DELAY_MS = 2000; // Start the load switch after this delay (to allow the ESP32 to disable the load switch if needed)
const unsigned long MAX_CRITICAL_SECTION_DURATION_MS = 10000; // Maximum critical section duration.
const unsigned long CUSTOM_LED_DISPLAY_TIME_MS = 2000; // Display the custom LED display during this period (set by serial API)

enum state_t {
  INIT,
  ESP32_STARTUP,
  ACTIVE,
  CRITICAL_SECTION_WAIT,
  SHUTDOWN
};

state_t currentState;
void enterState(enum state_t newState); // F*** Arduino. Explicit prototype function to avoid the auto generated one.

ace_button::AceButton button(PUSH_BUTTON_DETECT_PIN, LOW);
KXKM_STM32_Energy::PushButtonEvent buttonEvent = KXKM_STM32_Energy::NO_EVENT;
unsigned long battLevelDisplayStartTime;
unsigned long customLedSetTime;
unsigned long lastStateChangeTime;
unsigned long criticalSectionEndTime;

#define SERIAL_DEBUG(str) \
  beginSerial(); \
  Serial1.print(str); \
  endSerial();

void setup() {
  pinMode(POWER_ENABLE_PIN, OUTPUT);
  pinMode(ESP32_ENABLE_PIN, OUTPUT);
  pinMode(MAIN_OUT_ENABLE_PIN, OUTPUT);
  pinMode(PUSH_BUTTON_DETECT_PIN, INPUT);

  setESP32State(false); //Force reset if necesary

  for (int i = 0; i < 2; i++)
    pinMode(BATT_TYPE_SELECTOR_PINS[i], INPUT_PULLUP);

  initSerial();

  ace_button::ButtonConfig* buttonConfig = button.getButtonConfig();
  buttonConfig->setEventHandler(handleButtonEvent);
  buttonConfig->setFeature(ace_button::ButtonConfig::kFeatureClick);
  buttonConfig->setFeature(ace_button::ButtonConfig::kFeatureDoubleClick);
  buttonConfig->setFeature(ace_button::ButtonConfig::kFeatureLongPress);

  // To keep interactions consistent, a long press is required to start up the board.
  // If the MCU is still powered at the end of the delay, we can move along.
  delay(button.getButtonConfig()->getLongPressDelay());

  currentState = INIT;

  initLedGauge();

  // Shut down immediately if the battery type could not be determined and the
  // selector is not in "Custom" position
  if (!initBatteryMonitoring())
  {
    //Blink red LED while the board is powered (button pressed)
    while(1)
    {
      setSingleLed(0, 1);
      delay(50);
      clearLeds();
      delay(50);
    }
  }

  // Power up the board
  set3V3RegState(true); //Keep 3.3V regulator enabled

  // Start up LED animation
  for (uint8_t i = 0; i <= 100; i++)
  {
    setLedGaugePercentage(i);
    delay(4);
  }
  clearLeds();

  initWatchdog();
  enterState(ESP32_STARTUP);
}

void loop()
{
  refreshWatchdog();
  button.check();
  loopBatteryMonitoring();

  switch (currentState)
  {
    case ESP32_STARTUP:
      if (millis() - lastStateChangeTime > LOAD_SWITCH_START_DELAY_MS)
      {
        setLoadSwitchState(true);
        enterState(ACTIVE);
      }
      break;

    case ACTIVE:
      if (millis() - customLedSetTime > CUSTOM_LED_DISPLAY_TIME_MS)
      {
        // Display the battery level if battery is low or the push button has been pressed
        if ((millis() - battLevelDisplayStartTime > 0 && millis() - battLevelDisplayStartTime < BATT_DISPLAY_TIME_MS) || (getBatteryPercentage() >= 0 && getBatteryPercentage() < 10))
          displayBatteryLevel(getBatteryPercentage());
        else if (millis() - battLevelDisplayStartTime > BATT_DISPLAY_TIME_MS)
          displaySingleLedBatteryLevel(getBatteryPercentage());
      }

      if (getBatteryPercentage() == 0)
        enterState(CRITICAL_SECTION_WAIT); //Start shutdown process
      break;

    case CRITICAL_SECTION_WAIT:
      // Display a wait indicator
      {
        uint8_t idx = ((millis() - lastStateChangeTime) % 1100) / 100;
        if (idx < 6)
          setSingleLed(idx);
        else
          setSingleLed(10 - idx);
      }

      if (millis() > criticalSectionEndTime || millis() - lastStateChangeTime > MAX_CRITICAL_SECTION_DURATION_MS)
        enterState(SHUTDOWN);
      break;

    default:
      break;
  }
}


void handleButtonEvent(ace_button::AceButton* button, uint8_t eventType, uint8_t buttonState) {
  switch (eventType) {
    case ace_button::AceButton::kEventClicked:
      //Display the battery percentage on the LED gauge. Leave some time for the ESP32 to override the default behavior if needed.
      if (battLevelDisplayStartTime + BATT_DISPLAY_TIME_MS > millis())
        battLevelDisplayStartTime = millis();
      else 
        battLevelDisplayStartTime = millis() + BATT_DISPLAY_DELAY_MS;
      
      buttonEvent = KXKM_STM32_Energy::BUTTON_CLICK_EVENT;
      break;

    case ace_button::AceButton::kEventDoubleClicked:
    buttonEvent = KXKM_STM32_Energy::BUTTON_DOUBLE_CLICK_EVENT;
      break;

    case ace_button::AceButton::kEventLongPressed:
      if (millis() > STARTUP_GUARD_TIME_MS)
      {
        //Shut down LED animation
        for (int i = 100; i >= 0; i--)
        {
          setLedGaugePercentage(i);
          delay(4);
        }

        enterState(CRITICAL_SECTION_WAIT); //Start shutdown process
      }
      break;
  }
}


void enterState(enum state_t newState)
{
  // SERIAL_DEBUG(newState);

  // Exit current state actions
  switch (currentState)
  {
    default:
      break;
  }

  // Enter new state actions
  switch (newState)
  {
    case ESP32_STARTUP:
      setESP32State(true); //Enable ESP32
      break;

    case SHUTDOWN:
      shutdown();
      break;

    default:
      break;
  }

  currentState = newState;
  lastStateChangeTime = millis();
}
