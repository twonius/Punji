/*********************************************************************
 This is an example for our nRF52 based Bluefruit LE modules

 Pick one up today in the adafruit shop!

 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 MIT license, check LICENSE for more information
 All text above, and the splash screen below must be included in
 any redistribution
*********************************************************************/
#include <bluefruit.h>
#include "RTClib.h"
#include "Adafruit_VL6180X.h"

uint32_t DeviceID = 1; 

Adafruit_VL6180X vl = Adafruit_VL6180X();


// initialize Data buffer https://helloacm.com/how-do-you-design-a-circular-fifo-buffer-queue-in-c/
#define BUFFER_SIZE 10000
#define ERROR_EMPTY 0
#define ERROR_FULL 0xFF

const int packageSize = 12;
uint8_t buffer[BUFFER_SIZE][packageSize]; //lenght needs to be 6 to include timestamp
int head = 0, tail = 0;

uint8_t *notification;

//setup real time clock
RTC_PCF8523 rtc;
uint32_t tstamp;
//char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// moving average setup 
const int numReadings = 100;

int readings[numReadings];      // the readings from the analog input
int readIndex = 0;              // the index of the current reading
int total = 0;                  // the running total
int average = 0;                // the average

int inputPin = A1;
int buttonPin = 7;
int sensorEnablePin = 13; 
// boost converter on pin 13 
// battery characteristics (see: https://cdn-learn.adafruit.com/downloads/pdf/adafruit-feather-sense.pdf?timestamp=1597789631)
uint32_t vbat_pin = A6;

#define VBAT_MV_PER_LSB (0.73242188F) // 3.0V ADC range and 12-bit ADC resolution = 3000mV/4096

#define VBAT_DIVIDER (0.5F) // 150K + 150K voltage divider on VBAT
#define VBAT_DIVIDER_COMP (2.0F) // Compensation factor for the VBAT divider

#define REAL_VBAT_MV_PER_LSB (VBAT_DIVIDER_COMP * VBAT_MV_PER_LSB)

/* HRM Service Definitions
 * Heart Rate Monitor Service:  0x180D
 * Heart Rate Measurement Char: 0x2A37
 * Body Sensor Location Char:   0x2A38
 * Battery Service: 0x180F
 * Battery Level Char: 0x2A19
 */
BLEService        wsms = BLEService(0x181D); // weight scale
BLECharacteristic wsfc = BLECharacteristic(0x2A9E);  //weight scale feature
BLECharacteristic wmc = BLECharacteristic(0x2A9D); // weight measurement



BLEDis bledis;    // DIS (Device Information Service) helper class instance
BLEBas blebas;    // BAS (Battery Service) helper class instance

// BLE Client Current Time Service
BLEClientCts  bleCTime;


uint16_t  weight = 70;

void setup()  //************************************************************************************************************************
{
  Serial.begin(115200);

  // button and LED for advertising trigger after timeout. Timeout was enabled to save battery when not connected 
  pinMode(LED_BLUE, OUTPUT);
  pinMode(7, INPUT);
  pinMode(sensorEnablePin, OUTPUT);
  
  
  analogReadResolution(14); // Can be 8, 10, 12 or 14
  readVBAT();

  setupRTC(); 
  

  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = 0;
  }

  bleCTime.begin();

  Serial.println("Bluefruit52 Weight Scale Example");
  Serial.println("-----------------------\n");

  // Initialise the Bluefruit module
  Serial.println("Initialise the Bluefruit nRF52 module");
  Bluefruit.begin();

  // Set the advertised device name (keep it short!)
  Serial.println("Setting Device Name to 'Spot WS'");
  Bluefruit.setName("Spot WS");

  // Set the connect/disconnect callback handlers
  Bluefruit.Periph.setConnectCallback(connect_callback);
  Bluefruit.Periph.setDisconnectCallback(disconnect_callback);

  // Configure and Start the Device Information Service
  Serial.println("Configuring the Device Information Service");
  bledis.setManufacturer("Adafruit Industries");
  bledis.setModel("Spot WS");
  bledis.begin();

  // Start the BLE Battery Service and set it to 100%
  Serial.println("Configuring the Battery Service");
  blebas.begin();
  blebas.write(100);

  // Setup the Heart Rate Monitor service using
  // BLEService and BLECharacteristic classes
  Serial.println("Configuring the Weight Scale Service");
  setupHRM();

  // Setup the advertising packet(s)
  Serial.println("Setting up the advertising payload(s)");
  startAdv();

  Serial.println("Ready Player One!!!");
  Serial.println("\nAdvertising");

  vl.begin();
     
  delay(5000); // BAD PROGRAMMING, should use callback to say when notifications are ready 
}

void startAdv(void)
{
  // Advertising packet
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();

  // Include WS Service UUID
  Bluefruit.Advertising.addService(wsms);

  // include CT Service UUID
  Bluefruit.Advertising.addService(bleCTime);

  
  // include Battery Service UUID
  Bluefruit.Advertising.addService(blebas);

  // Include Name
  Bluefruit.Advertising.addName();

  
  /* Start Advertising
   * - Enable auto advertising if disconnected
   * - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
   * - Timeout for fast mode is 30 seconds
   * - Start(timeout) with timeout = 0 will advertise forever (until connected)
   * 
   * For recommended advertising interval
   * https://developer.apple.com/library/content/qa/qa1931/_index.html   
   */
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(120);                // 0 = Don't stop advertising after n seconds  
}

void setupRTC(void){
//  #ifndef ESP8266
//#endif

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    abort();
  }

  if (! rtc.initialized() || rtc.lostPower()) {
    Serial.println("RTC is NOT initialized, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
    //
    // Note: allow 2 seconds after inserting battery or applying external power
    // without battery before calling adjust(). This gives the PCF8523's
    // crystal oscillator time to stabilize. If you call adjust() very quickly
    // after the RTC is powered, lostPower() may still return true.
  }

  // When time needs to be re-set on a previously configured device, the
  // following line sets the RTC to the date & time this sketch was compiled
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  // This line sets the RTC with an explicit date & time, for example to set
  // January 21, 2014 at 3am you would call:
  // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));

  // When the RTC was stopped and stays connected to the battery, it has
  // to be restarted by clearing the STOP bit. Let's do this to ensure
  // the RTC is running.
  rtc.start();
  
}

void setupHRM(void)
{
  // Configure the Weight Scale service
  // See: https://www.bluetooth.com/xml-viewer/?src=https://www.bluetooth.com/wp-content/uploads/Sitecore-Media-Library/Gatt/Xml/Services/org.bluetooth.service.weight_scale.xml
  // Supported Characteristics:
  // Name                         UUID    Requirement Properties
  // ---------------------------- ------  ----------- ----------
  // Weight Scale Feature         0x2A37  Mandatory   Read
  // Weight Measurement           0x2A38  Mandatory   Notify

  wsms.begin();

  // Note: You must call .begin() on the BLEService before calling .begin() on
  // any characteristic(s) within that service definition.. Calling .begin() on
  // a BLECharacteristic will cause it to be added to the last BLEService that
  // was 'begin()'ed!

  // Configure the Weight Scale Feautres characteristic
  // See:https://www.bluetooth.com/xml-viewer/?src=https://www.bluetooth.com/wp-content/uploads/Sitecore-Media-Library/Gatt/Xml/Characteristics/org.bluetooth.characteristic.weight_scale_feature.xml
  // Properties = READ
  // Min Len    = 1
  // Max Len    = 8
  //    B0      = UINT8  - Flag (MANDATORY)
  //      b7:9  = Height Measurement Resolution (3 bits)
  //      b3:6  = Weight Measurement Resulution (4 bits)
  //      b2    = BMI Supported (t/f)
  //      b1    = Multiple Users Supported (t/f)
  //      b0    = Time Stamp Supported (t/f)
 
  wsfc.setProperties(CHR_PROPS_READ);
  wsfc.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
  wsfc.setFixedLen(1);
  wsfc.setCccdWriteCallback(cccd_callback);  // Optionally capture CCCD updates
  wsfc.begin();
  uint8_t weightfeaturedata[2] = { 0b00000001, 0x40 }; // Set the characteristic to use SI units, with time stamp present
  wsfc.write(weightfeaturedata, 2);
  

  // Configure the Weight Measurement characteristic
  // See: https://www.bluetooth.com/xml-viewer/?src=https://www.bluetooth.com/wp-content/uploads/Sitecore-Media-Library/Gatt/Xml/Characteristics/org.bluetooth.characteristic.weight_measurement.xml
  // Properties = Notify
  // Min Len    = 1
  // Max Len    = 8
  //    B0      = UINT8  - Flag (MANDATORY)
  //      b4:7  = Reserved
  //      b3    = BMI and height present ( 0 = false, 1 = true) 
  //      b2    = User ID Present (0 = false, 1  = true)
  //      b1    = Time stamp present (0 = false, 1 = true) 
  //      b0    = measurement units (0 = SI, 1 = Imperial)
  //    B1:2    = UINT16 - 16-bit Weight - Si
  //    B3:4    = UINT16 - 16-bit Weight - Imperial
  //    B5      = Time Stamp (?)
  //    B6:7    = Unit16 - BMI
  //    B8:9    = Unit16 - Height SI
  //    B10:11    = Unit16 - Height Imperial
  wmc.setProperties(CHR_PROPS_NOTIFY);
  wmc.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
  wmc.setFixedLen(packageSize);
  wmc.setCccdWriteCallback(cccd_callback);  // Optionally capture CCCD updates
  wmc.begin();
  uint8_t weightdata[2] = { 0b00001110, 0x40 }; // Set the characteristic to use SI units, with time stamp present
  wmc.write(weightdata, 2);


}

void connect_callback(uint16_t conn_handle)
{
  // Get the reference to current connection
  BLEConnection* connection = Bluefruit.Connection(conn_handle);

  char central_name[32] = { 0 };
  connection->getPeerName(central_name, sizeof(central_name));

  Serial.print("Connected to ");
  Serial.println(central_name);

  Serial.print("Discovering CTS ... ");
  if ( bleCTime.discover(conn_handle) )
  {
    Serial.println("Discovered");
    
    // iOS requires pairing to work, it makes sense to request security here as well
    Serial.print("Attempting to PAIR with the iOS device, please press PAIR on your phone ... ");
    if ( Bluefruit.requestPairing(conn_handle) )
    {
      Serial.println("Done");
      Serial.println("Enabling Time Adjust Notify");
      bleCTime.enableAdjust();

      Serial.print("Get Current Time chars value");
      bleCTime.getCurrentTime();

      Serial.print("Get Local Time Info chars value");
      bleCTime.getLocalTimeInfo();

      Serial.println();
    }
  }

}

/**
 * Callback invoked when a connection is dropped
 * @param conn_handle connection where this event happens
 * @param reason is a BLE_HCI_STATUS_CODE which can be found in ble_hci.h
 */
void disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
  (void) conn_handle;
  (void) reason;

  Serial.print("Disconnected, reason = 0x"); Serial.println(reason, HEX);
  Serial.println("Advertising!");
}

void cccd_callback(uint16_t conn_hdl, BLECharacteristic* chr, uint16_t cccd_value)
{
    // Display the raw request packet
    Serial.print("CCCD Updated: ");
    //Serial.printBuffer(request->data, request->len);
    Serial.print(cccd_value);
    Serial.println("");

    // Check the characteristic this CCCD update is associated with in case
    // this handler is used for multiple CCCD records.
    if (chr->uuid == wmc.uuid) {
        if (chr->notifyEnabled(conn_hdl)) {
            Serial.println("Weight Measurement 'Notify' enabled");
        } else {
            Serial.println("Weight Measurement 'Notify' disabled");
        }
    }
}

void cts_adjust_callback(uint8_t reason)
{
  const char * reason_str[] = { "Manual", "External Reference", "Change of Time Zone", "Change of DST" };

  Serial.print("iOS Device time changed due to ");
  Serial.println( reason_str[reason] );
}


float readVBAT(void) {
  float raw;
  // Set the analog reference to 3.0V (default = 3.6V)
  analogReference(AR_INTERNAL_3_0);
  // Set the resolution to 12-bit (0..4095)
  analogReadResolution(12); // Can be 8, 10, 12 or 14
  // Let the ADC settle
  delay(1);
  // Get the raw 12-bit, 0..3000mV ADC value
  raw = analogRead(vbat_pin);
  // Set the ADC back to the default settings
  analogReference(AR_DEFAULT);
  analogReadResolution(14);
  // Convert the raw value to compensated mv, taking the resistor-
  // divider into account (providing the actual LIPO voltage)
  // ADC range is 0..3000mV and resolution is 12-bit (0..4095)
  return raw * REAL_VBAT_MV_PER_LSB;
}

uint8_t mvToPercent(float mvolts) {
  if(mvolts<3300)
    return 0;
  if(mvolts <3600) {
    mvolts -= 3300;
    return mvolts/30;
  }
  mvolts -= 3600;
    return 10 + (mvolts * 0.15F ); // thats mvolts /6.66666666
}

// reads a byte from the buffer and return 
void fifoWrite(u_int8_t *val) {
   int i; 
   
   if (head + 1 == tail) ;
    head = (head + 1) % BUFFER_SIZE;
   for(i = 0; i < packageSize ; i++){
      buffer[head][i]= *(val+i);
   }
}

u_int8_t * fifoRead() {
   static u_int8_t row[packageSize]; 
   int i; 
   
   if (head == tail);
   tail = (tail + 1) % BUFFER_SIZE;

   for(i=0; i<packageSize;i++){ 
      row[i] = buffer[tail][i];
    };
   
   return row;
}

bool bufferEmpty(){
   if(head==tail){
     return true;
     Serial.println("Buffer Empty");
     }
     else{return false;}
}

int bufferLength(){
  return head-tail;
}

void printArray(uint8_t *ptr, size_t length)           
{         
    //for statement to print values using array             
    size_t i = 0;
    for( ; i < length; ++i )      
    printf("%d", ptr[i]);        
}  

void loop()
{



    // moving average code 
      for (int i = 0; i <= numReadings; i++) {
          // subtract the last reading:
        total = total - readings[readIndex];
          // read from the sensor:
        readings[readIndex] = vl.readRange();
          // add the reading to the total:
        total = total + readings[readIndex];
          // advance to the next position in the array:
        readIndex = readIndex + 1;

         // if we're at the end of the array...
            if (readIndex >= numReadings) {
          // ...wrap around to the beginning:
              readIndex = 0;
        }
        delay(20);
      }
      // calculate the average:
      weight = total *100 / numReadings;

      //get unix timestamp 
      DateTime now = rtc.now();
      tstamp = now.unixtime();
      Serial.print("Unix Timestamp: ");
      Serial.println(tstamp,HEX);


      // update battery status

      float vbat_mv = readVBAT();

      // Convert from raw mv to percentage (based on LIPO chemistry)
       uint8_t vbat_per = mvToPercent(vbat_mv);
    
      //build package
      uint8_t packet[packageSize]  = {0b00000010,DeviceID,highByte(weight), lowByte(weight),tstamp >> 24, tstamp >> 16, tstamp >>8, tstamp,vbat_per};

      //write package to buffer
      fifoWrite(packet); //write values to the buffer
  
  
  Serial.print("Buffer Length: "); 
  Serial.println(bufferLength());  
 

  digitalWrite(sensorEnablePin,LOW); // turn off the sensor  

  
  if ( Bluefruit.connected() ) {

      
      bleCTime.getCurrentTime();
      bleCTime.getLocalTimeInfo();
      Serial.printf(" %02d:%02d:%02d\n", bleCTime.Time.hour, bleCTime.Time.minute, bleCTime.Time.second);

    
    
    //Serial.print("hrmdata: "); Serial.print(hrmdata[0]);Serial.print(" ");Serial.print(hrmdata[1]);Serial.print(" ");Serial.println(hrmdata[2]);
    // Note: We use .notify instead of .write!
    // If it is connected but CCCD is not enabled
    // The characteristic's value is still updated although notification is not sent
    
    if(wmc.notifyEnabled()){
      while(!bufferEmpty()){
        int i=0;
        
        notification = fifoRead();
        Serial.printf("buffer length: %u \n", bufferLength()); 
        
        Serial.print("Notification: ");
        for(i=0;i<packageSize;i++){
        Serial.print(*(notification+i),HEX);
        }
        Serial.println("");
        

        if ( wmc.notify(notification, packageSize) ){
          Serial.println("Weight Measurement updated"); 
        }else{
          Serial.println("ERROR: Notify not set in the CCCD or not connected!");
        }
      
      };
    }
 
  

    blebas.write(vbat_per);
  
  }else{
      // read the state of the pushbutton value:
      bool buttonState = digitalRead(buttonPin);
      Serial.println("Disconnected");
      // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
      if (buttonState == LOW) {
        startAdv();
        Serial.println("Adv Requested"); 
        digitalWrite(LED_BLUE,HIGH);
        delay(500);
        digitalWrite(LED_BLUE,LOW);
      } 
    }


  // Only send update once per 10 seconds
  // this is where better sleep logic would come in

  delay(5000);
}
