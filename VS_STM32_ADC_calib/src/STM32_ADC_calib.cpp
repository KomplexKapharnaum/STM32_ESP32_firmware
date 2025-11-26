/*
KXKM - ESP32 audio & battery module
STM32 ADC calib test
REMEMBER TO SET THE SUPPLY VOLTAGE TO 24V TO THE BATT CALIBRATE !

*/
#include "Arduino.h"
#include <AceButton.h>

const uint8_t LED_PINS[] = {4,3,2,1};
const uint8_t POWER_ENABLE_PIN = 12; //Self power enable. Keep HIGH to stay powered
const uint8_t MAIN_OUT_ENABLE_PIN = 6; //Load switch enable line
const uint8_t ESP32_ENABLE_PIN = 7; //ESP32 enable line
const uint8_t PUSH_BUTTON_DETECT_PIN = 13; //Main On/off push button
const uint8_t BATT_TYPE_SELECTOR_PINS[] = {10,11}; //3-way selector
const uint8_t LOAD_CURRENT_SENSE_PIN = A0; //Load switch current measurement
const uint8_t BATT_VOLTAGE_SENSE_PIN = A2; //Battery voltage measurement
const uint8_t ESP32_TX_PIN = 8;

const uint8_t LED_ORDERING[] = {1,0,3,5,4,2};

const int ADC_AVG_COUNT = 256;

/**
 * Configure our STM32 for battery ADC's calibration
 */
void stmConfig()
{
  pinMode(POWER_ENABLE_PIN, OUTPUT); // stay powered
  pinMode(ESP32_ENABLE_PIN, OUTPUT);
  pinMode(MAIN_OUT_ENABLE_PIN, OUTPUT);
  pinMode(PUSH_BUTTON_DETECT_PIN, INPUT);

  for (int i = 0; i < 2; i++)
    pinMode(BATT_TYPE_SELECTOR_PINS[i], INPUT_PULLUP);

  pinMode(ESP32_TX_PIN, INPUT); // Switch TX to High Z (shared with ESP32 programmation connector)

  digitalWrite(POWER_ENABLE_PIN, HIGH); //Keep 3.3V regulator enabled

  analogReadResolution(12);

  Serial1.begin(115200);
}

/**
 * Store calibration value in the user Option Bytes
 * Use two bytes for store the 16 bits interger
 * Return 0 if all was fine
 */
uint8_t storeCalibrationValue(uint16_t calibValue)
{
  FLASH_OBProgramInitTypeDef OB_0, OB_1; // programming option structure
  OB_0.DATAAddress = OB_DATA_ADDRESS_DATA0; // address of type FLASHEx_OB_Data_Address
  OB_0.DATAData = (uint8_t)(calibValue & 0xFF); // value to be saved, filter the 8 less significant bits
  OB_0.OptionType = OPTIONBYTE_DATA; // of type FLASHEx_OB_Type

  OB_1.DATAAddress = OB_DATA_ADDRESS_DATA1; // address of type FLASHEx_OB_Data_Address
  OB_1.DATAData = (uint8_t)((calibValue >> 8) & 0xFF); // value to be saved, filter the 8 most significants bits
  OB_1.OptionType = OPTIONBYTE_DATA; // of type FLASHEx_OB_Type
  
  // unlock FLASH in general
  if(HAL_FLASH_Unlock() == HAL_OK) {
    // unlock option bytes in particular
    if(HAL_FLASH_OB_Unlock() == HAL_OK) {
       // erase option bytes before programming
       if(HAL_FLASHEx_OBErase() == HAL_OK) {
          if (HAL_FLASHEx_OBProgram(&OB_0) == HAL_OK && HAL_FLASHEx_OBProgram(&OB_1) == HAL_OK) {
            // program selected option byte
            Serial1.print("Flash status : ");
            Serial1.println(HAL_FLASH_GetError());
            if(HAL_FLASH_OB_Lock() == HAL_OK && HAL_FLASH_Lock() == HAL_OK) {
                //HAL_FLASH_OB_Launch(); // reset occurs here (sorry, debugger)
                return(0);
            }
            return(1);
          }
          return(2);
       }
       return(3);
    }
    return(4);
  }
  return(5);
}

/**
 * Get the two bytes in the Option Bytes
 * Return the stored uint16
 */
uint16_t readCalibrationValue()
{
  return (HAL_FLASHEx_OBGetUserData(OB_DATA_ADDRESS_DATA1) << 8) + HAL_FLASHEx_OBGetUserData(OB_DATA_ADDRESS_DATA0);
}

/**
 * Print to serial the value stored in the Option Bytes
 */
void printCalib(){
  Serial1.print("Stored calibration :  ");
  Serial1.println(readCalibrationValue());
}

/**
 * Read and average the ADC value
 */
uint16_t getAdcVal()
{
  Serial1.println("Reading ADC value...");

  delay(500);
  
  uint32_t adcRead = 0;
  for (int i = 0; i < ADC_AVG_COUNT; i++)
  {
    adcRead += analogRead(BATT_VOLTAGE_SENSE_PIN);
    delay(10);
  }
  adcRead /= ADC_AVG_COUNT;

  delay(500);
  return(adcRead);
}

/**
 * Calibration logic.
 * Show previously stored Option Byte, then read and store new.
 * If the storage was succesful, we reset the SMT, otherwise we show an error code
 */
void calibrate(){
  
  Serial1.println("----- STARTED -------");
  printCalib();

  // Get the current ADC read
  uint16_t adcVal = getAdcVal();
  
  Serial1.print("Readed ADC value : ");
  Serial1.println(adcVal);

  uint8_t writeStatus = storeCalibrationValue(adcVal);

  if(writeStatus == 0){
    Serial1.println("----- STORED -------");
    Serial1.println("Reseting....");
    delay(300);
    HAL_FLASH_OB_Launch();
  }
  else{
    Serial1.println("----- ERROR -------");
    Serial1.print("Status : ");
    Serial1.println(writeStatus);
  }
}


/**
 * Configure the button event
 */
ace_button::AceButton button(PUSH_BUTTON_DETECT_PIN, LOW);
void handleButtonEvent(ace_button::AceButton *button, uint8_t eventType, uint8_t buttonState) {
  switch (eventType) {
  case ace_button::AceButton::kEventClicked:
    calibrate();
  break;
  }
}


/**
 * Run once at boot
 */
void setup() {
  
  bool obReset = __HAL_RCC_GET_FLAG(RCC_FLAG_OBLRST);
  bool swReset = __HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST);
  bool porReset = __HAL_RCC_GET_FLAG(RCC_FLAG_PORRST);
  
  stmConfig();

  ace_button::ButtonConfig *buttonConfig = button.getButtonConfig();
  buttonConfig->setEventHandler(handleButtonEvent);
  buttonConfig->setFeature(ace_button::ButtonConfig::kFeatureClick);
  buttonConfig->setClickDelay(300);


  delay(3000); // Let some time for start serial monitor

  if (obReset) {
    printCalib();
    Serial1.println("   --> If you have just flash this STM, push the button for launch calibration process, otherwise all was done <--");
    Serial1.println("       Rock'n'roll and thanks for all the fish");
  }
  else {
    Serial1.println("  --> Push the button for launch calibration process <--");
  }

}

void loop() {
  button.check();
}