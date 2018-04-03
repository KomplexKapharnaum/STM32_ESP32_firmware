/*
 * A demo app that calculates the ammount of time taken by the
 * AceButton::check() method. It acts like a stopwatch, supporting the following
 * operations:
 *
 * - press: start the stopwatch
 * - press: stop the stopwatch, printing out the result
 * - long press: reset the stopwatch, allowing press to start the process again
 *
 * Each 'long press' alternates between enableAllEvents() and disableAllEvents()
 * so that we can compare the timing of check() when all events are off (except
 * Pressed and Released) and when all events are on.
 */

#include <AceButton.h>
#include <AdjustableButtonConfig.h>

using namespace ace_button;

// The pin number attached to the button.
const uint8_t BUTTON_PIN = 2;

// Use an instance of AdjustableButtonConfig for demo purposes. We could have
// just used the default System ButtonConfig since a long press delay of 1000 ms
// is good enough. But this shows how to configure an AceButton to use a
// different ButtonConfig.
AdjustableButtonConfig adjustableButtonConfig;

// One button wired to the pin at BUTTON_PIN. Automatically uses the System
// ButtonConfig. The alternative is to call the AceButton::init() method in
// setup() below.
AceButton button(BUTTON_PIN);

// counters to determine the duration of a single call to AceButton::check()
uint16_t innerLoopCounter = 0;
uint16_t outerLoopCounter = 0;
unsigned long startMillis = 0;
unsigned long stopMillis  = 0;

// states of the stopwatch
const uint8_t STOPWATCH_INIT = 0;
const uint8_t STOPWATCH_STARTED = 1;
const uint8_t STOPWATCH_STOPPED = 2;

// implements a finite state machine (FSM)
uint8_t stopwatchState = STOPWATCH_INIT;

bool allEventsEnabled = false;

void setup() {
  Serial.begin(9600);

  // Button uses the built-in pull up register.
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Override the System ButtonConfig with our own AdjustableButtonConfig
  // instance instead.
  button.setButtonConfig(&adjustableButtonConfig);

  // Configure the ButtonConfig with the event handler and enable LongPress.
  // SupressAfterLongPress is optional since we don't do anything if we get it.
  adjustableButtonConfig.setEventHandler(handleEvent);
  adjustableButtonConfig.setFeature(ButtonConfig::kFeatureLongPress);
  adjustableButtonConfig.setLongPressDelay(2000);

  while (! Serial); // Wait until Serial is ready - Leonardo
  Serial.println(F("setup(): stopwatch ready"));
}

void loop() {
  // We split the loop into an inner loop and an outer loop. The inner loop
  // allows us to measure the speed of button.check() without the overhead of
  // the outer loop. However, we must allow the outer loop() method to return
  // periodically to allow the microcontroller to its own stuff. This is
  // especially true on an ESP8266 board, where a Watch Dog Timer will
  // soft-reset the board if loop() doesn't return every few seconds.
  do {
    // button.check() Should be called every 20ms or faster for the default
    // debouncing time of ~50ms.
    button.check();

    // increment loop counter
    if (stopwatchState == STOPWATCH_STARTED) {
      innerLoopCounter++;
    }
  } while (innerLoopCounter);

  // Each time the innerLoopCounter rolls over (65536), increment the outer loop
  // counter, and return from loop(), to prevent WDT errors on ESP8266.
  outerLoopCounter++;
}

// The event handler for the button.
void handleEvent(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  unsigned long now = millis();
  switch (eventType) {
    case AceButton::kEventPressed:
      if (stopwatchState == STOPWATCH_INIT) {

        // enable or disable higher level events, to get different performance
        // numbers
        if (allEventsEnabled) {
          enableAllEvents();
        } else {
          disableAllEvents();
        }

        Serial.println(F("handleEvent(): stopwatch started"));
        startMillis = now;
        innerLoopCounter = 0;
        outerLoopCounter = 0;
        stopwatchState = STOPWATCH_STARTED;
      } else if (stopwatchState == STOPWATCH_STARTED) {
        stopMillis = now;
        stopwatchState = STOPWATCH_STOPPED;
        unsigned long duration = stopMillis - startMillis;
        uint32_t loopCounter = ((uint32_t) outerLoopCounter << 16) +
            innerLoopCounter;
        float microsPerLoop = duration * 1000.0f / loopCounter;

        // reenable all events after stopping
        enableAllEvents();

        Serial.println(F("handleEvent(): stopwatch stopped"));
        Serial.print(F("handleEvent(): duration (ms): "));
        Serial.print(duration);
        Serial.print(F("; loopCount: "));
        Serial.print(loopCounter);
        Serial.print(F("; micros/loop: "));
        Serial.println(microsPerLoop);

        // Setting 0 allows the loop() function to return periodically.
        innerLoopCounter = 0;
      }
      break;
    case AceButton::kEventLongPressed:
      if (stopwatchState == STOPWATCH_STOPPED) {
        stopwatchState = STOPWATCH_INIT;
        Serial.println(F("handleEvent(): stopwatch reset"));
        allEventsEnabled = !allEventsEnabled;
      }
      break;
  }
}

void enableAllEvents() {
  Serial.println(F("enabling high level events"));
  adjustableButtonConfig.setFeature(ButtonConfig::kFeatureClick);
  adjustableButtonConfig.setFeature(ButtonConfig::kFeatureDoubleClick);
  adjustableButtonConfig.setFeature(ButtonConfig::kFeatureLongPress);
  adjustableButtonConfig.setFeature(ButtonConfig::kFeatureRepeatPress);
}

void disableAllEvents() {
  Serial.println(F("disabling high level events"));
  adjustableButtonConfig.clearFeature(ButtonConfig::kFeatureClick);
  adjustableButtonConfig.clearFeature(ButtonConfig::kFeatureDoubleClick);
  adjustableButtonConfig.clearFeature(ButtonConfig::kFeatureLongPress);
  adjustableButtonConfig.clearFeature(ButtonConfig::kFeatureRepeatPress);
}
