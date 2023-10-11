var timers = [];

getWiFiStatus();

timers.push(
	setInterval(function () {
		getWiFiStatus();
		updateGauge();
	}, 5000)
);

function showContainer(containerId) {
	// Hide all containers
	document.getElementById("homeContainer").style.display = "none";
	document.getElementById("sensorsContainer").style.display = "none";
	document.getElementById("wifiContainer").style.display = "none";

	// Show the selected container
	document.getElementById(containerId + "Container").style.display = "block";
}

function showEditForm() {
	// Show form now
	document.getElementById("wifi_update_form").style.display = "block";

	updateValueWithId("wifi_ssid", "Not Set");
	updateValueWithId("wifi_status", "Configuring...");
	updateValueWithId("wifi_signal", "Not found");
}

function connectToWiFi() {
	// Hide form now
	document.getElementById("wifi_update_form").style.display = "none";

	const ssid = document.getElementById("ssid").value;
	const password = document.getElementById("password").value;

	updateValueWithId("wifi_ssid", ssid);
	updateValueWithId("wifi_status", "Connecting...");

	const postData = { ssid: ssid, password: password };
	const postUrl = "/config";

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

function updateValueWithId(id, value) {
	document.getElementById(`${id}`).innerText = value;
}

function getWiFiStatus() {
	const apiUrl = "/config?ssid=?&status=?&rssi=?";

	function handleResponse(error, response) {
		if (error) {
			console.error("Error:", error);
		} else {
			console.log("SSID:", response.report.ssid);
			console.log("STATUS:", response.report.status);
			console.log("RSSI:", response.report.rssi);

			updateValueWithId("wifi_ssid", response.report.ssid);
			updateValueWithId("wifi_status", response.report.status);
			updateValueWithId("wifi_signal", response.report.rssi);
		}
	}

	sendGetRequest(apiUrl, null, handleResponse);
}

function sendGetRequest(url, queryParams, callback) {
	const xhr = new XMLHttpRequest();

	// Append query parameters to the URL
	if (queryParams) {
		const params = new URLSearchParams(queryParams);
		url += "?" + params.toString();
	}

	xhr.open("GET", url, true);

	xhr.onreadystatechange = function () {
		if (xhr.readyState == XMLHttpRequest.DONE) {
			if (xhr.status === 200) {
				// Request was successful
				const response = JSON.parse(xhr.responseText);
				callback(null, response);
			} else {
				// Request failed
				callback(`GET request failed with status ${xhr.status}`, null);
			}
		}
	};

	// Send the request
	xhr.send();

	console.log("Sent GET", url);
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

	console.log("Sent POST:", url, jsonData);
}

function updateGauge(value) {
	var gauge = document.gauges.get("radial-one");
	gauge.value = gauge.value + 140;
	console.log("New Value for Gauge: ", gauge.value);
}
