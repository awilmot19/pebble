//matchmaking.js

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log('Received message: ' + JSON.stringify(e.payload));
  }
);

var form;

function sendLoc(pos) {
  form.onsubmit = function(e) {
    e.preventDefault();
    
    var locObj = {"loc" : pos.coords.latitude, "lng" : pos.coords.longitude};
    
    var xhr = new XMLHttpRequest();
    xhr.open(form.method, form.action, true);
    xhr.setRequestHeader('Content-Type', 'application/json; charset=UTF-8');
    xhr.send(locObj);
    xhr.onloadend = function() {
    };
  };
}

/*
var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};
*/

function locationSuccess(pos) {
  console.log('Location Success');
  var dictionary = {
    'KEY_START' : true,
    'KEY_OPP_POS': 20
};
  
Pebble.sendAppMessage(dictionary,
  function(e) {
    console.log('test info sent to Pebble successfully!');
  },
  function(e) {
    console.log('Error sending test info to Pebble!');
  }
  );        
}

function locationError(err) {
  console.log('Error requesting location!');
}

function lookForMatch() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 60000}
  );
}

function get_pos(id) {
  
}

// Listen for when the app is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log('PebbleKit JS ready!');
    sendLoc(navigator.geolocation.getCurrentPosition());
    // Find a match
    lookForMatch();
  }
);
