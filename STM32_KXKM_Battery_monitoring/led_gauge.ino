/* LED gauge functions
 * Each LED can be set individually, with a 3-bit (8 values) PWM level.
 *
 * There are multiple helper functions to light up a single LED, use the gauge as a floating point number display, etc.
 * Internally, the LED state is stored as a single unsigned long int..
 */

const uint8_t LED_PWM_MAX_VAL = 7;
const uint8_t LED_COUNT = 6;
const uint8_t LED_PINS_COUNT = 4;

volatile uint32_t _ledGaugeState;

stimer_t _timer;
volatile uint8_t _currentLedIndex = 0;

/** Initialize LED gauge display */
void initLedGauge()
{
  _ledGaugeState = 0;

  _timer.timer = TIM16;
  attachIntHandle(&_timer, ledTimerInterrupt);
  TimerHandleInit(&_timer, 2, (uint16_t)(HAL_RCC_GetHCLKFreq() / 1000) - 1); //2ms
}

void clearLeds()
{
  for (int i = 0; i < LED_PINS_COUNT; i++)
    pinMode(LED_PINS[i], INPUT);

  _ledGaugeState = 0;
}

/** Light up a single LED at full brightness */
void setSingleLed(uint8_t index)
{
  setSingleLed(index, LED_PWM_MAX_VAL);
}

/** Light up a single LED at the specified value */
void setSingleLed(uint8_t index, uint8_t value)
{
  index = constrain(index, 0, LED_COUNT-1);
  value = constrain(value, 0, LED_PWM_MAX_VAL);
  _ledGaugeState = value << (4*index);
}

/** Use the gauge to display a number in the range [0 - 100] */
void setLedGaugePercentage(int value)
{
  value = constrain(value, 0, 100) * LED_COUNT;
  _ledGaugeState = 0;

  uint8_t i;
  for (i = 0; i < value / 100; i++)
    _ledGaugeState |= (LED_PWM_MAX_VAL << (4*i));

  _ledGaugeState |= (((value - (value / 100) * 100) * LED_PWM_MAX_VAL / 100) << (4*i));
}



// Timer interrupt
void ledTimerInterrupt(stimer_t *timer)
{
  for (int i = 0; i < LED_PINS_COUNT; i++)
  pinMode(LED_PINS[i], INPUT);

  uint8_t value = (uint8_t)(_ledGaugeState >> (4*_currentLedIndex) ) & 0x0F;

  if (value > 0)
  {
    //Physical ordering
    uint8_t phyIndex = LED_ORDERING[_currentLedIndex];

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

  _currentLedIndex++;

  if (_currentLedIndex >= LED_COUNT)
    _currentLedIndex = 0;
}
