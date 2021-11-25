/* Battery monitoring functions
 *
 * Battery monitoring is based on the following inputs :
 *  * input voltage measurement through a resistive voltage divider
 *  * battery type selection by a 3-way switch :
 *    * pos 1 : Li-Po batteries - Nominal cell voltage 3.7V, max cell voltage 4.2V, min cell voltage 3.0V
 *    * pos 2 : Li-Fe batteries - Nominal cell voltage 3.3V, max cell voltage 3.6V, min cell voltage 2.8V
 *    * pos 3 : custom. By default the battery voltage is not monitored, except if custom min/max voltages are
 *              received on the serial port.
 *
 * The number of cells is determined at startup by measuring the initial voltage.
 * As the number of cells increase, voltage ranges will overlap. To address this issue, the initial min cell voltage is
 * 3.5V for LiPo batteries, e.g. a minimum state of charge is required to start up the device.
 * Additionnally, some combinations are not recognized (5S, 6S for LiPo and LiFe) and are supported only in custom mode.
 *
 * The following document has been used for LiPo voltage breaks (0.5C):
 * https://learn.adafruit.com/li-ion-and-lipoly-batteries/voltages
 * To preserve the batteries, they are not discharged beyond 90% (3.5V).
 *
 * LiFePo4 : http://gwl-power.tumblr.com/post/98740855201/winston-battery-lifeypo4-discharge
 *
 * The voltage is interpolated linearly between voltage breaks.
 */

KXKM_STM32_Energy::BatteryType getBatteryTypeSelectorState();
void loopBatteryMonitoring();
unsigned int readBatteryVoltage();
unsigned int getAverageBatteryVoltage();
unsigned int getInstantBatteryVoltage();
unsigned int readLoadCurrent();
unsigned int getInstantLoadCurrent();
int readApproxTempDegC();
int getBatteryPercentage();
uint16_t readCalibrationValue();
uint8_t findCellCount(unsigned int voltage, unsigned int cellMin, unsigned int cellMax);

/* All voltages are given in mV */
const unsigned int LIPO_VOLTAGE_BREAKS[] = {3500, 3650, 3700, 3750, 3825, 3950, 4200}; //For one cell
const unsigned int LIFE_VOLTAGE_BREAKS[] = {2920, 3140, 3200, 3220, 3240, 3260, 3600}; //For one cell

const unsigned int INITIAL_CELL_VOLTAGE_TOLERANCE = 50; // Tolerance added to the cell charged voltage

const unsigned int ADC_READS_COUNT = 4; // Averaging readings to improve resolution
const unsigned int CALIBRATION_VOLTAGE = 24000; // Voltage used for the calibration

const unsigned long ADC_READ_PERIOD_MS = 5; // ADC read every 5ms

const int BATT_LOW_LEVEL = 10; // Low battery level (%)

// ADC reading : exponential averaging
// Time constant (samples) =  -1 / ln(OLD_WEIGHT/(OLD_WEIGHT+NEW_WEIGHT))
// Time constant (seconds) =  - ADC_READ_PERIOD_MS / 1000 * ln(OLD_WEIGHT/(OLD_WEIGHT+NEW_WEIGHT))
const unsigned int LONG_TERM_OLD_WEIGHT = 999; //Time constant : around 5s
const unsigned int LONG_TERM_NEW_WEIGHT = 1;
const unsigned int SHORT_TERM_OLD_WEIGHT = 95; //Time constant : around 0.1s
const unsigned int SHORT_TERM_NEW_WEIGHT = 5;

//Load switch current reading : V (mV) = R (ohm) * I_load (mA) / 10000
#if HW_REVISION == 1
  const unsigned long CURRENT_MEAS_RESISTOR = 470;
#elif HW_REVISION == 2
  const unsigned long CURRENT_MEAS_RESISTOR = 2000;
  #elif HW_REVISION == 3
  const unsigned long CURRENT_MEAS_RESISTOR = 2000;
#endif
// Multiplier is split in two to avoid overflow.
const unsigned long CURRENT_MEAS_MULTIPLIER1 = 3300 * 100;
const unsigned long CURRENT_MEAS_DIVIDER =  (4095 * CURRENT_MEAS_RESISTOR);
const unsigned long CURRENT_MEAS_MULTIPLIER2 = 100;

//Increase fixed point precision to allow longer time constants.
const unsigned int VOLTAGE_MEAS_DECIMAL_PART = 5; // 2^5
const unsigned int CURRENT_MEAS_DECIMAL_PART = 5; // 2^5

unsigned int _battVoltageBreaks[7];
unsigned int _avgBattVoltage;
unsigned int _instantBattVoltage;
unsigned int _instantLoadCurrent;
KXKM_STM32_Energy::BatteryType _battType;

/* Initialize battery monitoring resources and determine the type and voltage of
 the attached battery.

 Return : true if it is safe to continue operation, false if the board should not be
 powered.
 */
bool initBatteryMonitoring()
{
  analogReadResolution(12);

  _avgBattVoltage = _instantBattVoltage = readBatteryVoltage();
  _instantLoadCurrent = readLoadCurrent();
  
  _battType = getBatteryTypeSelectorState();

  switch (_battType)
  {
    case KXKM_STM32_Energy::BATTERY_LIPO: //LiPo
    {
      uint8_t cells = findCellCount(getAverageBatteryVoltage(), LIPO_VOLTAGE_BREAKS[0], LIPO_VOLTAGE_BREAKS[6]);

      //SERIAL_DEBUG("LiPo");
      //SERIAL_DEBUG(cells);

      for (int i = 0; i < 7; i++)
        _battVoltageBreaks[i] = cells * LIPO_VOLTAGE_BREAKS[i];

      if (cells == 0)
        return false; // The selector is on the LiPo / LiFe state but the voltage doesn't match !

      break;
    }

    case KXKM_STM32_Energy::BATTERY_LIFE: //LiFe
    {
      uint8_t cells = findCellCount(getAverageBatteryVoltage(), LIFE_VOLTAGE_BREAKS[0], LIFE_VOLTAGE_BREAKS[6]);

      //SERIAL_DEBUG("LiFe");
      //SERIAL_DEBUG(cells);

      for (int i = 0; i < 7; i++)
        _battVoltageBreaks[i] = cells * LIFE_VOLTAGE_BREAKS[i];

      if (cells == 0)
        return false; // The selector is on the LiPo / LiFe state but the voltage doesn't match !

      break;
    }
    case KXKM_STM32_Energy::BATTERY_CUSTOM: //Custom. Battery monitoring disabled or will be set later.
    default:
      // SERIAL_DEBUG("custom");
      for (int i = 0; i < 7; i++)
        _battVoltageBreaks[i] = 0;
      break;
  }

  return true;
}

/* Perform regular battery monitoring actions (update ADC reading, check voltage, etc) */
void loopBatteryMonitoring()
{
  static unsigned long lastAdcRead = millis();
  if (millis() - lastAdcRead > ADC_READ_PERIOD_MS)
  {
    lastAdcRead = millis();
    
    //Exponential smoothing
    _instantBattVoltage = (_instantBattVoltage * SHORT_TERM_OLD_WEIGHT + readBatteryVoltage() * SHORT_TERM_NEW_WEIGHT) / (SHORT_TERM_OLD_WEIGHT + SHORT_TERM_NEW_WEIGHT);
    _instantLoadCurrent = (_instantLoadCurrent * SHORT_TERM_OLD_WEIGHT + readLoadCurrent() * SHORT_TERM_NEW_WEIGHT) / (SHORT_TERM_OLD_WEIGHT + SHORT_TERM_NEW_WEIGHT);
    _avgBattVoltage = (_avgBattVoltage * LONG_TERM_OLD_WEIGHT + _instantBattVoltage * LONG_TERM_NEW_WEIGHT) / (LONG_TERM_OLD_WEIGHT + LONG_TERM_NEW_WEIGHT);
  }
}

/* Take a battery voltage measurement and return the result in mV */
unsigned int readBatteryVoltage()
{
  unsigned int adcRead = 0;

  // unsigned long millisStart = millis();

  for (int i = 0; i < ADC_READS_COUNT; i++)
    adcRead += analogRead(BATT_VOLTAGE_SENSE_PIN);

  adcRead /= ADC_READS_COUNT;

  // unsigned long millisEnd = millis();

  // SERIAL_DEBUG(adcRead);
  // SERIAL_DEBUG(millisEnd - millisStart);

  return (adcRead << VOLTAGE_MEAS_DECIMAL_PART) * CALIBRATION_VOLTAGE / readCalibrationValue();
}

/* Return the average battery voltage */
unsigned int getAverageBatteryVoltage()
{
  return _avgBattVoltage >> VOLTAGE_MEAS_DECIMAL_PART;
}

unsigned int getInstantBatteryVoltage()
{
  return _instantBattVoltage >> VOLTAGE_MEAS_DECIMAL_PART;
}

/* Take a load current measurement and return the result in mA */
unsigned int readLoadCurrent()
{
  unsigned long adcRead = 0;

  // unsigned long millisStart = millis();

  for (int i = 0; i < ADC_READS_COUNT; i++)
    adcRead += analogRead(LOAD_CURRENT_SENSE_PIN);

  adcRead /= ADC_READS_COUNT;

  // unsigned long millisEnd = millis();

  // SERIAL_DEBUG(adcRead);
  // SERIAL_DEBUG(millisEnd - millisStart);

  return (adcRead * CURRENT_MEAS_MULTIPLIER1 / CURRENT_MEAS_DIVIDER * CURRENT_MEAS_MULTIPLIER2) << CURRENT_MEAS_DECIMAL_PART;
}

/* Return the load current */
unsigned int getInstantLoadCurrent()
{
  return _instantLoadCurrent >> CURRENT_MEAS_DECIMAL_PART;
}

/* Return the approximate temperature in degree Celsius from the on-board thermistor.
 * Not supported on hardware revision 1.
 */
int readApproxTempDegC()
{
  #if HW_REVISION == 1
    return 25;
  #else
    pinMode(TEMP_MEAS_PIN, INPUT);
    delayMicroseconds(10);

    unsigned long adcRead = 0;
    for (int i = 0; i < ADC_READS_COUNT; i++)
      adcRead += analogRead(TEMP_MEAS_PIN);
    adcRead /= ADC_READS_COUNT;
    
    pinMode(TEMP_MEAS_PIN, OUTPUT);
    digitalWrite(TEMP_MEAS_PIN, LOW); // Avoid thermistor self heating
    
    return approximateTemperatureInt(adcRead);
  #endif
}

/* Return the battery percentage using the average reading.
   At least the low and high voltage breaks are required.

   If the percentage could not be determined, return -1
 */
int getBatteryPercentage()
{
  if (_battVoltageBreaks[0] == 0 || _battVoltageBreaks[6] == 0)
    return -1;

  if (getAverageBatteryVoltage() < _battVoltageBreaks[0])
    return 0;

  if (getAverageBatteryVoltage() >= _battVoltageBreaks[6])
    return 100;

  // Find out the first defined voltage break above 0 and interpolate between break 0 and this break.
  // Then repeat starting with the first break above 0
  uint8_t lowerIdx = 0;
  while (lowerIdx < 6)
  {
    uint8_t upperIdx = lowerIdx + 1;
    while (_battVoltageBreaks[upperIdx] == 0 && upperIdx <= 6)
      upperIdx++;

    unsigned int battVoltage = getAverageBatteryVoltage();
    if (battVoltage >= _battVoltageBreaks[lowerIdx] && battVoltage < _battVoltageBreaks[upperIdx])
      return (lowerIdx * 100 / 6) + (battVoltage - _battVoltageBreaks[lowerIdx]) * (upperIdx - lowerIdx) * (100 / 6) / (_battVoltageBreaks[upperIdx] - _battVoltageBreaks[lowerIdx]);

    lowerIdx = upperIdx;
  }

  return -1; //should have returned before !
}

/* Read the battery type selector */
KXKM_STM32_Energy::BatteryType getBatteryTypeSelectorState()
{
  if (!digitalRead(BATT_TYPE_SELECTOR_PINS[0]))
    return KXKM_STM32_Energy::BATTERY_LIPO;

  else if (!digitalRead(BATT_TYPE_SELECTOR_PINS[1]))
    return KXKM_STM32_Energy::BATTERY_LIFE;

  else
    return KXKM_STM32_Energy::BATTERY_CUSTOM;
}

/* Read the calibration value stored in the option byte.
 */
uint16_t readCalibrationValue()
{
  uint16_t ob = (HAL_FLASHEx_OBGetUserData(OB_DATA_ADDRESS_DATA1) << 8) + HAL_FLASHEx_OBGetUserData(OB_DATA_ADDRESS_DATA0);

  if (ob == 0xFFFF) //Unprogrammed
    ob = 24000 * 316 / (316+2700) * 4095 / 3300; // Default value : voltage is sensed through a 31.6k / 270k resistive divider, referenced to 3.3V

  return ob;
}

/* Determine the number of cells based on the battery voltage and the given min and max cell voltages.
 * Return 0 if the voltage doesn't match any number of cells.
 *
 * 2, 3, 4 and 7 cells are supported.
 */
uint8_t findCellCount(unsigned int voltage, unsigned int cellMin, unsigned int cellMax)
{
  for (int i = 2; i <= 7; i++)
  {
    if (i == 5 || i == 6)
      continue;

    if (voltage > i * cellMin && voltage <= i * (cellMax + INITIAL_CELL_VOLTAGE_TOLERANCE))
      return i;
  }

  return 0;
}