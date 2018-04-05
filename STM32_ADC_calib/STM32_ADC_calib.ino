/*
KXKM - ESP32 audio & battery module
STM32 ADC calib test
*/


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

const int ADC_AVG_COUNT = 256;

void setup() {
  pinMode(POWER_ENABLE_PIN, OUTPUT);
  pinMode(ESP32_ENABLE_PIN, OUTPUT);
  pinMode(MAIN_OUT_ENABLE_PIN, OUTPUT);
  pinMode(PUSH_BUTTON_DETECT_PIN, INPUT);

  for (int i = 0; i < 2; i++)
    pinMode(BATT_TYPE_SELECTOR_PINS[i], INPUT_PULLUP);

  pinMode(ESP32_TX_PIN, INPUT); // Switch TX to High Z (shared with ESP32 programmation connector)

  digitalWrite(POWER_ENABLE_PIN, HIGH); //Keep 3.3V regulator enabled

  analogReadResolution(12);

  Serial1.begin(115200);

  Serial1.print("OB : ");
  Serial1.println(readCalibrationValue());

  // Store into option byte the current ADC read
  uint32_t adcRead = 0;
  for (int i = 0; i < ADC_AVG_COUNT; i++)
  {
    adcRead += analogRead(BATT_VOLTAGE_SENSE_PIN);
  }
  adcRead /= ADC_AVG_COUNT;

  delay(2000);

  Serial1.print("Storing ");
  Serial1.println(adcRead);

  delay(1000);

  storeCalibrationValue(adcRead);

}

void loop() {

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
             //HAL_FLASH_OB_Launch(); // reset occurs here (sorry, debugger)
          }
       }
    }
  }
}

uint16_t readCalibrationValue()
{
  return (HAL_FLASHEx_OBGetUserData(OB_DATA_ADDRESS_DATA1) << 8) + HAL_FLASHEx_OBGetUserData(OB_DATA_ADDRESS_DATA0);
}
