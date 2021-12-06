#ifndef BOARD_H
#define BOARD_H

#include <Arduino.h>

// Hardware definitions
// HW_REVISION is defined from the board variant (Hardware version menu in Arduino IDE)
const uint8_t LED_PINS_COUNT = 4;
const uint8_t LED_COUNT = 6;

#if HW_REVISION == 1
// const uint8_t LED_PINS[LED_PINS_COUNT] = {4, 3, 2, 1};
// const uint8_t POWER_ENABLE_PIN = 12;                //Self power enable. Keep HIGH to stay powered
// const uint8_t MAIN_OUT_ENABLE_PIN = 6;              //Load switch enable line
// const uint8_t ESP32_ENABLE_PIN = 7;                 //ESP32 enable line
// const uint8_t PUSH_BUTTON_DETECT_PIN = 0;           //Main On/off push button
// const uint8_t BATT_TYPE_SELECTOR_PINS[] = {10, 11}; //3-way selector
// const uint8_t LOAD_CURRENT_SENSE_PIN = 5;           //Load switch current measurement
// const uint8_t BATT_VOLTAGE_SENSE_PIN = 14;          //Battery voltage measurement
// const uint8_t ESP32_TX_PIN = 8;

// const uint8_t LED_ORDERING[LED_COUNT] = {1, 0, 3, 5, 4, 2};

const uint8_t LED_PINS[LED_PINS_COUNT] = {PA4, PA3, PA2, PA1};
const uint8_t POWER_ENABLE_PIN = PF0;                //Self power enable. Keep HIGH to stay powered
const uint8_t MAIN_OUT_ENABLE_PIN = PA6;              //Load switch enable line
const uint8_t ESP32_ENABLE_PIN = PA7;                 //ESP32 enable line
const uint8_t PUSH_BUTTON_DETECT_PIN = PA0;           //Main On/off push button
const uint8_t BATT_TYPE_SELECTOR_PINS[] = {PA13, PA14}; //3-way selector
const uint8_t LOAD_CURRENT_SENSE_PIN = PA5;           //Load switch current measurement
const uint8_t BATT_VOLTAGE_SENSE_PIN = PB1;          //Battery voltage measurement
const uint8_t ESP32_TX_PIN = PA10;                     //

const uint8_t LED_ORDERING[LED_COUNT] = {1, 0, 3, 5, 4, 2};

#elif HW_REVISION == 2
// const uint8_t LED_PINS[LED_PINS_COUNT] = {3,2,0,1};
// const uint8_t POWER_ENABLE_PIN = 12; //Self power enable. Keep HIGH to stay powered
// const uint8_t MAIN_OUT_ENABLE_PIN = 6; //Load switch enable line
// const uint8_t ESP32_ENABLE_PIN = 7; //ESP32 enable line
// const uint8_t PUSH_BUTTON_DETECT_PIN = 13; //Main On/off push button
// const uint8_t BATT_TYPE_SELECTOR_PINS[] = {10,11}; //3-way selector
// const uint8_t LOAD_CURRENT_SENSE_PIN = 5; //Load switch current measurement
// const uint8_t BATT_VOLTAGE_SENSE_PIN = 14; //Battery voltage measurement
// const uint8_t ESP32_TX_PIN = 8;
// const uint8_t TEMP_MEAS_PIN = 4; //Thermistor measurement

// const uint8_t LED_ORDERING[LED_COUNT] = {1,0,3,5,4,2};

const uint8_t LED_PINS[LED_PINS_COUNT] = {PA3, PA2, PA0, PA1};
const uint8_t POWER_ENABLE_PIN = PF0;                   //Self power enable. Keep HIGH to stay powered
const uint8_t MAIN_OUT_ENABLE_PIN = PA6;                //Load switch enable line
const uint8_t ESP32_ENABLE_PIN = PA7;                   //ESP32 enable line
const uint8_t PUSH_BUTTON_DETECT_PIN = PF1;             //Main On/off push button
const uint8_t BATT_TYPE_SELECTOR_PINS[] = {PA13, PA14}; //3-way selector
const uint8_t LOAD_CURRENT_SENSE_PIN = PA5;             //Load switch current measurement
const uint8_t BATT_VOLTAGE_SENSE_PIN = PB1;             //Battery voltage measurement
const uint8_t ESP32_TX_PIN = PA10;                      //
const uint8_t TEMP_MEAS_PIN = PA4;                      //Thermistor measurement

const uint8_t LED_ORDERING[LED_COUNT] = {1, 0, 3, 5, 4, 2};

#elif HW_REVISION == 3
// const uint8_t LED_PINS[LED_PINS_COUNT] = {3,2,0,1};
// const uint8_t POWER_ENABLE_PIN = 12; //Self power enable. Keep HIGH to stay powered
// const uint8_t MAIN_OUT_ENABLE_PIN = 6; //Load switch enable line
// const uint8_t ESP32_ENABLE_PIN = 7; //ESP32 enable line
// const uint8_t PUSH_BUTTON_DETECT_PIN = 13; //Main On/off push button
// const uint8_t BATT_TYPE_SELECTOR_PINS[] = {10,11}; //3-way selector
// const uint8_t LOAD_CURRENT_SENSE_PIN = 5; //Load switch current measurement
// const uint8_t BATT_VOLTAGE_SENSE_PIN = 14; //Battery voltage measurement
// const uint8_t ESP32_TX_PIN = 8;
// const uint8_t TEMP_MEAS_PIN = 4; //Thermistor measurement

// const uint8_t LED_ORDERING[LED_COUNT] = {1,0,3,5,4,2};

const uint8_t LED_PINS[LED_PINS_COUNT] = {PA3, PA2, PA0, PA1};
const uint8_t POWER_ENABLE_PIN = PF0;                   //Self power enable. Keep HIGH to stay powered
const uint8_t MAIN_OUT_ENABLE_PIN = PA6;                //Load switch enable line
const uint8_t ESP32_ENABLE_PIN = PA7;                   //ESP32 enable line
const uint8_t PUSH_BUTTON_DETECT_PIN = PF1;             //Main On/off push button
const uint8_t BATT_TYPE_SELECTOR_PINS[] = {PA13, PA14}; //3-way selector
const uint8_t LOAD_CURRENT_SENSE_PIN = PA5;             //Load switch current measurement
const uint8_t BATT_VOLTAGE_SENSE_PIN = PB1;             //Battery voltage measurement
const uint8_t ESP32_TX_PIN = PA10;                      //
const uint8_t TEMP_MEAS_PIN = PA4;                      //Thermistor measurement

const uint8_t LED_ORDERING[LED_COUNT] = {1, 0, 3, 5, 4, 2};

#else
#error "HW_REVISION undefined or invalid. Should be 1 or 2 or 3"
#endif

#endif