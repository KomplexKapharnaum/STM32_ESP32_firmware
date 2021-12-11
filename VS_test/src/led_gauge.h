/* LED gauge functions
 * Each LED can be set individually, with 5 PWM levels (0-4).
 *
 * There are multiple helper functions to light up a single LED, use the gauge as a floating point number display, etc.
 * Internally, the LED state is stored as a single unsigned long int. Each LED uses 4 bits.
 */

const uint8_t LED_PWM_MAX_VAL = 4;

volatile uint32_t _ledGaugeState; // LED state. Don't change directly ! Use the helper functions. Update should be atomic because the timer interrupt may trigger at any time.

HardwareTimer *_timer;

volatile uint8_t _currentLedIndex = 0;
volatile uint8_t _currentCycleIndex = 0; //For each LED there are LED_PWM_MAX_VAL cycles

void initLedGauge();
void clearLeds();
void setSingleLed(uint8_t index);
void setSingleLed(uint8_t index, uint8_t value);
void setLed(uint8_t index);
void setLed(uint8_t index, uint8_t value);
void setLedGaugePercentage(int value);
void displayBatteryLevel(int value);
void displaySingleLedBatteryLevel(int value);
void ledTimerInterrupt(void);

/** Initialize LED gauge display */
void initLedGauge()
{
  _ledGaugeState = 0;

  _timer = new HardwareTimer(TIM16);
  _timer->setOverflow(1000, HERTZ_FORMAT); // ~1ms
  _timer->attachInterrupt(ledTimerInterrupt);
  _timer->resume();

  // TimerHandleInit(&_timer, 1, (uint16_t)(HAL_RCC_GetHCLKFreq() / 1000) / 4); // ~1ms
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

/** Set a LED at full brightness without clearing the others */
void setLed(uint8_t index)
{
  setLed(index, LED_PWM_MAX_VAL);
}

/** Set a LED at the specified value without clearing the others */
void setLed(uint8_t index, uint8_t value)
{
  index = constrain(index, 0, LED_COUNT-1);
  value = constrain(value, 0, LED_PWM_MAX_VAL);

  uint32_t state = _ledGaugeState;

  state &= ~(0x0F << (4*index));
  state |= value << (4*index);

  _ledGaugeState = state;
}

/** Use the gauge to display a number in the range [0 - 100] */
void setLedGaugePercentage(int value)
{
  value = constrain(value, 0, 100) * LED_COUNT;
  uint32_t state = 0;

  uint8_t i;
  for (i = 0; i < value / 100; i++)
    state |= (LED_PWM_MAX_VAL << (4*i));

  state |= (((value - (value / 100) * 100) * LED_PWM_MAX_VAL / 100) << (4*i));

  //Atomic update
  _ledGaugeState = state;
}

/** Use the gauge to display the battery level :
  * If the level is unknown (-1), blinks all LEDs
  * If the level is below 10% blink the last LED
  */
void displayBatteryLevel(int value)
{
  if (value < 0)
  {
    if (millis() % 1000 < 500)
      setLedGaugePercentage(100);
    else
      clearLeds();
  }
  else if (value < 10)
  {
    if (millis() % 1000 < 600 && millis() % 200 < 100)
      setSingleLed(0);
    else
      clearLeds();
  }
  else
  {
    setLedGaugePercentage(value);
  }
}

/* Display the battery level on a single LED, as a 'ON' indicator. */
void displaySingleLedBatteryLevel(int value)
{
  if (value < 0)
    setSingleLed(0, 1);
  else
    setSingleLed(value * 6 / 100, 1);
}

// Timer interrupt
void ledTimerInterrupt(void)
{
  uint8_t value = (uint8_t)(_ledGaugeState >> (4*_currentLedIndex) ) & 0x0F;

  if (_currentCycleIndex == 0 && value > 0)
  {
    //Turn off all LEDs
    for (int i = 0; i < LED_PINS_COUNT; i++)
      pinMode(LED_PINS[i], INPUT);

    // Light up the current LED
    uint8_t phyIndex = LED_ORDERING[_currentLedIndex]; //Physical ordering

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

  if (_currentCycleIndex >= value)
  {
    //Turn off all LEDs
    for (int i = 0; i < LED_PINS_COUNT; i++)
      pinMode(LED_PINS[i], INPUT);
  }

  _currentCycleIndex++;

  if (_currentCycleIndex >= LED_PWM_MAX_VAL)
  {
    _currentCycleIndex = 0;
    _currentLedIndex++;

    if (_currentLedIndex >= LED_COUNT)
      _currentLedIndex = 0;
  }
}