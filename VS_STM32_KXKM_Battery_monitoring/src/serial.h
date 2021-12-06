/* Serial communication functions.
 *
 * Used for debug and communication with the ESP32.
 *
 * The STM32 and the programmation connector use the same serial port on the ESP32
 * so the TX line must be released (switched to High-Z) after each communication to
 * leave the programmation connector available.
 */
#include "KXKM_STM32_energy_API.h"

void endSerial();
void sendAnswer(int value);

void initSerial()
{
  Serial1.begin(115200);
  Serial1.setTimeout(2);
  endSerial();
}

void beginSerial()
{
  //Set TX pin to UART TX function. Taken from uart.c
  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_TypeDef *port;
  uint32_t function = (uint32_t)NC;

  port = set_GPIO_Port_Clock(STM_PORT(digitalPinToPinName(ESP32_TX_PIN)));
  function = pinmap_function(digitalPinToPinName(ESP32_TX_PIN), PinMap_UART_TX);
  GPIO_InitStruct.Pin         = STM_GPIO_PIN(digitalPinToPinName(ESP32_TX_PIN));
  GPIO_InitStruct.Mode        = STM_PIN_MODE(function);
  GPIO_InitStruct.Speed       = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Pull        = STM_PIN_PUPD(function);
  GPIO_InitStruct.Alternate   = STM_PIN_AFNUM(function);
  HAL_GPIO_Init(port, &GPIO_InitStruct);
}

void endSerial()
{
  Serial1.flush();
  pinMode(ESP32_TX_PIN, INPUT_PULLUP); //Set TX pin to Hi Z to allow ESP32 programmation from external connector
}

void serialEvent1()
{
  //SERIAL_DEBUG(Serial1.readStringUntil('\n'));

  while (Serial1.available())
  {
    char rdChar = Serial1.peek();
    if (rdChar == KXKM_STM32_Energy::PREAMBLE[0])
      break;
    else
      Serial1.read();
  }

  if (Serial1.findUntil(KXKM_STM32_Energy::PREAMBLE, "\n"))
  {
    char cmd;
    Serial1.readBytesUntil(' ', &cmd, 1); //Wait if necessary for data to arrive
    long arg = 0;

    // SERIAL_DEBUG(cmd);
    // SERIAL_DEBUG(Serial1.available());
    // SERIAL_DEBUG(Serial1.readStringUntil('\n'));

    if (KXKM_STM32_Energy::hasArgument((KXKM_STM32_Energy::CommandType)cmd))
    {
      char tmp;
      arg = Serial1.parseInt();
      //SERIAL_DEBUG(arg);
    }

    switch (cmd)
    {
      case KXKM_STM32_Energy::GET_HW_REVISION:
        sendAnswer(HW_REVISION);
        break;

      case KXKM_STM32_Energy::GET_HW_ID:
        sendAnswer(HW_ID);
        break;
        
      case KXKM_STM32_Energy::GET_BOARD_ID:
        sendAnswer(BOARD_ID);
        break;
        
      case KXKM_STM32_Energy::GET_API_VERSION:
        sendAnswer(KXKM_STM32_Energy::API_VERSION);
        break;

      case KXKM_STM32_Energy::GET_FW_VERSION:
        sendAnswer(FIRMWARE_VERSION);
        break;

      case KXKM_STM32_Energy::GET_BATTERY_VOLTAGE:
        sendAnswer(getInstantBatteryVoltage());
        break;

      case KXKM_STM32_Energy::GET_BATTERY_PERCENTAGE:
        sendAnswer(getBatteryPercentage());
        break;

      case KXKM_STM32_Energy::GET_BATTERY_TYPE:
        sendAnswer(_battType);
        break;
      
      case KXKM_STM32_Energy::GET_LOAD_CURRENT:
        sendAnswer(getInstantLoadCurrent());
        break;
      
      case KXKM_STM32_Energy::GET_TEMPERATURE:
        sendAnswer(readApproxTempDegC());
        break;

      case KXKM_STM32_Energy::SET_LEDS:
        customLedSetTime = millis();
        for (int i = 0; i < 6; i++)
        {
          // SERIAL_DEBUG(arg % 10);
          setLed(i, arg % 10);
          arg /= 10;
        }
        break;

      case KXKM_STM32_Energy::SET_LED_GAUGE:
        customLedSetTime = millis();
        setLedGaugePercentage(arg);
        break;

      case KXKM_STM32_Energy::SET_LOAD_SWITCH:
        setLoadSwitchState(arg > 0);

        //Override default behavior if still in ESP32 startup time
        if (currentState == ESP32_STARTUP)
          enterState(ACTIVE);
        break;

      case KXKM_STM32_Energy::SHUTDOWN:
        enterState(SHUTDOWN);
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
        //Accept custom batt characteristics only if the selector is on "Custom" position
        if (_battType == KXKM_STM32_Energy::BATTERY_CUSTOM)
          _battVoltageBreaks[cmd - KXKM_STM32_Energy::SET_BATTERY_VOLTAGE_LOW] = arg;
        break;

      case KXKM_STM32_Energy::ENTER_CRITICAL_SECTION:
        criticalSectionEndTime = millis() + constrain(arg, 0, MAX_CRITICAL_SECTION_DURATION_MS);
        //SERIAL_DEBUG(millis());
        //SERIAL_DEBUG(criticalSectionEndTime);
        break;

      case KXKM_STM32_Energy::LEAVE_CRITICAL_SECTION:
        criticalSectionEndTime = millis();
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