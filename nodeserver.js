var express = require('express');
var path = require('path');
var favicon = require('serve-favicon');
var logger = require('morgan');
var cookieParser = require('cookie-parser');
var bodyParser = require('body-parser');
var request = require('request');
var routes = require('./routes/index');
var users = require('./routes/users');
var app = express();
​
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: false }));
app.use(cookieParser());
app.use(express.static(path.join(__dirname, 'public')));
​
app.post('/test', function(req,res) {
  var request = require("request");
  console.log('request recieved');
​
var options = { method: 'POST',
  url: 'https://api-http.littlebitscloud.cc/devices/00e04c034330/output',
  headers:
   { 'postman-token': '4f6dacf6-e7f3-250c-0e79-368cd0675752',
     'cache-control': 'no-cache',
     'content-type': 'application/json',
     authorization: 'Bearer ad8d35ff5c8eed4eab00c79fb0ddea8f0de3a99da2df8391f566d445011288a0' },
      body: { percent: '100', duration_ms: '15000' },
      json: true };
​
    request(options, function (error, response, body) {
      if (error) throw new Error(error);
​
      console.log(body);
    });
});
​
​
​
app.listen(process.env.PORT || 1234);
console.log('app listening on port 1234')
​
module.exports = app;