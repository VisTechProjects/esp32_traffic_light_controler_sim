var ws = new WebSocket('ws://' + window.location.hostname + '/ws');

ws.onmessage = function (event) {
    var data = JSON.parse(event.data);
    updateTrafficLight(data.state);
};

ws.onopen = function () {
    console.log("Connected to WebSocket server");
};

// Handle WebSocket errors
ws.onerror = function (error) {
    console.error("WebSocket Error:", error);
};

function updateTrafficLight(state) {
    document.getElementById('traffic-light').src = '/images/trfc_lt_' + state + '.png';
}

// Function to fetch current delay values from ESP32
function fetchDelays() {
    fetch('/get_delays')
        .then(response => response.json())
        .then(data => {
            document.querySelector('input[name="red_delay"]').value = data.red / 1000;  // Convert ms to seconds
            document.querySelector('input[name="yellow_delay"]').value = data.yellow / 1000;
            document.querySelector('input[name="green_delay"]').value = data.green / 1000;
        })
        .catch(error => console.error("Error fetching delay values:", error));
}

// Open popup and load current delay values
function openPopup() {
    fetchDelays();  // Fetch latest values
    document.getElementById("popup").style.display = "block";
    document.getElementById("overlay").style.display = "block";
}

function openPopup_time_waisted() {
    alert('Why did I waste my time making this')
}

function toggleMode() {
    fetch('/toggle_mode')
        .then(response => response.text())
        .then(data => {
            var toggleModeSwitch = document.getElementById('toggleModeSwitch');
            var toggleModeLabel = document.getElementById('toggleModeLabel');
            toggleModeLabel.textContent = toggleModeSwitch.checked ? "Blink Mode" : "Cycle Mode";
        });
}

function toggleCatMode() {
    fetch('/toggle_cat_mode')
        .then(response => response.text())
        .then(data => {
            var toggleCatModeSwitch = document.getElementById('toggleCatModeSwitch');
            var toggleCatModeLabel = document.getElementById('toggleCatModeLabel');
            toggleCatModeLabel.textContent = toggleCatModeSwitch.checked ? "Cat Mode" : "Normal Mode";
        });
}

document.getElementById('blinkColorSelect').addEventListener('change', function () {
    var color = this.value;
    fetch('/blink_mode?color=' + color)
        .then(response => response.text())
        .then(data => {
            var toggleModeSwitch = document.getElementById('toggleModeSwitch');
            if (!toggleModeSwitch.checked) {
                toggleModeSwitch.checked = true;
                document.getElementById('toggleModeLabel').textContent = "Blink Mode";
            }
        });
});

// Light Cycle Delay Form - Prevent Page Reload & Show Popup
document.getElementById('setDelayForm').addEventListener('submit', function (event) {
    event.preventDefault(); // Prevent form from redirecting

    let formData = new FormData(this);
    let params = new URLSearchParams(formData).toString();

    fetch('/set_delay?' + params, { method: 'GET' })
        .then(response => response.text())
        .then(data => {
            // alert("Light cycle delay updated successfully!"); // Popup confirmation
            closePopup(); // Close the popup after submission
        })
        .catch(error => {
            console.error('Error:', error);
            alert("Failed to update light cycle delay.");
        });
});

// Popup Functions
function openPopup() {
    document.getElementById("popup").style.display = "block";
    document.getElementById("overlay").style.display = "block";
}

function closePopup(event) {
    if (event) event.preventDefault(); // Prevent button's default behavior
    document.getElementById("popup").style.display = "none";
    document.getElementById("overlay").style.display = "none";
}