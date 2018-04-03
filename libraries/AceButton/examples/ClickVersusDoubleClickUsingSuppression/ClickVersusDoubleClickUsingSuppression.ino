/*
 * A demo that uses kFeatureSuppressClickBeforeDoubleClick to distinguish a
 * Clicked event from a DoubleClicked event. Click turns on the LED. A
 * DoubleClick turns off the LED.
 *
 * The only way to suppress the Clicked "after" a DoubleClicked is to postpone
 * the sending of the Clicked event until getDoubleClickDelay() time after the
 * Clicked. At that time, we can tell if a DoubleClicked has occurred or not.
 * But this means that every Clicked event is delayed by (kClickDelay +
 * kDoubleClickDelay + 2 * kDebounceDelay) which is 700 ms using the default
 * values, and you'll notice this delay in the LED turning on.
 *
 * The other side-effect is that if a user doesn't input a clean Click (which
 * results in a normal Press/Release sequence), then nothing happens to the LED.
 * Depending on the application, this may or may not be the desirable result.

 * See Also:
 *    examples/ClickVersusDoubleClickUsingReleased/
 *      - uses the Released event instead of the Clicked event
 */

#include <AceButton.h>
using namespace ace_button;

// The pin number attached to the button.
const int BUTTON_PIN = 2;

// LED states. Some microcontrollers wire their built-in LED the reverse.
const int LED_ON = HIGH;
const int LED_OFF = LOW;

// One button wired to the pin at BUTTON_PIN. Automatically uses the default
// ButtonConfig. The alternative is to call the AceButton::init() method in
// setup() below.
AceButton button(BUTTON_PIN);

void setup() {
  // initialize built-in LED as an output
  pinMode(LED_BUILTIN, OUTPUT);

  // Button uses the built-in pull up register.
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  ButtonConfig* buttonConfig = button.getButtonConfig();
  buttonConfig->setEventHandler(handleEvent);
  buttonConfig->setFeature(ButtonConfig::kFeatureDoubleClick);
  buttonConfig->setFeature(
      ButtonConfig::kFeatureSuppressClickBeforeDoubleClick);
}

void loop() {
  // Should be called every 20ms or faster for the default debouncing time
  // of ~50ms.
  button.check();
}

// The event handler for the button.
void handleEvent(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  switch (eventType) {
    case AceButton::kEventClicked:
      digitalWrite(LED_BUILTIN, LED_ON);
      break;
    case AceButton::kEventDoubleClicked:
      digitalWrite(LED_BUILTIN, LED_OFF);
      break;
  }
}
