var dataVar={"zone": 0,"state":"na"};
var obj = { zone: 0, estado: 0 };
var networkIP;

var os = require('os');
var ifaces = os.networkInterfaces();

Object.keys(ifaces).forEach(function (ifname) {
  var alias = 0;


  ifaces[ifname].forEach(function (iface) {
    
  
    if ('IPv4' !== iface.family || iface.internal !== false) {
      // skip over internal (i.e. 127.0.0.1) and non-ipv4 addresses
      return;
    }

    if (alias >= 1) {
      // this single interface has multiple ipv4 addresses
      console.log(ifname + ':' + alias, iface.address);
    } else {
      // this interface has only one ipv4 adress
      console.log(ifname, iface.address);
      networkIP = ifname + ' ' + iface.address;
    }
    ++alias;
  });
});


var http = require('http');
http.createServer(function (request, response) {
  response.writeHead(200, {'Content-Type': 'application/json'});
  var jsonData = 
  response.end(JSON.stringify(dataVar));
}).listen(8124);
console.log('Server running at ' + networkIP);


var net = require('net');
var server = net.createServer(function(c) { //'connection' listener
  console.log('client connected');
  c.on('end', function() {
    console.log('client disconnected');
  });
  c.on('data', function (data) {
      console.log(data);
      //dataVar = data.toString('utf-8');
	  dataVar = JSON.parse(data);
	  console.log(JSON.stringify(dataVar));
	  console.log(dataVar.zone);
	  console.log(dataVar.state);
  });
    c.on('error',(err) =>console.log("Caught an error"))
});

server.listen(8125, function() { //'listening' listener
  console.log('server bound');
});