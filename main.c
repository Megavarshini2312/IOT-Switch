<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>IOT GANG SWITCH</title>
    <style>
        body, html {
            margin: 0;
            padding: 0;
            height: 100%;
            font-family: Arial, sans-serif;
            display: flex;
            justify-content: center;
            align-items: center;
            background-color: #f0f0f0;
        }

        .container {
            background-color: black;
            padding: 20px;
            border-radius: 15px;
            box-shadow: 0 0 15px rgba(0, 0, 0, 0.5);
            width: 300px;
            text-align: center;
            overflow: hidden;
        }

        h1 {
            color: white;
            font-size: 20px;
            margin-bottom: 20px;
        }

        .switch-wrapper {
            margin-bottom: 15px; /* Spacing between each switch */
        }

        .switch-wrapper div {
            display: flex;
            align-items: center;
            justify-content: center; /* Center switch and label horizontally */
        }

        h3 {
            margin: 0;
            margin-right: 10px; /* Space between text and switch */
            font-size: 16px;
            color: white;
        }

        .switch {
            position: relative;
            display: inline-block;
            width: 50px; /* Adjusted width */
            height: 28px; /* Adjusted height */
        }

        .switch input {
            opacity: 0;
            width: 0;
            height: 0;
        }

        .slider {
            position: absolute;
            cursor: pointer;
            top: 0;
            left: 0;
            right: 0;
            bottom: 0;
            background-color: #ccc;
            transition: .4s;
            border-radius: 34px;
        }

        .slider:before {
            position: absolute;
            content: "";
            height: 22px; /* Adjusted size */
            width: 22px; /* Adjusted size */
            left: 4px;
            bottom: 4px;
            background-color: white;
            transition: .4s;
            border-radius: 50%;
        }

        input:checked + .slider {
            background-color: #2196F3;
        }

        input:checked + .slider:before {
            transform: translateX(22px); /* Adjusted to match new width */
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>IOT GANG SWITCH</h1>

        <div class="switch-wrapper">
            <div>
                <h3>Switch 1:</h3>
                <label class="switch">
                    <input type="checkbox" id="Relay1Value">
                    <span class="slider round"></span>
                </label>
            </div>
        </div>
        <div class="switch-wrapper">
            <div>
                <h3>Switch 2:</h3>
                <label class="switch">
                    <input type="checkbox" id="Relay2Value">
                    <span class="slider round"></span>
                </label>
            </div>
        </div>
        <div class="switch-wrapper">
            <div>
                <h3>Switch 3:</h3>
                <label class="switch">
                    <input type="checkbox" id="Relay3Value">
                    <span class="slider round"></span>
                </label>
            </div>
        </div>
        <div class="switch-wrapper">
            <div>
                <h3>Switch 4:</h3>
                <label class="switch">
                    <input type="checkbox" id="Relay4Value">
                    <span class="slider round"></span>
                </label>
            </div>
        </div>
    </div>

    <script src="https://www.gstatic.com/firebasejs/8.6.8/firebase-app.js"></script>
    <script src="https://www.gstatic.com/firebasejs/8.6.8/firebase-database.js"></script>

    <script>
        // Your web app's Firebase configuration
        var firebaseConfig = {
            apiKey: "YOUR_API_KEY",
            authDomain: "YOUR_AUTH_DOMAIN",
            databaseURL: "https://esp32c3-816d6-default-rtdb.asia-southeast1.firebasedatabase.app/",
            projectId: "YOUR_PROJECT_ID",
            storageBucket: "YOUR_STORAGE_BUCKET",
            messagingSenderId: "YOUR_MESSAGING_SENDER_ID",
            appId: "YOUR_APP_ID"
        };
        // Initialize Firebase
        firebase.initializeApp(firebaseConfig);

        const database = firebase.database();

        // Function to update switch state
        function updateSwitchState(field, value) {
            const switchElement = document.getElementById(field + 'Value');
            const shouldBeChecked = value === 1;
            if (switchElement.checked !== shouldBeChecked) {
                switchElement.checked = shouldBeChecked;
            }
        }

        // Listen for changes in each relay and update the switch state
        database.ref('IOT_Switches/Relay1').on('value', snapshot => {
            updateSwitchState('Relay1', snapshot.val());
        });
        database.ref('IOT_Switches/Relay2').on('value', snapshot => {
            updateSwitchState('Relay2', snapshot.val());
        });
        database.ref('IOT_Switches/Relay3').on('value', snapshot => {
            updateSwitchState('Relay3', snapshot.val());
        });
        database.ref('IOT_Switches/Relay4').on('value', snapshot => {
            updateSwitchState('Relay4', snapshot.val());
        });

        // Function to update specific data (r1, r2, r3, or r4) in Firebase
        async function updateData(field) {
            const value = document.getElementById(field + 'Value').checked ? 1 : 0;
            try {
                await database.ref('IOT_Switches/' + field).set(value);
            } catch (error) {
                console.error('Error updating data:', error);
            }
        }

        // Function to handle switch changes
        function handleSwitchChange(event) {
            const field = event.target.id.replace('Value', '');
            updateData(field);
        }

        // Add event listeners for the switches
        document.getElementById('Relay1Value').addEventListener('change', handleSwitchChange);
        document.getElementById('Relay2Value').addEventListener('change', handleSwitchChange);
        document.getElementById('Relay3Value').addEventListener('change', handleSwitchChange);
        document.getElementById('Relay4Value').addEventListener('change', handleSwitchChange);
    </script>
</body>
</html>
