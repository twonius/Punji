// Preliminary pass at getting GP2Y0E03 working.
// It appears to work but i haven’t the slightest idea how.
// The same command of Wire.read() is assumed to get two different sets of data.</p>
#include <Wire.h>
#define SENSOR_ADRS 0x40 // I2C address of GP2Y0E03
#define DISTANCE_ADRS 0x5E // Data address of Distance Value // Functions to process only at power-on and reset
void setup ()
{
// Initialize serial communication
Wire.begin(); // Initialize I2C,
delay(1000); // start after 1 second
Serial.begin(9600);
// Main processing function repeatedly executed
 pinMode(5, OUTPUT); // GPIO Pin must be high to read 
 pinMode(6, OUTPUT); // VIn (IO)  Pin must be high to read 

}
void loop ()
{
int ans ;
byte c[2];
digitalWrite(5, HIGH); //turn on module 
digitalWrite(6, HIGH); //turn on module
Wire.beginTransmission (SENSOR_ADRS); // start communication processing
Wire.write (DISTANCE_ADRS); // specify the address of the table storing the distance value
ans = Wire.endTransmission(); // send and close data
delay(200);
if(ans == 0){
  ans = Wire.requestFrom(SENSOR_ADRS, 2) ;
  c[0] = Wire.read(); // Read the 11th to 4th bits of data c [1]
  c[1] = Wire.read(); // Read the 3rd and 0th bits of the data
  ans = ((c [0] * 16 + c [1]) / 16) / 4; // distance
  Serial.print(ans);
  Serial.println ("cm"); //to display on serial monitor
  }
else{
  Serial.print("ERROR NO. ="); // Can not communicate with GP2Y0E03
  Serial.println(ans);
}
delay (200); // repeat after 200 ms
}
