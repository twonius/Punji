#include "HX711.h"

// HX711 circuit wiring
const int LOADCELL_DOUT_PIN =  PIN_WIRE_SDA;
const int LOADCELL_SCK_PIN = PIN_WIRE_SCL;

HX711 scale;

void setup() {
  Serial.begin(57600);
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
}

void loop() {

  if (scale.is_ready()) {
    long reading = scale.read();
    Serial.print("HX711 reading: ");
    Serial.println(reading);
  } else {
    Serial.println("HX711 not found.");
  }

  delay(1000);
  
}
