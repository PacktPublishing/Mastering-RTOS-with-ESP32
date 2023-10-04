// function reqListener() {
// 	console.log(this.responseText);
// }

// const req = new XMLHttpRequest();
// req.addEventListener("load", reqListener);
// req.open("GET", "/bootstrap/scripts.js");
// req.send();

if (!Array.prototype.forEach) {
	Array.prototype.forEach = function (cb) {
		var i = 0,
			s = this.length;
		for (; i < s; i++) {
			cb && cb(this[i], i, this);
		}
	};
}

document.fonts &&
	document.fonts.forEach(function (font) {
		font.loaded.then(function () {
			if (font.family.match(/Led/)) {
				document.gauges.forEach(function (gauge) {
					gauge.update();
					gauge.options.renderTo.style.visibility = "visible";
				});
			}
		});
	});

var timers = [];

function animateGauges() {
	document.gauges.forEach(function (gauge) {
		timers.push(
			setInterval(function () {
				gauge.value =
					Math.random() *
						(gauge.options.maxValue - gauge.options.minValue) +
					gauge.options.minValue;
			}, gauge.animation.duration + 50)
		);
	});
}

function updateGauge() {
	document.gauges.forEach(function (gauge) {
		gauge.value = gauge.value + 10;
		// gauge.value =
		// 	Math.random() * (gauge.options.maxValue - gauge.options.minValue) +
		// 	gauge.options.minValue;
	});
}
