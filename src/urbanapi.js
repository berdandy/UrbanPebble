var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function udRandom() {
  // get a random pile of definitions. Returns 10, currently.
  var url = 'http://api.urbandictionary.com/v0/random';
  xhrRequest(url, 'GET', 
    function(responseText) {
      var json = JSON.parse(responseText);
		console.log('UD says: ' + json);
		
		// Assemble dictionary using our keys
// var dictionary = {
// 	'KEY_WORD: word,
// 	'KEY_DEFINITION': definition,
// 	'KEY_EXAMPLE': example,
// };

// // Send to Pebble
// Pebble.sendAppMessage(dictionary,
//   function(e) {
//     console.log('Weather info sent to Pebble successfully!');
//   },
//   function(e) {
//     console.log('Error sending weather info to Pebble!');
//   }
// );
    }      
  );
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log('PebbleKit JS ready!');
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log('AppMessage received!');
  }                     
);