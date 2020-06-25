

var deviceName = 'Bluefruit52 WS'
var bleService = 'weight_scale'
var bleCharacteristic = 'weight_measurement'
var bluetoothDeviceDetected
var gattCharacteristic

const ws = new WebSocket('ws://localhost:9898/');
ws.onopen = function() {
    console.log('WebSocket Client Connected');
    ws.send('Hi this is web client.');
};
ws.onmessage = function(e) {
  console.log("Received: '" + e.data + "'");
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
    optionalServices: [bleService],
    filters: [
      { "name": deviceName }
    ]
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
  .then(connectGATT)
  .then(_ => {
    console.log('Reading Weight...')
    return gattCharacteristic.startNotifications()
  })
  .catch(error => {
    console.log('Waiting to start reading: ' + error)
  })
}

function connectGATT() {
  if (bluetoothDeviceDetected.gatt.connected && gattCharacteristic) {
    return Promise.resolve()
  }

  return bluetoothDeviceDetected.gatt.connect()
  .then(server => {
    console.log('Getting GATT Service...')
    return server.getPrimaryService(bleService)
    console.log(bleservice)
  })
  .then(service => {
    console.log('Getting GATT Characteristic...')
    return service.getCharacteristic(bleCharacteristic)
    console.log(bleCharacteristic)
  })
  .then(characteristic => {
    gattCharacteristic = characteristic
    gattCharacteristic.addEventListener('characteristicvaluechanged',
        handleNotifications)
    document.querySelector('#start').disabled = false
    document.querySelector('#stop').disabled = true
  })
}

// function handleChangedValue(event) {
//   let value = event.target.value.getUint16(2)
//   var now = new Date()
//   console.log('> ' + now.getHours() + ':' + now.getMinutes() + ':' + now.getSeconds() + ' Heart Rate is ' + value)
// }

//swapped in code from https://googlechrome.github.io/samples/web-bluetooth/notifications.html
function handleNotifications(event) {
let value = event.target.value;
let a = [];
// Convert raw data bytes to hex values just for the sake of showing something.
// In the "real" world, you'd use data.getUint8, data.getUint16 or even
// TextDecoder to process raw data bytes.
var weightReading = value.getUint16(1)
console.log(weightReading);
var weightDisp = document.getElementById("weightDisplay")

weightDisp.textContent = weightReading

}
function start() {
  gattCharacteristic.startNotifications()
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
  gattCharacteristic.stopNotifications()
  .then(_ => {
    console.log('Stop reading...')
    document.querySelector('#start').disabled = false
    document.querySelector('#stop').disabled = true
  })
  .catch(error => {
    console.log('[ERROR] Stop: ' + error)
  })
}