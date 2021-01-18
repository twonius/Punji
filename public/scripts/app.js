

var deviceName = 'Spot WS'
var wsService = 'weight_scale'
var wsCharacteristic = 'weight_measurement'
var battService = 'battery_service'
var battCharacteristic = 'battery_level'
var bluetoothDeviceDetected
var gattCharacteristic
var weightCharacteristic
var battCharacteristic
var battStatus = 999

var weightData = new Array()

const ws = new WebSocket('ws://localhost:9898/');
ws.onopen = function() {
    console.log('WebSocket Client Connected');

};
ws.onmessage = function(e) {
  //console.log("Received: '" + e.data + "'");
};


document.querySelector('#read').addEventListener('click', function() {
  if (isWebBluetoothEnabled()) { read() }
})

document.querySelector('#start').addEventListener('click', function(event) {
  if (isWebBluetoothEnabled()) { start() }
})

document.querySelector('#stop').addEventListener('click', function(event) {
  if (isWebBluetoothEnabled()) { stop() }
})




function isWebBluetoothEnabled() {
  if (!navigator.bluetooth) {
    console.log('Web Bluetooth API is not available in this browser!')
    return false
  }

  return true
}

function getDeviceInfo() {
  let options = {

    filters: [
     { "name": deviceName,
     services: ['weight_scale','battery_service']
  }]
  }

  console.log('Requesting any Bluetooth Device...')
  return navigator.bluetooth.requestDevice(options).then(device => {
    bluetoothDeviceDetected = device
  }).catch(error => {
    console.log('Argh! ' + error)
  })
}

function read() {
  return (bluetoothDeviceDetected ? Promise.resolve() : getDeviceInfo())
  .then(connectGATTAsync)
  .then(_ => {
    //console.log('Reading Weight...')
    return weightCharacteristic.startNotifications()
  })
  .catch(error => {
    console.log('Waiting to start reading: ' + error)
  })
}



async function connectGATTAsync() {
  try {


    const server = await bluetoothDeviceDetected.gatt.connect();
    console.log('Connecting to GATT Server...');

    console.log('Getting Battery Service...');
    const batteryService = await server.getPrimaryService(battService);

    console.log('Getting Battery Level Characteristic...');
    batteryCharacteristic = await batteryService.getCharacteristic(battCharacteristic);

    console.log('Getting Weight Service...');
    const weightService = await server.getPrimaryService(wsService);

    console.log('Getting Weight Measurement Characteristic...');
    weightCharacteristic = await weightService.getCharacteristic(wsCharacteristic);



    weightCharacteristic.addEventListener('characteristicvaluechanged',handleNotifications);



}catch(error) {
    console.log('Argh! ' + error);
  }
}

async function readBattery(){
  const value = await batteryCharacteristic.readValue();
  battStatus = value.getUint8(0)
  console.log('> Battery Level is ' + battStatus + '%');

}



//swapped in code from https://googlechrome.github.io/samples/web-bluetooth/notifications.html
function handleNotifications(event) {

  //console.log('notification Received')
  let value = event.target.value;

  // Convert raw data bytes to hex values just for the sake of showing something.
  // In the "real" world, you'd use data.getUint8, data.getUint16 or even
  // TextDecoder to process raw data bytes.
  var setup = value.getUint8()
  var device = value.getUint8(1)
  var weightReading = value.getUint16(2)
  var tstamp = value.getUint32(4)
  var battStatus = value.getUint8(8)

  console.log(tstamp.toString(16))
  //console.log(timeConverter(unix_timestamp));


  // build array to plot
  weightData.push(weightReading);

  msg = {
    deviceID: device, // update based on login info
    weight: weightReading,
    timestamp: tstamp,
    battery: battStatus
  }
  ws.send(JSON.stringify(msg)); //send over websocket

  var weightDisp = document.getElementById("weightDisplay");
  var unit = "";
  var weightReading_str = weightReading.toString();

  //console.log('weight: ' + weightReading_str);
  weightDisp.textContent = weightReading_str.concat(unit);



  //readBattery()

}


function start() {
  weightCharacteristic.startNotifications()
  .then(_ => {
    console.log('Start reading...')
    document.querySelector('#start').disabled = true
    document.querySelector('#stop').disabled = false
  })
  .catch(error => {
    console.log('[ERROR] Start: ' + error)
  })
}

function stop() {
  weightCharacteristic.stopNotifications()
  .then(_ => {
    console.log('Stop reading...')
    document.querySelector('#start').disabled = false
    document.querySelector('#stop').disabled = true
  })
  .catch(error => {
    console.log('[ERROR] Stop: ' + error)
  })
}
