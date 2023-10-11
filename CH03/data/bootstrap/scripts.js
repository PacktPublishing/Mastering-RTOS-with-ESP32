var timers = [];

getParams();

timers.push(
	setInterval(function () {
		getParams();
	}, 3000)
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

function getParams() {
	const apiUrl = "/config?ssid=?&status=?&rssi=?&adc_34=?&adc_35=?";

	function handleResponse(error, response) {
		if (error) {
			console.error("Error:", error);
		} else {
			console.log("Report:", response.report);

			if (response.report.ssid) {
				updateValueWithId("wifi_ssid", response.report.ssid);
			}

			if (response.report.status) {
				updateValueWithId("wifi_status", response.report.status);
			}

			if (response.report.rssi) {
				updateValueWithId("wifi_signal", response.report.rssi);
			}

			if (response.report.adc_34) {
				updateGauge1(parseInt(response.report.adc_34));
			} else {
				updateGauge1(0);
			}

			if (response.report.adc_35) {
				updateGauge2(parseInt(response.report.adc_35));
			} else {
				updateGauge2(0);
			}
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

function updateGauge1(value) {
	var gauge = document.gauges.get("radial-one");
	gauge.value = value;
	console.log("New Value for Gauge1: ", gauge.value);
}

function updateGauge2(value) {
	var gauge = document.gauges.get("radial-two");
	gauge.value = value;
	console.log("New Value for Gauge2: ", gauge.value);
}
