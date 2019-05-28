/*
KXKM - ESP32 audio & battery module
STM32 coprocessor hardware test

The following features are tested :
  * LEDs 1-3 (selected by 3-way switch)
  * LEDs 4-6 (selected by 3-way switch)
  * Load switch activation (displays current on the LED gauge)
  * Battery voltage measurement (display level on the LED gauge)
  * Temperature measurement (display level on the LED gauge)
  * ESP32 activation and communication (send a string and expect to receive one.)
  * Self power cut

The main push button is used to cycle through the tests.

When reading the battery voltage, a calibration value is stored.

REMEMBER TO SET THE SUPPLY VOLTAGE TO 24V BEFORE SWITCHING TO THE BATT TEST !

*/

#include <AceButton.h>

#define HW_REVISION 2

#if HW_REVISION == 1
  const uint8_t LED_PINS[] = {4,3,2,1};
  const uint8_t POWER_ENABLE_PIN = 12; //Self power enable. Keep HIGH to stay powered
  const uint8_t MAIN_OUT_ENABLE_PIN = 6; //Load switch enable line
  const uint8_t ESP32_ENABLE_PIN = 7; //ESP32 enable line
  const uint8_t PUSH_BUTTON_DETECT_PIN = 0; //Main On/off push button
  const uint8_t BATT_TYPE_SELECTOR_PINS[] = {10,11}; //3-way selector
  const uint8_t LOAD_CURRENT_SENSE_PIN = 5; //Load switch current measurement
  const uint8_t BATT_VOLTAGE_SENSE_PIN = 14; //Battery voltage measurement
  const uint8_t ESP32_TX_PIN = 8;

  const uint8_t LED_ORDERING[] = {1,0,3,5,4,2};

  const uint8_t LOAD_SENSE_GAIN = 8;
  
#elif HW_REVISION == 2
  const uint8_t LED_PINS[] = {3,2,0,1};
  const uint8_t POWER_ENABLE_PIN = 12; //Self power enable. Keep HIGH to stay powered
  const uint8_t MAIN_OUT_ENABLE_PIN = 6; //Load switch enable line
  const uint8_t ESP32_ENABLE_PIN = 7; //ESP32 enable line
  const uint8_t PUSH_BUTTON_DETECT_PIN = 13; //Main On/off push button
  const uint8_t BATT_TYPE_SELECTOR_PINS[] = {10,11}; //3-way selector
  const uint8_t LOAD_CURRENT_SENSE_PIN = 5; //Load switch current measurement
  const uint8_t BATT_VOLTAGE_SENSE_PIN = 14; //Battery voltage measurement
  const uint8_t ESP32_TX_PIN = 8;
  const uint8_t TEMP_MEAS_PIN = 4; //Thermistor measurement

  const uint8_t LED_ORDERING[] = {1,0,3,5,4,2};

  const uint8_t LOAD_SENSE_GAIN = 16;

#else
  #error "HW_REVISION undefined or invalid. Should be 1 or 2"
#endif


const unsigned int ADC_READS_COUNT = 256; // Averaging readings to improve resolution
const unsigned int ADC_AVG_CALIB = 16; // More average ! For calibration value

enum test_type_t {
  INIT,
  TEST_LED_1,
  TEST_LED_2,
  TEST_LOAD_SW,
  TEST_BATT_MEAS,
  TEST_TEMP_MEAS,
  TEST_ESP32,
  TEST_POWER_CUT
} currentTestType;

ace_button::AceButton button(PUSH_BUTTON_DETECT_PIN, LOW);

unsigned long lastSerialSend = 0;
bool calibrationDone = false;

void setup() {
  pinMode(POWER_ENABLE_PIN, OUTPUT);
  pinMode(ESP32_ENABLE_PIN, OUTPUT);
  pinMode(MAIN_OUT_ENABLE_PIN, OUTPUT);
  pinMode(PUSH_BUTTON_DETECT_PIN, INPUT);

  for (int i = 0; i < 2; i++)
    pinMode(BATT_TYPE_SELECTOR_PINS[i], INPUT_PULLUP);

  pinMode(ESP32_TX_PIN, INPUT); // Switch TX to High Z (shared with ESP32 programmation connector)
  
  #if HW_REVISION > 1
  pinMode(TEMP_MEAS_PIN, OUTPUT);
  digitalWrite(TEMP_MEAS_PIN, LOW); //Avoid thermistor self heating
  #endif

  digitalWrite(POWER_ENABLE_PIN, HIGH); //Keep 3.3V regulator enabled

  button.setEventHandler(handleButtonEvent);

  analogReadResolution(12);

  //Cycle through all LEDs
  for (int i = 0; i < 6; i++)
  {
    setLed(i);
    delay(50);
  }
  clearLeds();

  currentTestType = INIT;
}

void loop() {
  switch (currentTestType)
  {
    case TEST_LED_1:
      setLed(getBatteryTypeSelectorState());
      break;

    case TEST_LED_2:
      setLed(getBatteryTypeSelectorState()+3);
      break;

    case TEST_LOAD_SW:
    {
      uint32_t loadSense = analogRead(LOAD_CURRENT_SENSE_PIN);
      setLed(loadSense * 6 * LOAD_SENSE_GAIN / 4095);
      break;
    }

    case TEST_BATT_MEAS:
    {
      uint32_t adcRead = readBattVoltage();

      setLed(adcRead * 6 / 4095);
      openSerial();
      Serial1.println(adcRead);
      closeSerial();

      /* Wait for the button to be released because it may mess with the ADC measurement */
      if (!calibrationDone && button.isReleased(button.getLastButtonState()))
      {
        //Average further the ADC value before calibration
        for (int i = 0; i < ADC_AVG_CALIB-1; i++)
          adcRead += readBattVoltage();

        adcRead /= ADC_AVG_CALIB;

        // Store the current reading as calibration value.
        storeCalibrationValue(adcRead);
        openSerial();
        Serial1.print("Cal : ");
        Serial1.println(adcRead);
        closeSerial();

        calibrationDone = true;
      }

      break;
    }
    
    case TEST_TEMP_MEAS:
    {
      uint32_t voltage = analogRead(TEMP_MEAS_PIN);
      setLed(voltage * 6 / 4095);
      break;
    }

    case TEST_ESP32:
      if (Serial1.available())
      {
        setLed(Serial1.read() % 6);
      }

      if (millis() - lastSerialSend > 500)
      {
        lastSerialSend = millis();
        openSerial();
        Serial1.write("ping\n");
        closeSerial();
      }
      break;

    default:
      break;
  };

  button.check();
  delay(5);
}

void handleButtonEvent(ace_button::AceButton* button, uint8_t eventType, uint8_t buttonState) {
  switch (eventType) {
    //case AceButton::kEventPressed:
    //  digitalWrite(LED_BUILTIN, LED_ON);
    //  break;
    case ace_button::AceButton::kEventReleased:
      endTest(currentTestType);
      currentTestType = (test_type_t)((int)currentTestType + 1);
      beginTest(currentTestType);
      break;
  }
}

void beginTest(test_type_t test)
{
  switch (test)
  {
    case TEST_LOAD_SW:
      digitalWrite(MAIN_OUT_ENABLE_PIN, HIGH);
      break;

    #if HW_REVISION > 1
    case TEST_TEMP_MEAS:
      pinMode(TEMP_MEAS_PIN, INPUT);
      break;
    #endif
    
    case TEST_ESP32:
      digitalWrite(ESP32_ENABLE_PIN, HIGH);
      break;

    case TEST_POWER_CUT:
      setLed(0);
      pinMode(POWER_ENABLE_PIN, INPUT);
      break;

    default:
      break;
  };
}

void endTest(test_type_t test)
{
  clearLeds();

  switch (test)
  {
    case TEST_LOAD_SW:
      digitalWrite(MAIN_OUT_ENABLE_PIN, LOW);
      break;
      
    #if HW_REVISION > 1
    case TEST_TEMP_MEAS:
      pinMode(TEMP_MEAS_PIN, OUTPUT);
      digitalWrite(TEMP_MEAS_PIN, LOW); //Avoid thermistor self heating
      break;
    #endif


    case TEST_ESP32:
      digitalWrite(ESP32_ENABLE_PIN, LOW);
      break;

    default:
      break;
  };
}

uint8_t getBatteryTypeSelectorState()
{
  if (!digitalRead(BATT_TYPE_SELECTOR_PINS[0]))
    return 0;

  else if (!digitalRead(BATT_TYPE_SELECTOR_PINS[1]))
    return 1;

  else
    return 2;
}

void setLed(uint8_t index)
{
  clearLeds();

  //Physical ordering
  uint8_t phyIndex = LED_ORDERING[index];

  pinMode(LED_PINS[phyIndex/2], OUTPUT);
  pinMode(LED_PINS[phyIndex/2 + 1], OUTPUT);

  if (phyIndex % 2 == 1)
  {
    digitalWrite(LED_PINS[phyIndex/2], LOW);
    digitalWrite(LED_PINS[phyIndex/2 + 1], HIGH);
  }
  else
  {
    digitalWrite(LED_PINS[phyIndex/2], HIGH);
    digitalWrite(LED_PINS[phyIndex/2 + 1], LOW);
  }
}

void clearLeds()
{
  for (int i = 0; i < 4; i++)
    pinMode(LED_PINS[i], INPUT);
}

void openSerial()
{
  Serial1.begin(115200);
}

void closeSerial()
{
  Serial1.flush();
  pinMode(ESP32_TX_PIN, INPUT); //Set TX pin to Hi Z to allow ESP32 programmation from external connector
}

uint32_t readBattVoltage()
{
  uint32_t adcRead = 0;
  for (int i = 0; i < ADC_READS_COUNT; i++)
  {
    adcRead += analogRead(BATT_VOLTAGE_SENSE_PIN);
  }
  adcRead /= ADC_READS_COUNT;

  return adcRead;
}

void storeCalibrationValue(uint16_t calibValue)
{
  FLASH_OBProgramInitTypeDef OB_0, OB_1; // programming option structure
  OB_0.DATAAddress = OB_DATA_ADDRESS_DATA0; // address of type FLASHEx_OB_Data_Address
  OB_0.DATAData = (uint8_t)(calibValue & 0xFF); // value to be saved
  OB_0.OptionType = OPTIONBYTE_DATA; // of type FLASHEx_OB_Type

  OB_1.DATAAddress = OB_DATA_ADDRESS_DATA1; // address of type FLASHEx_OB_Data_Address
  OB_1.DATAData = (uint8_t)((calibValue >> 8) & 0xFF); // value to be saved
  OB_1.OptionType = OPTIONBYTE_DATA; // of type FLASHEx_OB_Type

  // unlock FLASH in general
  if(HAL_FLASH_Unlock() == HAL_OK) {
    // unlock option bytes in particular
    if(HAL_FLASH_OB_Unlock() == HAL_OK) {
       // erase option bytes before programming
       if(HAL_FLASHEx_OBErase() == HAL_OK) {
          // program selected option byte
          HAL_FLASHEx_OBProgram(&OB_0); // result not checked as there is no recourse at this point
          HAL_FLASHEx_OBProgram(&OB_1); // result not checked as there is no recourse at this point
          if(HAL_FLASH_OB_Lock() == HAL_OK) {
             HAL_FLASH_Lock(); // again, no recourse
            // HAL_FLASH_OB_Launch(); // reset occurs here (sorry, debugger)
          }
       }
    }
  }
}
