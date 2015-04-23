// Function to send a message to the Pebble using AppMessage API
function sendMessage()
{
	Pebble.sendAppMessage({"status": 0});
	
	// PRO TIP: If you are sending more than one message, or a complex set of messages, 
	// it is important that you setup an ackHandler and a nackHandler and call 
	// Pebble.sendAppMessage({ /* Message here */ }, ackHandler, nackHandler), which 
	// will designate the ackHandler and nackHandler that will be called upon the Pebble 
	// ack-ing or nack-ing the message you just sent. The specified nackHandler will 
	// also be called if your message send attempt times out.
}


// Called when JS is ready
Pebble.addEventListener("ready",
function(e)
{
	console.log("ready");

	try
	{
		var settings = JSON.parse(localStorage.getItem("settings"));
		console.log("Settings: " + localStorage.getItem("settings"));

		Pebble.sendAppMessage(settings);
	}
	catch(err)
	{
		console.log("No JSON response or received Cancel event");
	}	
}
);
												
// Called when incoming message from the Pebble is received
Pebble.addEventListener("appmessage",
function(e)
{
	var feed = e.payload.feed;
	var channel = e.payload.channel;
	var chartWidth = e.payload.chartWidth.toFixed(1);
	var chartHeight = e.payload.chartHeight.toFixed(1);
	
	if (chartWidth === undefined)
		chartHeight = 32.0;
	
	if (chartHeight === undefined)
		chartHeight = 32.0;

	requestFeed(feed, channel, chartWidth, chartHeight);
}
);

Pebble.addEventListener('showConfiguration', function(e)
{
	var url = 'http://www.cbrunner.at/pebble.html?' + encodeURIComponent(localStorage.getItem("settings"));  
	console.log("showConfiguration: " + url);
	Pebble.openURL(url);
}
);

Pebble.addEventListener('webviewclosed',
function(e)
{
	
	try
	{
		var settings = JSON.parse(decodeURIComponent(e.response));
		localStorage.clear();
		localStorage.setItem("settings", JSON.stringify(settings));
		console.log("Settings: " + localStorage.getItem("settings"));
		
		Pebble.sendAppMessage(settings);
	}
	catch(err)
	{
		console.log("No JSON response or received Cancel event");
	}	
}
);

function requestFeed(feed, channel, chartWidth, chartHeight)
{
	var settings = JSON.parse(localStorage.getItem("settings"));

	var req = new XMLHttpRequest();
	var url = 'https://api.xively.com/v2/feeds/' + feed + '/datastreams/' + channel + '.json?interval=' + settings.interval + '&duration=' + settings.duration + '&key=' + settings.apiKey;

	req.open('GET', url, true);
	req.onload = function(e)
	{
		if (req.readyState == 4 && req.status == 200)
		{
			if(req.status == 200)
			{
				var response = JSON.parse(req.responseText);
				
				var rawData = new Array();
				
				for (var idx = response.datapoints.length - 1; idx >= 0  && rawData.length < chartWidth; idx--)
				{
					rawData.push(parseFloat(response.datapoints[idx].value));
					// console.log(response.datapoints[idx].value + " @ " + response.datapoints[idx].at);
				}
				
				var chart = "";
				var value = response.current_value;
				var valueMin = Math.min.apply(null, rawData);
				var valueMax = Math.max.apply(null, rawData);
				var step = (valueMax - valueMin) / (chartHeight - 1.0);
			
				for (idx = rawData.length - 1; idx >= 0 ; idx--)
				{
					chart += String.fromCharCode(Math.round((rawData[idx] - valueMin) / step) + 65);
					// console.log(rawData[idx] + " @ " + Math.round((rawData[idx] - minValue) / step));
				}

				console.log("feed: " + feed + ", channel: " + channel + ", value: " + parseFloat(value).toFixed(1) + ", valueMin: " + parseFloat(valueMin).toFixed(1) + ", valueMax: " + parseFloat(valueMax).toFixed(1));
				Pebble.sendAppMessage({ 'feed':feed, 'channel':channel, 'value':parseFloat(value).toFixed(1), 'valueMin':parseFloat(valueMin).toFixed(1), 'valueMax':parseFloat(valueMax).toFixed(1), 'data':chart });
			}
			else
			{
				console.log('Error');
				Pebble.sendAppMessage({ 'feed':feed, 'channel':channel, 'value':'N/A', 'valueMax':'error', 'valueMin':'error', 'data':'' });
			}
		}
		else
		{
			console.log('Error');
			Pebble.sendAppMessage({ 'feed':feed, 'channel':channel, 'value':'N/A', 'valueMax':'error', 'valueMin':'error', 'data':'' });
		}
			
	};
	req.send(null);
}