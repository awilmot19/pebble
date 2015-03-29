//matchmaking.js
var register = '/register';
var unregister = '/unregister';
var findOpponent = '/findOpponent';
var give_pos = '/give_pos';
var get_pos = '/get_pos';
var baseUrl = 'http://whispering-hollows-6492.herokuapp.com:80/pong';
var player_num;
var opponent_num = -1;

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log('Received message: ' + JSON.stringify(e.payload));
  }
);


var xhrRequest = function (url, type, data, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  
  xhr.open(type, url, false);
  xhr.setRequestHeader('Content-type', 'application/json');
  xhr.send(JSON.stringify(data));
};

function locationSuccess(pos) {
  // Construct URL
  var url = baseUrl + register;
  
  xhrRequest(url, 'POST', { "lat":pos.coords.latitude, "lng":pos.coords.longitude },
    function(responseText) {
      console.log("responseText:" + responseText);
      
      // responseText contains a JSON object
      var json = JSON.parse(responseText);
      
      player_num = json.player_id;
      console.log(player_num);
      
      url = baseUrl + findOpponent;
      xhrRequest(url, 'POST', { "lat":pos.coords.latitude, "lng":pos.coords.longitude, "player_id":player_num }, function(newResponseText){
        console.log("findOpp ResponseText:" + newResponseText);
        var json = JSON.parse(newResponseText);
        opponent_num = json.opponent_id;
        console.log("Opponent num is " + opponent_num);
      });
    });
        
    var start = 1;
    if(opponent_num == -1)
      start = 0;
      
    // Assemble dictionary using our keys
    var dictionary = {
      'KEY_START': start,
      'KEY_PLAYER_NUM': player_num,
      'KEY_OPPONENT_NUM': opponent_num
    };

    // Send to Pebble
    Pebble.sendAppMessage(dictionary,
    function(e) {
      console.log('initial data sent successfully!');
    },
    function(e) {
      console.log('Error sending initial info to Pebble!');
    });
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

// Listen for when the app is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log('PebbleKit JS ready!');
    // Find a match
    lookForMatch();
  }
);