/*
  PicoButtonAsync - Chord & Multi-Button Demo
  
  This sketch demonstrates how to detect "Chords" (multiple buttons 
  pressed at the same time) and how to manage multiple button instances.
  
  Circuit:
  - Button 1: GPIO 14 to GND
  - Button 2: GPIO 15 to GND

  Because the DebounceManager has the full 32-bit state of the GPIOs,
  checking a chord is a single bitwise operation, making it virtually
  free in terms of CPU usage.
*/

#include <PicoButtonAsync.h>

DebounceManager debouncer(5);

PicoButton btnA(debouncer, 14);
PicoButton btnB(debouncer, 15);

// Define a bitmask for the "Chord" (Both pins 14 and 15)
const uint32_t CHORD_MASK = (1UL << 14) | (1UL << 15);

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  Serial.println("--- PicoButtonAsync Chord Demo ---");
  Serial.println("Press A (G14) and B (G15) together for the chord action!");

  debouncer.begin();
}

void loop() {
  // 1. Handle individual button logic
  if (btnA.justPressed()) Serial.println("Button A Pressed");
  if (btnB.justPressed()) Serial.println("Button B Pressed");

  // 2. Handle the Chord (Combination)
  // We check if BOTH pins are currently in the "Pressed" state (low/0)
  if (debouncer.checkChord(CHORD_MASK)) {
    // We use a simple debounce/timer here to prevent spamming the serial port
    static uint32_t lastChordAction = 0;
    if (millis() - lastChordAction > 1000) {
      Serial.println("!!! CHORD DETECTED (A + B Held) !!!");
      lastChordAction = millis();
    }
  }

  // 3. Handle Releases
  if (btnA.justReleased()) Serial.println("Button A Released");
  if (btnB.justReleased()) Serial.println("Button B Released");
}
