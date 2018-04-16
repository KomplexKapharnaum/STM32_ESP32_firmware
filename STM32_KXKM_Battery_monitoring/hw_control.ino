/* Hardware control functions (power state of different board components) */

/* 3.3V regulator enable. If set to false, the whole board will be shutdown (
including this microcontroller) */
void set3V3RegState(bool state)
{
  digitalWrite(POWER_ENABLE_PIN, state);
}

/* Main out load switch */
void setLoadSwitchState(bool state)
{
  digitalWrite(MAIN_OUT_ENABLE_PIN, state);
}

/* ESP32 enable */
void setESP32State(bool state)
{
  digitalWrite(ESP32_ENABLE_PIN, state);
}

/* Full shutdown sequence */
void shutdown()
{
  //TODO handle critical section here ?
  setLoadSwitchState(false);
  setESP32State(false);
  set3V3RegState(false); //Bye bye !
}
