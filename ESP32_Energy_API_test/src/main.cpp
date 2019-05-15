/*
KXKM - ESP32 audio & battery module
STM32 Energy API test from ESP32

Battery voltage and percentage ; load current ; board temperature are queried every 2s.

The following features are tested :
  * setting LEDs independently
  * using the LED gauge to display a percentage
  * enabling / disabling the load switch (the load switch is explicitly disabled at startup)
  * setting custom batt characteristics (12-14V)
  * Starting a critical section with a 8s timeout
    * Leaving the critical section (click)
    * requesting a shutdown (double click)
  * Request a self reset

To test the critical section, it should be done by setting the voltage under 12V.

The main push button is used to cycle through the tests on clicks (via STM32 serial).

Connect with telnet to get the debug information 
*/

#include <Arduino.h>
#include "KXKM_STM32_energy_API.h"
#include "serial.h"
#include <WiFi.h>
#include <DNSServer.h>
#include "ESPmDNS.h"
#include "RemoteDebug.h"        //https://github.com/JoaoLopesF/RemoteDebug

#define HOST_NAME "KXKM_ESP32_Energy_API_Test"

const unsigned long BATTERY_CHECK_PERIOD_MS = 2000;
const unsigned long BUTTON_CHECK_PERIOD_MS = 200;

enum test_type_t {
  STARTUP,
  INIT,
  TEST_LED_1,
  TEST_LED_2,
  TEST_LOAD_SW,
  TEST_CUSTOM_BATT,
  TEST_ENTER_CRITICAL_SECTION,
  TEST_LEAVE_CRITICAL_SECTION,
  TEST_SELF_RESET
} currentTestType;

RemoteDebug Debug;

const char* ssid = "ssid";
const char* password = "password";

void processCmdRemoteDebug();
void beginTest(test_type_t test);
void endTest(test_type_t test);

void setup() {
  Serial.begin(115200, SERIAL_8N1);
  Serial.setTimeout(10);

  //Disable the load switch immediately
  debugI("Disabling load switch.");
  sendSerialCommand(KXKM_STM32_Energy::SET_LOAD_SWITCH, 0);
  
  WiFi.begin(ssid, password);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    static int ledId = 0;
    setSingleLed(ledId++ % 6);
    delay(300);
  }

  // Register host name in WiFi and mDNS
  MDNS.begin(HOST_NAME);
  MDNS.addService("telnet", "tcp", 23);
  
  // Initialize RemoteDebug
  Debug.begin(HOST_NAME); // Initialize the WiFi server
  Debug.setResetCmdEnabled(true); // Enable the reset command
  Debug.showProfiler(true); // Profiler (Good to measure times, to optimize codes)
  Debug.showColors(true); // Colors
  Debug.setHelpProjectsCmds("begin to begin tests. Then use the button to cycle through the tests");
  Debug.setCallBackProjectCmds(&processCmdRemoteDebug);

  currentTestType = STARTUP;
}

void beginTesting() {
  debugI("Beginning Energy API test sketch.");

  currentTestType = INIT;
  
  sendSerialCommand(KXKM_STM32_Energy::GET_HW_REVISION);
  debugI("Hardware revision : %d", readSerialAnswer());
  
  sendSerialCommand(KXKM_STM32_Energy::GET_BOARD_ID);
  debugI("Board ID : %d", readSerialAnswer());

  sendSerialCommand(KXKM_STM32_Energy::GET_API_VERSION);
  debugI("STM32 API version : %d", readSerialAnswer());
  debugI("Local API version : %d", KXKM_STM32_Energy::API_VERSION);

  sendSerialCommand(KXKM_STM32_Energy::GET_FW_VERSION);
  debugI("STM32 firmware version : %d", readSerialAnswer());

  sendSerialCommand(KXKM_STM32_Energy::GET_BATTERY_TYPE);
  debugI("Battery type : ");
  switch (readSerialAnswer())
  {
    case KXKM_STM32_Energy::BATTERY_LIPO: debugI("LiPo"); break;
    case KXKM_STM32_Energy::BATTERY_LIFE: debugI("LiFe"); break;
    case KXKM_STM32_Energy::BATTERY_CUSTOM: debugI("custom"); break;
  }
}

void loop() {
  static unsigned long lastBatteryCheck, lastButtonCheck;
  if (millis() - lastBatteryCheck > BATTERY_CHECK_PERIOD_MS)
  {
    lastBatteryCheck = millis();
    sendSerialCommand(KXKM_STM32_Energy::GET_BATTERY_VOLTAGE);
    debugI("Batt voltage : %d mV", readSerialAnswer());

    sendSerialCommand(KXKM_STM32_Energy::GET_BATTERY_PERCENTAGE);
    debugI("Batt percentage : %d %%", readSerialAnswer());
    
    sendSerialCommand(KXKM_STM32_Energy::GET_LOAD_CURRENT);
    debugI("Load current : %d mA", readSerialAnswer());
    
    sendSerialCommand(KXKM_STM32_Energy::GET_TEMPERATURE);
    debugI("Board temperature : %d deg. C", readSerialAnswer());
  }

  if (millis() - lastButtonCheck > BUTTON_CHECK_PERIOD_MS)
  {
    lastButtonCheck = millis();
    sendSerialCommand(KXKM_STM32_Energy::GET_BUTTON_EVENT);

    int buttonEvent = readSerialAnswer();
    if (buttonEvent == KXKM_STM32_Energy::BUTTON_CLICK_EVENT)
    {
      debugI("Main button clicked.");
      endTest(currentTestType);
      currentTestType = (test_type_t)((int)currentTestType + 1);
      beginTest(currentTestType);
    }
    else if (buttonEvent == KXKM_STM32_Energy::BUTTON_DOUBLE_CLICK_EVENT)
    {
      debugI("Main button double clicked.");
      switch (currentTestType)
      {
        case TEST_ENTER_CRITICAL_SECTION:
        case TEST_LEAVE_CRITICAL_SECTION:
        {
          debugI("Shutting down...");
          unsigned int currentTime = millis();
          while (millis() - currentTime < 1000)
          {
            Debug.handle();          
          }
          sendSerialCommand(KXKM_STM32_Energy::SHUTDOWN);
          break;
        }

        default:
          break;
      }
    }
  }

  switch (currentTestType)
  {
    case TEST_LED_1:
    {
      static unsigned long ledUpdateTime = millis();
      if (millis() - ledUpdateTime > 100)
      {
        ledUpdateTime = millis();
        uint8_t leds[] = {0, 0, 0, 0, 0, 0};
        static uint8_t idx = 0;
        leds[idx] = 4;
        if (idx < 5)
          leds[idx + 1] = 2;
        if (idx > 0)
          leds[idx - 1] = 1;

        setLeds(leds);
        idx = (idx + 1) % 6;
      }
      break;
    }

    case TEST_LED_2:
    {
      static unsigned long ledUpdateTime = millis();
      if (millis() - ledUpdateTime > 20)
      {
        ledUpdateTime = millis();
        static uint8_t percentage = 0;
        sendSerialCommand(KXKM_STM32_Energy::SET_LED_GAUGE, percentage++);

        if (percentage >= 100)
          percentage = 0;
      }
      break;
    }

    default:
      break;
  };
  
  Debug.handle();
}

void beginTest(test_type_t test)
{
  switch (test)
  {
    case TEST_LOAD_SW:
      debugI("Enabling load switch.");
      sendSerialCommand(KXKM_STM32_Energy::SET_LOAD_SWITCH, 1);
      break;

    case TEST_CUSTOM_BATT:
      debugI("Setting new battery characteristics.");
      sendSerialCommand(KXKM_STM32_Energy::SET_BATTERY_VOLTAGE_LOW, 12000);
      sendSerialCommand(KXKM_STM32_Energy::SET_BATTERY_VOLTAGE_3, 12500);
      sendSerialCommand(KXKM_STM32_Energy::SET_BATTERY_VOLTAGE_6, 14000);
      break;

    case TEST_ENTER_CRITICAL_SECTION:
      debugI("Entering critical section for 8s.");
      sendSerialCommand(KXKM_STM32_Energy::ENTER_CRITICAL_SECTION, 8000);
      break;

    case TEST_LEAVE_CRITICAL_SECTION:
      debugI("Leaving critical section");
      sendSerialCommand(KXKM_STM32_Energy::LEAVE_CRITICAL_SECTION);
      break;

    case TEST_SELF_RESET:
      debugI("Self reset");
      sendSerialCommand(KXKM_STM32_Energy::REQUEST_RESET);

    default:
      break;
  };
}

void endTest(test_type_t test)
{
  switch (test)
  {
    case TEST_LOAD_SW:
      debugI("Disabling load switch.");
      sendSerialCommand(KXKM_STM32_Energy::SET_LOAD_SWITCH, 0);
      break;

    default:
      break;
  };
}


void processCmdRemoteDebug() {
	String lastCmd = Debug.getLastCommand();

	if (lastCmd == "begin") {
    beginTesting();
	}
}
