/*
  PicoButtonAsync - Multicore Isolation Demo
  
  This sketch demonstrates how to isolate button handling to Core 1.
  This ensures that even if Core 0 is 100% busy with heavy calculations,
  the buttons remain perfectly responsive.
  
  Note: This requires the Earle Philhower RP2040/RP2350 Core.
*/

#include <PicoButtonAsync.h>

// Initialize manager
DebounceManager debouncer(5);
PicoButton btn(debouncer, 14);

// --- CORE 1: UI & INPUT TASK ---
// In the Philhower core, setup1() and loop1() run on the second core.
void setup1() {
  // By calling begin() here, the background ISR is pinned to Core 1.
  debouncer.begin();
}

void loop1() {
  // All button logic happens here, isolated from Core 0.
  if (btn.justPressed()) {
    Serial.println("[Core 1] Button Pressed - Loop 0 is still busy!");
  }
  
  if (btn.justReleased()) {
    Serial.println("[Core 1] Button Released");
  }
  
  delay(10); // Core 1 sleeps for a small amount of time
}

// --- CORE 0: HEAVY COMPUTATION ---
void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("Starting heavy math on Core 0...");
}

void loop() {
  // Simulate a heavy, blocking task on Core 0
  // In a normal library, this would make buttons feel "broken" or laggy.
  double result = 0;
  for (int i = 0; i < 1000000; i++) {
    result += sin(i) * cos(i);
  }
  
  // Briefly report Core 0 is alive
  Serial.print("."); 
  delay(100); 
}
