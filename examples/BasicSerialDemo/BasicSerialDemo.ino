/*
  PicoButtonAsync - Basic Serial Demo
  
  This sketch demonstrates how to detect various button states using 
  the PicoButtonAsync library without blocking the main loop.
  
  Circuit:
  - Connect a momentary push button between GPIO 14 and GND.
  - No external resistor is needed (internal pull-up is used).
*/

#include <PicoButtonAsync.h>

// 1. Initialize the DebounceManager with a 5ms sampling interval.
// This runs in the background using the Pico SDK async_context.
DebounceManager debouncer(5);

// 2. Define a button on GPIO 14.
PicoButton myButton(debouncer, 14);

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);  // Wait for Serial monitor

  Serial.println("--- PicoButtonAsync Demo ---");
  Serial.println("Button configured on GPIO 14 (Connect to GND)");

  // 3. Start the background sampling engine.
  // Note: Calling this in setup() pins the interrupt to Core 0.
  debouncer.begin();
}

void loop() {
  // justPressed() returns true only at the moment of impact.
  if (myButton.justPressed()) {
    Serial.println("[EVENT] Button Pressed");
  }

  // justLongPressed() triggers once after being held for 1 second (1000ms).
  if (myButton.justLongPressed(1000)) {
    Serial.println("[EVENT] Long Press Detected (1.0s)");
  }

  // repeat() triggers every 100ms after the button is held for 500ms.
  // Useful for scrolling menus or volume control.
  if (myButton.repeat(500, 100)) {
    Serial.println("[REPEAT] ...");
  }

  // justReleased() returns true only when the finger is lifted.
  if (myButton.justReleased()) {
    Serial.println("[EVENT] Button Released");
    Serial.println("-------------------------");
  }

  // You can add other code here and it won't miss button edges.
}
