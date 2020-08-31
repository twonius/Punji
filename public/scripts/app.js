

var deviceName = 'Spot WS'
var bleService = 'weight_scale'
var bleCharacteristic = 'weight_measurement'
var batteryService = 'battery_service'
var batteryCharacteristic = 'battery_level'
var bluetoothDeviceDetected
var gattCharacteristic
var weightData = new Array()

const ws = new WebSocket('ws://localhost:9898/');
ws.onopen = function() {
    console.log('WebSocket Client Connected');
    ws.send(9999);
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

Plotly.plot('chart',[{
  y:[],
  type:'line'
}]);

function isWebBluetoothEnabled() {
  if (!navigator.bluetooth) {
    console.log('Web Bluetooth API is not available in this browser!')
    return false
  }

  return true
}

function getDeviceInfo() {
  let options = {
    optionalServices: [bleService]
  , filters: [
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

// build array to plot
weightData.push(weightReading);

ws.send(weightReading); //send over websocket

var weightDisp = document.getElementById("weightDisplay");
var unit = " lbs";
var weightReading_str = weightReading.toString();
console.log(weightReading_str.concat(unit));
weightDisp.textContent = weightReading_str.concat(unit);


Plotly.extendTraces('chart', { y: [[weightReading]] }, [0]);

cnt = weightData.length;

if(cnt>500) {
  Plotly.relayout('chart',{
    xaxis: {
    range: [cnt-500,cnt]}
  });
}
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
