
var deviceName = 'Spot WS'
var wsService = 'weight_scale'
var wsCharacteristic = 'weight_measurement'
var battService = 'battery_service'
var battCharacteristic = 'battery_level'
var bluetoothDeviceDetected
var gattCharacteristic
var weightCharacteristic
var batteryCharacteristic
var battStatus = 999

var weightData = new Array()

// const ws = new WebSocket('ws://localhost:9898/');
// ws.onopen = function() {
//     console.log('WebSocket Client Connected');
// };
// ws.onmessage = function(e) {
//   //console.log("Received: '" + e.data + "'");
// };

var myHeaders = new Headers();
myHeaders.append("Content-Type", "application/json");





const monthNames = ["Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"];

document.querySelector('#read').addEventListener('click', function() {
  if (isWebBluetoothEnabled()) { read() }
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
    bluetoothDeviceDetected.addEventListener('gattserverdisconnected', onDisconnected);
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


function onDisconnected(){
  console.log('disconnected, retrying')
  read();

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
    batteryCharacteristic.addEventListener('characteristicvaluechanged',handleBatteryNotifications);

    start();

}catch(error) {
    console.log('Argh! ' + error);
  }
}



function handleBatteryNotifications(event){

  let level = event.target.value;
  battStatus = level.getUint8();
  // console.log('> Battery Level is ' + battStatus + '%');


}


//swapped in code from https://googlechrome.github.io/samples/web-bluetooth/notifications.html
function handleNotifications(event) {

  //console.log('notification Received')
  let value = event.target.value;

  // Convert raw data bytes to hex values just for the sake of showing something.
  // In the "real" world, you'd use data.getUint8, data.getUint16 or even
  // TextDecoder to process raw data bytes.
  var setup = value.getUint8();
  var weightReading_low = value.getUint8(1);
  var weightReading_high = value.getUint8(2);
  var y2 = value.getUint8(3);
  var y1 = value.getUint8(4);
  var month = value.getUint8(5);
  var day = value.getUint8(6);
  var hour = value.getUint8(7);
  var minute = value.getUint8(8);
  var second = value.getUint8(9);
  var userID = value.getUint8(10);

  var weightInput = document.getElementById("weightInput");
  console.log("weightInput: "+ weightInput.getValue)

  //console.log(timeConverter(unix_timestamp));
  weightReading = ((weightReading_high & 0xFF) << 8) | (weightReading_low & 0xFF);
  year = ((y1 & 0xFF)<< 8 ) | (y2 & 0xFF);
  // build array to plot
  weightData.push(weightReading);

  msg = {
    userID: userID, // update based on login info
    weight: weightReading,
    timestamp: (monthNames[month-1] + " " + String(day) + " " + String(year) + " , " + String(hour) + ":" + String(minute) + ":" + String(second)),
    battery: battStatus,
    weightInput: string(weightInput.getValue)
  }
  //ws.send(JSON.stringify(msg)); //send over websocket


  // Post requst using fetch


  var raw = JSON.stringify(msg);

  var requestOptions = {
    method: 'POST',
    headers: myHeaders,
    body: raw,
    redirect: 'follow'
  };

  fetch("https://punjii.herokuapp.com/data", requestOptions)
    .then(response => response.text())
    .then(result => console.log(result))
    .catch(error => console.log('error', error));



  var weightDisp = document.getElementById("weightDisplay");
  var battDisp = document.getElementById("batteryLevel");
  var devDisp = document.getElementById("deviceID");

  var unit = "";
  var weightReading_str = weightReading.toString();
  if (battStatus == 999){
      var battReading_str = '- ';
  }else{
    var battReading_str = battStatus.toString();
  };

  var device_str = userID.toString();

  // console.log('batt: '+ battReading_str);
  //console.log('weight: ' + weightReading_str);
  weightDisp.textContent = weightReading_str.concat(unit);
  battDisp.textContent = ('Battery: ' + battReading_str.concat("%"));
  devDisp.textContent = ('Device ID: ' + device_str);



}


function start() {
  batteryCharacteristic.startNotifications()
  .then(_ => {
    console.log('Start reading...')

  })
  .catch(error => {
    console.log('[ERROR] Start: ' + error)
  });


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
