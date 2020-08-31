#include <Oversampling.h>

void setup()
{
  Serial.begin(19200);
  Serial.print("OverSampled");
  Serial.print("\t");
  Serial.println("Filtered");
  pinMode(13, OUTPUT);
  analogReadResolution(14); // Can be 8, 10, 12 or 14

 
}

void loop()
{
  int sensorValue = analogRead(A0);
 
  Serial.println(sensorValue);

  delay(100);
}
