let offset = 0;

function fetchOffset() {
    fetch("/data?type=offset")
        .then((response) => response.text())
        .then((data) => {
            document.getElementById("offset").innerText = data;

            offset = data;
        })
        .catch((error) => console.error("Err:", error));
}

let lastValue = -1;

function map(x, in_min, in_max, out_min, out_max) {
    return ((x - in_min) * (out_max - out_min)) / (in_max - in_min) + out_min;
}

let value;

let maxkPa = 0;

// 1
// let multiplier = 0.00153;

// let mapInMin = 3000,
//     mapOutMin = 3000;

// let mapInMax = 4000,
//     mapOutMax = 4000;

// 2
// let multiplier = 0.00139;

// let mapInMin = 3000,
//     mapOutMin = 3000;

// let mapInMax = 4000,
//     mapOutMax = 4000;

// 3
// let multiplier = 0.001445;

// let mapInMin = 3000,
//     mapOutMin = 3000;

// let mapInMax = 4000,
//     mapOutMax = 4000;

let Par = 0,
    Pa = 0,
    kPa = 0;

function calculateMultiplier(real) {
    return multiplier * (kPa / real);
}

function fetchValue() {
    fetch("/data?type=value")
        .then((response) => response.text())
        .then((data) => {
            document.getElementById("raw").innerText = data;
            document.getElementById("value").innerText = data - offset;

            value = data;

            Par = (data - offset) * multiplier;

            document.getElementById("cal").innerText = Par.toFixed(2);

            Pa = map(Par, mapInMin, mapInMax, mapOutMin, mapOutMax);

            if (lastValue != -1) {
                const speed = (Pa - lastValue) * 60;

                const kPa = speed < 0 ? 0 : speed / 1000.0;

                document.getElementById("speed").innerText = kPa.toFixed(2);
            }

            lastValue = Pa;

            kPa = Pa / 1000.0;

            if (kPa >= maxkPa) {
                maxkPa = kPa;

                document.getElementById("max").innerText = maxkPa.toFixed(2);
            }

            const alarm = Number(document.getElementById("alarm").value);

            if (alarm > 0) {
                if (kPa > alarm) {
                    if (document.getElementById("control-buzzer").checked)
                        fetch("/output/buzzer?value=1");
                    if (document.getElementById("control-pump").checked)
                        fetch("/output/pump?value=0");
                } else {
                    if (document.getElementById("control-buzzer").checked)
                        fetch("/output/buzzer?value=0");
                    if (document.getElementById("control-pump").checked)
                        fetch("/output/pump?value=1");
                }
            }

            document.getElementById("kpa").innerText = kPa.toFixed(4);
        })
        .catch((error) => console.error("Err:", error));
}

function onLoad() {
    fetchOffset();

    setInterval(() => {
        fetchValue();
    }, 1000);

    document
        .getElementById("enable-pump")
        .addEventListener("click", function (e) {
            e.preventDefault();
            fetch("/output/pump?value=1");
        });

    document
        .getElementById("disable-pump")
        .addEventListener("click", function (e) {
            e.preventDefault();
            fetch("/output/pump?value=0");
        });

    document
        .getElementById("enable-buzzer")
        .addEventListener("click", function (e) {
            e.preventDefault();
            fetch("/output/buzzer?value=1");
        });

    document
        .getElementById("disable-buzzer")
        .addEventListener("click", function (e) {
            e.preventDefault();
            fetch("/output/buzzer?value=0");
        });
}

document.addEventListener("DOMContentLoaded", onLoad);
