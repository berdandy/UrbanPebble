var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function getRandom() {
  // get a random pile of definitions. Returns 10, currently.
  var url = 'http://api.urbandictionary.com/v0/random';
  xhrRequest(url, 'GET', 
    function(responseText) {
		var data = JSON.parse(responseText);
		
		var term = data.list[0];
		var word = term.word;
		var definition = term.definition;
		var example = term.example;
		
		console.log("GOT FROM UD: " + word + " :: " + definition + " :: " + example);
		
		var returnMessage = {
			'KEY_WORD': word,
			'KEY_DEFINITION': definition,
			'KEY_EXAMPLE': example,
		};
		Pebble.sendAppMessage(returnMessage,
			function(e) {
				console.log('UD term sent to Pebble successfully!');
			},
			function(e) {
				console.log('Error sending UD term to Pebble!');
			}
		);

	}
  );
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log('Fetching first term');
	// getRandom();
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
	function(e) {
		console.log('Refreshing term');
		getRandom();
  }                     
);