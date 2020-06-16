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

/* HRM Service Definitions
 * Heart Rate Monitor Service:  0x180D
 * Heart Rate Measurement Char: 0x2A37
 * Body Sensor Location Char:   0x2A38
 */
BLEService        wsms = BLEService(0x181D); // weight scale
BLECharacteristic wsfc = BLECharacteristic(0x2A9E);  //weight scale feature
BLECharacteristic wmc = BLECharacteristic(0x2A9D); // weight measurement

BLEDis bledis;    // DIS (Device Information Service) helper class instance
BLEBas blebas;    // BAS (Battery Service) helper class instance

uint16_t  weight = 70;

void setup()
{
  Serial.begin(115200);
  while ( !Serial ) delay(10);   // for nrf52840 with native usb

  Serial.println("Bluefruit52 Weight Scale Example");
  Serial.println("-----------------------\n");

  // Initialise the Bluefruit module
  Serial.println("Initialise the Bluefruit nRF52 module");
  Bluefruit.begin();

  // Set the advertised device name (keep it short!)
  Serial.println("Setting Device Name to 'Feather52 WS'");
  Bluefruit.setName("Bluefruit52 WS");

  // Set the connect/disconnect callback handlers
  Bluefruit.Periph.setConnectCallback(connect_callback);
  Bluefruit.Periph.setDisconnectCallback(disconnect_callback);

  // Configure and Start the Device Information Service
  Serial.println("Configuring the Device Information Service");
  bledis.setManufacturer("Adafruit Industries");
  bledis.setModel("Bluefruit Feather52");
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
}

void startAdv(void)
{
  // Advertising packet
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();

  // Include HRM Service UUID
  Bluefruit.Advertising.addService(wsms);

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
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds  
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
  wsfc.setFixedLen(2);
  wsfc.setCccdWriteCallback(cccd_callback);  // Optionally capture CCCD updates
  wsfc.begin();
  uint8_t weightfeaturedata[2] = { 0b00000000, 0x40 }; // Set the characteristic to use SI units, with time stamp present
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
  wmc.setFixedLen(2);
  wmc.setCccdWriteCallback(cccd_callback);  // Optionally capture CCCD updates
  wmc.begin();
  uint8_t weightdata[2] = { 0b00000000, 0x40 }; // Set the characteristic to use SI units, with time stamp present
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
            Serial.println("Heart Rate Measurement 'Notify' enabled");
        } else {
            Serial.println("Heart Rate Measurement 'Notify' disabled");
        }
    }
}

void loop()
{
  digitalToggle(LED_RED);
  
  if ( Bluefruit.connected() ) {
    uint8_t hrmdata[2] = { 0b00000000,weight++};           // Sensor connected, increment BPS value
    
    // Note: We use .notify instead of .write!
    // If it is connected but CCCD is not enabled
    // The characteristic's value is still updated although notification is not sent
    if ( wmc.notify(hrmdata, sizeof(hrmdata)) ){
      Serial.print("Weight Measurement updated to: "); Serial.println(weight); 
    }else{
      Serial.println("ERROR: Notify not set in the CCCD or not connected!");
    }
  }

  // Only send update once per second
  delay(1000);
}
