/* Serial communication functions.
 *
 * Used for debug and communication with the ESP32.
 *
 * The STM32 and the programmation connector use the same serial port on the ESP32
 * so the TX line must be released (switched to High-Z) after each communication to
 * leave the programmation connector available.
 */
#include "KXKM_STM32_energy_API.h"

void beginSerial()
{
  Serial1.begin(115200);
}

void endSerial()
{
  Serial1.flush();
  pinMode(ESP32_TX_PIN, INPUT); //Set TX pin to Hi Z to allow ESP32 programmation from external connector
}

void serialEvent1()
{
  if (Serial1.find(KXKM_STM32_Energy::PREAMBLE))
  {
    char cmd = Serial1.read();
    long arg = 0;

    if (KXKM_STM32_Energy::hasArgument((KXKM_STM32_Energy::CommandType)cmd))
      arg = Serial1.parseInt();

    switch (cmd)
    {
      case KXKM_STM32_Energy::GET_BATTERY_VOLTAGE:
        sendAnswer(_avgBattVoltage);
        break;

      case KXKM_STM32_Energy::GET_BATTERY_PERCENTAGE:
        sendAnswer(getBatteryPercentage());
        break;

      case KXKM_STM32_Energy::SET_LEDS:
        for (int i = 0; i < 6; i++)
        {
          setSingleLed(i, arg % 10);
          arg /= 10;
        }
        break;

      case KXKM_STM32_Energy::SET_LED_GAUGE:
        setLedGaugePercentage(arg);
        break;

      case KXKM_STM32_Energy::SET_LOAD_SWITCH:
        //TODO if not set disable autoset
        setLoadSwitchState(arg > 0);
        break;

      case KXKM_STM32_Energy::SHUTDOWN:
        //TODO leave critical section ?
        shutdown();
        break;

      case KXKM_STM32_Energy::REQUEST_RESET:
        setESP32State(false);
        delay(10);
        setESP32State(true);
        break;

      case KXKM_STM32_Energy::SET_BATTERY_VOLTAGE_LOW:
      case KXKM_STM32_Energy::SET_BATTERY_VOLTAGE_1:
      case KXKM_STM32_Energy::SET_BATTERY_VOLTAGE_2:
      case KXKM_STM32_Energy::SET_BATTERY_VOLTAGE_3:
      case KXKM_STM32_Energy::SET_BATTERY_VOLTAGE_4:
      case KXKM_STM32_Energy::SET_BATTERY_VOLTAGE_5:
      case KXKM_STM32_Energy::SET_BATTERY_VOLTAGE_6:
        _battVoltageBreaks[cmd - KXKM_STM32_Energy::SET_BATTERY_VOLTAGE_LOW] = arg;
        break;

      case KXKM_STM32_Energy::ENTER_CRITICAL_SECTION:
        //TODO
        //TODO max timeout
        break;

      case KXKM_STM32_Energy::LEAVE_CRITICAL_SECTION:
        //TODO
        break;

      case KXKM_STM32_Energy::GET_BUTTON_EVENT:
        sendAnswer((int)buttonEvent);
        buttonEvent = KXKM_STM32_Energy::NO_EVENT;
        break;

      default:
        break;
    };
  }
}

void sendAnswer(int value)
{
  beginSerial();
  Serial1.write(KXKM_STM32_Energy::PREAMBLE);
  Serial1.println(value);
  endSerial();
}
