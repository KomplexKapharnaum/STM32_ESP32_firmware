/* Serial communication functions.
 *
 * Used for debug and communication with the ESP32.
 *
 * The STM32 and the programmation connector use the same serial port on the ESP32
 * so the TX line must be released (switched to High-Z) after each communication to
 * leave the programmation connector available.
 */

void beginSerial()
{
  Serial1.begin(115200);
}

void endSerial()
{
  Serial1.flush();
  pinMode(ESP32_TX_PIN, INPUT); //Set TX pin to Hi Z to allow ESP32 programmation from external connector
}
