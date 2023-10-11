function showContainer(containerId) {
	// Hide all containers
	document.getElementById("homeContainer").style.display = "none";
	document.getElementById("sensorsContainer").style.display = "none";
	document.getElementById("wifiContainer").style.display = "none";

	// Show the selected container
	document.getElementById(containerId + "Container").style.display = "block";
}

function showEditForm() {
	document.getElementById("editForm").style.display = "block";
	document.getElementById("connectedStatus").innerText = "Configuring"; // Clear previous status
	document.getElementById("ssidStatus").innerText = "Waiting..."; // Clear previous status
}

function connectToWiFi() {
	document.getElementById("editForm").style.display = "none";

	const ssid = document.getElementById("ssid").value;
	const password = document.getElementById("password").value;

	document.getElementById("ssidStatus").innerText = ssid; // Clear previous status
	document.getElementById("connectedStatus").innerText = "Connecting.."; // Clear previous status

	const postData = { ssid: ssid, password: password };
	const postUrl = "/config"; // Replace with your server endpoint
	// const postUrl = "http://192.168.2.40/config"; // Replace with your server endpoint

	function handleResponse(error, response) {
		if (error) {
			console.error("Error:", error);
		} else {
			console.log("Response:", response);
		}
	}

	sendPostRequest(postUrl, postData, handleResponse);

	showContainer("wifi");
}

function sendPostRequest(url, data, callback) {
	const xhr = new XMLHttpRequest();

	xhr.open("POST", url, true);

	xhr.onreadystatechange = function () {
		if (xhr.readyState == XMLHttpRequest.DONE) {
			if (xhr.status === 200) {
				// Request was successful
				const response = JSON.parse(xhr.responseText);
				callback(null, response);
			} else {
				// Request failed
				callback(`POST request failed with status ${xhr.status}`, null);
			}
		}
	};

	// Prepare the data to be sent
	const jsonData = JSON.stringify(data);

	// Send the request with the data
	xhr.setRequestHeader("Content-type", "application/json");
	xhr.send(jsonData);

	console.log("POST:", url, jsonData);
}

function updateGauge() {
	document.gauges.forEach(function (gauge) {
		gauge.value = gauge.value + 10;
		// gauge.value =
		// 	Math.random() * (gauge.options.maxValue - gauge.options.minValue) +
		// 	gauge.options.minValue;
	});
}
