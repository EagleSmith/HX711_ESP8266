var udp = require('dgram');
var ws = require("nodejs-websocket");
var fs = require("fs");


var configuration = {

  //baseline needs to be re-done, bat+adc needs to be re-done
  "192.168.1.191" : {
    name: "3Lobe",
    baseline: 8455000,
    stepsPerKilo: 5000,
    adc: [658,754],
    bat: [376,430]
  },

  //bat+adc needs to be re-done
    "192.168.1.192" : {
    name: "4Lobe",
    baseline: 8322000,
    stepsPerKilo: 10000,
    adc: [658,757],
    bat: [376,420]
  },

  //ALL checked
  "192.168.1.193" : {
    name: "5Lobe",
    baseline: 8380000,
    stepsPerKilo: 10000,
    adc: [658,754],
    bat: [376,430]
  }

}

function mapValue(num, in_min, in_max, out_min, out_max) {
  return (num-in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// websocket server
var ws_server = ws.createServer(function (conn) {
  console.log("New connection")
  conn.on("text", function (str) {
    console.log("Received "+str)
    conn.sendText(str.toUpperCase()+"!!!")
  })
  conn.on("close", function (code, reason) {
    console.log("Connection closed")
  })
  conn.on("error", function(errObj) {
    console.log("Connection Error: "+errObj);
  })
}).listen(8001)

function broadcast(server, msg) {
  server.connections.forEach(function (conn) {
    conn.sendText(msg)
  })
}



// --------------------creating a udp server --------------------

// creating a udp server
var udp_server = udp.createSocket('udp4');

// emits when any error occurs
udp_server.on('error',function(error){
  console.log('Error: ' + error);
  udp_server.close();
});

// emits on new datagram msg
udp_server.on('message',function(msg,info){
  console.log('Data received from client : ' + msg.toString());
  console.log('Received %d bytes from %s:%d\n',msg.length, info.address, info.port);

  //msg is load,adc
  data = msg.toString().split(",");

  loadData = data[0];
  adcData = data[1];

  config = configuration[info.address.split(":")[0]];

  batData = (mapValue(adcData,config.adc[0],config.adc[1],config.bat[0],config.bat[1])/100).toFixed(2);
  loadKgData = ((loadData - config.baseline)/config.stepsPerKilo);

  console.log(loadData);
  console.log(adcData);

  broadcastJsonStr = JSON.stringify({ time: new Date().getTime(), name : config.name, loadRaw: loadData, loadKg : loadKgData, adc : adcData, bat: batData });
  console.log(broadcastJsonStr);
  fs.appendFile('log.txt', broadcastJsonStr+"\r\n", function (err) { console.log(err); } )
  broadcast(ws_server, broadcastJsonStr);
});

//emits when socket is ready and listening for datagram msgs
udp_server.on('listening',function(){
  var address = udp_server.address();
  var port = address.port;
  var family = address.family;
  var ipaddr = address.address;
  console.log('Server is listening at port' + port);
  console.log('Server ip :' + ipaddr);
  console.log('Server is IP4/IP6 : ' + family);
});

//emits after the socket is closed using socket.close();
udp_server.on('close',function(){
  console.log('Socket is closed !');
});

udp_server.bind(2222);


