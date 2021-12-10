
#include "Arduino.h"
#include "PinNames.h"
#include "pinmap.h"


const PinName myPins[18] = {
  PA_0,   //D0
  PA_1,   //D1
  PA_2,   //D2
  PA_3,   //D3
  PA_4,   //D4
  PA_5,   //D5
  PA_6,   //D6
  PA_7,   //D7
  PA_9,   //D8
  PA_10,  //D9
  PA_13,  //D10
  PA_14,  //D11
  PF_0,   //D12
  PF_1,   //D13
  PB_1,   //D14
  PA_4,   //D15/A0 - Duplicated for ADC use
  PA_5,   //D16/A1
  PB_1,   //D17/A2
};

void setup() 
{

  for (int k=0; k<18; k++) {

    PinName pin = myPins[k];

    // pinMode OUTPUT
    pin_function(pin, STM_PIN_DATA(STM_MODE_OUTPUT_PP, GPIO_NOPULL, 0));

    // write HIGH
    digital_io_write(get_GPIO_Port(STM_PORT(pin)), STM_LL_GPIO_PIN(pin), HIGH);
  }

}

void loop() 
{

}