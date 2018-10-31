/*
KXKM - ESP32 audio & battery module
OLED screen test

Battery voltage and percentage are queried every 2s and displayed on screen.
*/

#include "KXKM_STM32_energy_API.h"
#include "SSD1306Wire.h"

const unsigned long BATTERY_CHECK_PERIOD_MS = 200;

SSD1306Wire  display(0x3c, 2, 4); //SDA = 2, SCL = 4

int stm32FwVer = 0;
String battType;

void setup() {
  Serial.begin(115200, SERIAL_8N1);
  Serial.setTimeout(10);

  Serial.println("Beginning OLED test sketch.");

  sendSerialCommand(KXKM_STM32_Energy::GET_FW_VERSION);
  stm32FwVer = readSerialAnswer();

  sendSerialCommand(KXKM_STM32_Energy::GET_BATTERY_TYPE);
  switch (readSerialAnswer()) {
    case KXKM_STM32_Energy::BATTERY_LIPO: battType = "LiPo"; break;
    case KXKM_STM32_Energy::BATTERY_LIFE: battType = "LiFe"; break;
    case KXKM_STM32_Energy::BATTERY_CUSTOM: battType = "custom"; break;
  }

  display.init();
}

void loop() {
  static unsigned long lastBatteryCheck;
  if (millis() - lastBatteryCheck > BATTERY_CHECK_PERIOD_MS)
  {
    lastBatteryCheck = millis();

    display.clear();
    display.drawString(0, 0, "STM32 FW ver. " + String(stm32FwVer));
    display.drawString(0, 10, "Battery type : " + battType);

    sendSerialCommand(KXKM_STM32_Energy::GET_BATTERY_VOLTAGE);
    display.drawString(0, 38, "Battery voltage : " + String((float)readSerialAnswer() / 1000.0) + "V");

    sendSerialCommand(KXKM_STM32_Energy::GET_BATTERY_PERCENTAGE);
    display.drawString(0, 48, "Batt percentage : " + String(readSerialAnswer()) + "%");

    display.display();
  }
}
