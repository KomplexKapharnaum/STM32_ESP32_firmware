// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ADC Value to Temperature for NTC Thermistor.
// Author: James Sleeman http://sparks.gogo.co.nz/ntc.html
// Licence: BSD (see footer for legalese)
//
// Thermistor characteristics:
//   Nominal Resistance 10000 at 25°C
//   Beta Value 3380
//
// Usage Examples:
//   float bestAccuracyTemperature    = convertAnalogToTemperature(analogRead(analogPin));
//   float lesserAccuracyTemperature  = approximateTemperatureFloat(analogRead(analogPin));
//   int   lowestAccuracyTemperature  = approximateTemperatureInt(analogRead(analogPin));
//
// Better accuracy = more resource (memory, flash) demands, the approximation methods 
// will only produce reasonable results in the range -5-40°C
//
//
// Thermistor Wiring:
//   Vcc -> [10000 Ohm Resistor] -> Thermistor -> Gnd
//                             |
//                             \-> analogPin
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

/** Calculate the temperature in °C from ADC (analogRead) value (best accuracy).
 *
 *  This conversion should generate reasonably accurate results over the entire range of 
 *  the thermistor, it implements the common 'Beta' approximation for a thermistor
 *  having Beta of 3380, and nominal values of 10000Ω at 25°C
 *
 *  @param   The result of an ADC conversion (analogRead) in the range 0 to 4095
 *  @return  Temperature in °C
 */

float  convertAnalogToTemperature(unsigned int analogReadValue)
{
  // If analogReadValue is 4095, we would otherwise cause a Divide-By-Zero,
  // Treat as crazy out-of-range temperature.
  if(analogReadValue == 4095) return 1000.0; 
                                               
  return (1/((log(((10000.0 * analogReadValue) / (4095.0 - analogReadValue))/10000.0)/3380.0) + (1 / (273.15 + 25.000)))) - 273.15;
}




/** Approximate the temperature in °C from ADC (analogRead) value, using floating-point math.
 *
 *  This approximation uses floating point math, but much less complex so may be useful for 
 *  improved performance, reducing program memory consumption, and reducing runtime memory
 *  usage.
 *
 *  This conversion has the following caveats...
 *    Suitable Range              : -5°C to 40°C 
 *    Average Error (Within Range): +/- 0.198 °C°C
 *    Maximum Error (Within Range): 0.639 °C°C
 *
 *  This approximation implements a linear regression of the Beta approximation 
 *  for a thermistor having Beta of 3380, and nominal values of 10000Ω at 
 *  25°C calculated for temperatures across the range above.
 *
 * @param   The result of an ADC conversion (analogRead) in the range 0 to 4095
 * @return  Temperature in °C (+/- 0.639 °C)
 */

float  approximateTemperatureFloat(unsigned int analogReadValue)
{
  return -0.0259503655562173*analogReadValue+78.4162992015644;
}

/** Approximate the temperature in °C from ADC (analogRead) value, using integer math.
 *
 *  This approximation uses only integer math, so has a subsequent resolution of
 *  of only 1°C, but for very small microcontrollers this is useful as floating point
 *  math eats your program memory.
 *
 *  This conversion has the following caveats...
 *    Suitable Range              : -5°C to 40°C 
 *    Average Error (Within Range): +/- 0.386 °C°C
 *    Maximum Error (Within Range): 1.100 °C°C
 *
 *  This approximation implements a linear regression of the Beta approximation 
 *  for a thermistor having Beta of 3380, and nominal values of 10000Ω at 
 *  25°C calculated for temperatures across the range above.
 *
 * @param   The result of an ADC conversion (analogRead) in the range 0 to 4095
 * @return  Temperature in °C (+/- 1.100 °C)
 */

int approximateTemperatureInt(unsigned int analogReadValue)
{
  return ((((((long)analogReadValue*3) / -116)) + 78)) - 0;
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Legal Mumbo Jumo Follows, in short: do whatever you want, just don't sue me.
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Copyright © 2019 James Sleeman. All Rights Reserved.
// 
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions are met:
// 
// 1. Redistributions of source code must retain the above copyright notice,
//  this list of conditions and the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright notice,
//  this list of conditions and the following disclaimer in the documentation 
//  and/or other materials provided with the distribution.
// 
// 3. The name of the author may not be used to endorse or promote products 
//  derived from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY James Sleeman AS IS AND ANY EXPRESS OR IMPLIED
//  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
//  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
//  EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
//  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED 
//  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
//  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"											
