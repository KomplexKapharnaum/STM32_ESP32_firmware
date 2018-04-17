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

/* All voltages are given in mV */
const unsigned int LIPO_VOLTAGE_BREAKS[] = {3500, 3650, 3700, 3750, 3825, 3950, 4200}; //For one cell
const unsigned int LIFE_VOLTAGE_BREAKS[] = {2920, 3140, 3200, 3220, 3240, 3260, 3600}; //For one cell

const unsigned int ADC_READS_COUNT = 4; // Averaging readings to improve resolution
const unsigned int CALIBRATION_VOLTAGE = 24000; // Voltage used for the calibration

const unsigned long ADC_READ_PERIOD_MS = 5; // ADC read every 10ms

const int BATT_LOW_LEVEL = 10; // Low battery level (%)

//ADC reading : exponential averaging
const unsigned int ADC_OLD_WEIGHT = 97;
const unsigned int ADC_NEW_WEIGHT = 3;

unsigned int _battVoltageBreaks[7];
unsigned int _avgBattVoltage;

/* Initialize battery monitoring resources and determine the type and voltage of
 the attached battery.

 Return : true if it is safe to continue operation, false if the board should not be
 powered.
 */
bool initBatteryMonitoring()
{
  analogReadResolution(12);

  _avgBattVoltage = readBatteryVoltage();

  switch (getBatteryTypeSelectorState())
  {
    case 0: //LiPo
    {
      uint8_t cells = findCellCount(_avgBattVoltage, LIPO_VOLTAGE_BREAKS[0], LIPO_VOLTAGE_BREAKS[6]);

      SERIAL_DEBUG("LiPo");
      SERIAL_DEBUG(cells);

      for (int i = 0; i < 7; i++)
        _battVoltageBreaks[i] = cells * LIPO_VOLTAGE_BREAKS[i];

      if (cells == 0)
        return false; // The selector is on the LiPo / LiFe state but the voltage doesn't match !

      break;
    }

    case 1: //LiFe
    {
      uint8_t cells = findCellCount(_avgBattVoltage, LIFE_VOLTAGE_BREAKS[0], LIFE_VOLTAGE_BREAKS[6]);

      SERIAL_DEBUG("LiFe");
      SERIAL_DEBUG(cells);

      for (int i = 0; i < 7; i++)
        _battVoltageBreaks[i] = cells * LIFE_VOLTAGE_BREAKS[i];

      if (cells == 0)
        return false; // The selector is on the LiPo / LiFe state but the voltage doesn't match !

      break;
    }
    case 2: //Custom. Battery monitoring disabled or will be set later.
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
    _avgBattVoltage = (_avgBattVoltage * ADC_OLD_WEIGHT + readBatteryVoltage() * ADC_NEW_WEIGHT) / (ADC_OLD_WEIGHT + ADC_NEW_WEIGHT);

    // SERIAL_DEBUG(_avgBattVoltage);
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

  return adcRead * CALIBRATION_VOLTAGE / readCalibrationValue();
}

/* Return the battery percentage using the average reading.
   If the percentage could not be determined, return -1
   TODO if not all voltage breaks are present !?
 */
int getBatteryPercentage()
{
  if (_battVoltageBreaks[0] == 0)
    return -1;

  if (_avgBattVoltage < _battVoltageBreaks[0])
    return 0;

  if (_avgBattVoltage >= _battVoltageBreaks[6])
    return 100;

  for (int i = 0; i < 6; i++)
  {
    if (_avgBattVoltage >= _battVoltageBreaks[i] && _avgBattVoltage < _battVoltageBreaks[i+1])
    {
      return (i * 100 / 6) + (_avgBattVoltage - _battVoltageBreaks[i]) * (100 / 6) / (_battVoltageBreaks[i+1] - _battVoltageBreaks[i]);
    }
  }

  return -1; //should have returned before !
}

/* Read the battery type selector */
uint8_t getBatteryTypeSelectorState()
{
  if (!digitalRead(BATT_TYPE_SELECTOR_PINS[0]))
    return 0;

  else if (!digitalRead(BATT_TYPE_SELECTOR_PINS[1]))
    return 1;

  else
    return 2;
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

    if (voltage > i*cellMin && voltage <= i*cellMax)
      return i;
  }

  return 0;
}
