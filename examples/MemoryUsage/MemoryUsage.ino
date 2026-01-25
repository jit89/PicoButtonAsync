/*
  PicoButtonAsync - Memory Footprint Utility
  
  Use this sketch to determine the RAM usage of the library 
  classes for your documentation.

  NOTE: Values may vary slightly depending on the compiler
  optimization settings (-Os vs -O3)
*/

#include <PicoButtonAsync.h>

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("--- PicoButtonAsync Memory Profile ---");
  Serial.println();

  // 1. Measure the Manager
  Serial.print("Size of DebounceManager: ");
  Serial.print(sizeof(DebounceManager));
  Serial.println(" bytes");

  // 2. Measure a single Button instance
  Serial.print("Size of PicoButton:      ");
  Serial.print(sizeof(PicoButton));
  Serial.println(" bytes");

  Serial.println();
  Serial.println("Technical Breakdown:");
  Serial.println("- DebounceManager contains the history buffer (32-bit * samples).");
  Serial.println("- PicoButton contains independent state trackers and timing registers.");
  Serial.println("---------------------------------------");
}

void loop() {
  // Nothing to do here
}
