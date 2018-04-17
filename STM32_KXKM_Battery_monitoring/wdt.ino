/* Watchdog timer functions */


IWDG_HandleTypeDef IwdgHandle;

/* Init watchdog with a 250ms period */
void initWatchdog()
{
  IwdgHandle.Instance = IWDG;

  /* Set counter reload value to obtain 250ms IWDG TimeOut.
     LSI Frequency : around 40kHz
     IWDG counter clock Frequency = LsiFreq / 32 => ~ 1250Hz
     Counter Reload Value = 250ms / IWDG counter clock period
                          = 0.25s / (32/LsiFreq)
                          = LsiFreq / (32 * 4)
                          = LsiFreq / 128 */
  IwdgHandle.Init.Prescaler = IWDG_PRESCALER_32;
  //IwdgHandle.Init.Reload    = uwLsiFreq/128;
  IwdgHandle.Init.Reload    = 312; //40000 / 128
  IwdgHandle.Init.Window = IWDG_WINDOW_DISABLE;

  HAL_IWDG_Init(&IwdgHandle);
}

void refreshWatchdog()
{
  HAL_IWDG_Refresh(&IwdgHandle);
}
