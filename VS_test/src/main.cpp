
// HW_REVISION
#define HW_REVISION 2

#include "Arduino.h"

#include "pin_mapping.h"
#include "led_gauge.h"

void setup()
{
  pinMode(POWER_ENABLE_PIN, OUTPUT);
  pinMode(ESP32_ENABLE_PIN, OUTPUT);
  pinMode(MAIN_OUT_ENABLE_PIN, OUTPUT);
  pinMode(PUSH_BUTTON_DETECT_PIN, INPUT);

  for (int i = 0; i < 2; i++)
    pinMode(BATT_TYPE_SELECTOR_PINS[i], INPUT_PULLUP);

  initLedGauge();

  // // Power up the board
  // set3V3RegState(true); //Keep 3.3V regulator enabled
  digitalWrite(POWER_ENABLE_PIN, HIGH);
}

void loop()
{
  if (digitalRead(AUTO_BOOT_PIN))
  {

    // Start up LED animation
    for (uint8_t i = 1; i <= 100; i++)
    {
      setLedGaugePercentage(i);
      delay(40);
    }
    for (uint8_t i = 100; i >= 1; i--)
    {
      setLedGaugePercentage(i);
      delay(40);
    }
    clearLeds();
    for (uint8_t i = 1; i <= 100; i++)
    {
      displaySingleLedBatteryLevel(i);
      delay(40);
    }
    for (uint8_t i = 100; i >= 1; i--)
    {
      displaySingleLedBatteryLevel(i);
      delay(40);
    }
    clearLeds();
  }
  else
  {
    setLedGaugePercentage(5);
  }
}
