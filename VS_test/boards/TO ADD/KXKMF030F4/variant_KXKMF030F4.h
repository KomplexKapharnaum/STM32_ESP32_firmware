/*
 *******************************************************************************
 * Copyright (c) 2020-2021, STMicroelectronics
 * All rights reserved.
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 *******************************************************************************
 */
#pragma once

// #include "PeripheralPins.h"

// #ifdef __cplusplus
// extern "C"{
// #endif // __cplusplus

// extern const PinName digitalPin[];

// enum {
//   PA0,  //D0
//   PA1,  //D1
//   PA2,  //D2
//   PA3,  //D3
//   PA4,  //D4
//   PA5,  //D5
//   PA6,  //D6
//   PA7,  //D7
//   PA9,  //D8
//   PA10, //D9
//   PA13, //D10
//   PA14, //D11
//   PF0,  //D12
//   PF1,  //D13,
//   PB1,  //D14
//   PA4_2,  //D15/A0 - Duplicated for ADC use
//   PA5_2,  //D16/A1
//   PB1_2,  //D17/A2
//   PEND
// };

/*----------------------------------------------------------------------------
 *        STM32 pins number
 *----------------------------------------------------------------------------*/
// #define PA0         D0
// #define PA1         D1
// #define PA2         D2
// #define PA3         D3
// #define PA4         D4
// #define PA5         D5
// #define PA6         D6
// #define PA7         D7
// #define PA9         D9
// #define PA10        D10
// #define PA13        D13
// #define PA14        D14
// #define PF0         D12
// #define PF1         D13
// #define PB1         D8
// #define PA4_2       A0
// #define PA5_2       A1
// #define PB1_2       A2

#define NUM_DIGITAL_PINS 18
#define NUM_ANALOG_INPUTS 3

// On-board LED pin number
#define LED_BUILTIN 4

// On-board user button

// Timer Definitions
#define TIMER_TONE TIM16

#define TIMER_SERVO TIM3

// UART Definitions
#define SERIAL_UART_INSTANCE 1

// Default pin used for generic 'Serial' instance
// Mandatory for Firmata
#define PIN_SERIAL_RX 9

#define PIN_SERIAL_TX 8

// #ifdef __cplusplus
// } // extern "C"
// #endif

/*----------------------------------------------------------------------------
 *        Arduino objects - C++ only
 *----------------------------------------------------------------------------*/

#ifdef __cplusplus
// These serial port names are intended to allow libraries and architecture-neutral
// sketches to automatically default to the correct port name for a particular type
// of use.  For example, a GPS module would normally connect to SERIAL_PORT_HARDWARE_OPEN,
// the first hardware serial port whose RX/TX pins are not dedicated to another use.
//
// SERIAL_PORT_MONITOR        Port which normally prints to the Arduino Serial Monitor
//
// SERIAL_PORT_USBVIRTUAL     Port which is USB virtual serial
//
// SERIAL_PORT_LINUXBRIDGE    Port which connects to a Linux system via Bridge library
//
// SERIAL_PORT_HARDWARE       Hardware serial port, physical RX & TX pins.
//
// SERIAL_PORT_HARDWARE_OPEN  Hardware serial ports which are open for use.  Their RX & TX
//                            pins are NOT connected to anything by default.
#define SERIAL_PORT_MONITOR Serial
#define SERIAL_PORT_HARDWARE Serial

#endif
