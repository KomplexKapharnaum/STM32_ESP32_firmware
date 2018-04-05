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

const unsigned int ADC_READS_COUNT = 32; // Averaging readings to improve resolution

unsigned int _battVoltageBreaks[7];

/* Initialize battery monitoring resources and determine the type and voltage of
 the attached battery.

 Return : true if it is safe to continue operation, false if the board should not be
 powered.
 */
bool initBatteryMonitoring()
{
  analogReadResolution(12);

  unsigned int initialBattVoltage;

  for (int i = 0; i < 5; i++)
  {
    delay(100);
    initialBattVoltage = readBatteryVoltage();
    SERIAL_DEBUG(initialBattVoltage);
  }

  switch (getBatteryTypeSelectorState())
  {
    case 0: //LiPo
    {
      uint8_t cells = findCellCount(initialBattVoltage, LIPO_VOLTAGE_BREAKS[0], LIPO_VOLTAGE_BREAKS[6]);

      // SERIAL_DEBUG("lipo");
      // SERIAL_DEBUG(cells);
      //TODO if cells=0 return false ?

      for (int i = 0; i < 7; i++)
        _battVoltageBreaks[i] = cells * LIPO_VOLTAGE_BREAKS[i];

      break;
    }

    case 1: //LiFe
    {
      uint8_t cells = findCellCount(initialBattVoltage, LIFE_VOLTAGE_BREAKS[0], LIFE_VOLTAGE_BREAKS[6]);

      // SERIAL_DEBUG("life");
      // SERIAL_DEBUG(cells);
      //TODO if cells=0 return false ?

      for (int i = 0; i < 7; i++)
        _battVoltageBreaks[i] = cells * LIFE_VOLTAGE_BREAKS[i];

      break;
    }
    case 2: //Custom. Battery monitoring disabled or will be set later.
    default:
      // SERIAL_DEBUG("custom");
      for (int i = 0; i < 7; i++)
        _battVoltageBreaks[i] = 0;
      break;
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

  SERIAL_DEBUG(adcRead);
  // SERIAL_DEBUG(millisEnd - millisStart);

  //TODO calibration values
  return adcRead * (316 + 2700) / 316 * 3300 / 4095; //Voltage is sensed through a 31.6k / 270k resistive divider, referenced to 3.3V
}

/* Return the battery percentage. If the percentage could not be determined, return -1
 */
int readBatteryPercentage()
{
  if (_battVoltageBreaks[0] == 0)
    return -1;

  unsigned int voltage = readBatteryVoltage();

  if (voltage < _battVoltageBreaks[0])
    return 0;

  if (voltage >= _battVoltageBreaks[6])
    return 100;

  for (int i = 0; i < 6; i++)
  {
    if (voltage >= _battVoltageBreaks[i] && voltage < _battVoltageBreaks[i+1])
    {
      return (i * 100 / 6) + (voltage - _battVoltageBreaks[i]) * (100 / 6) / (_battVoltageBreaks[i+1] - _battVoltageBreaks[i]);
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
