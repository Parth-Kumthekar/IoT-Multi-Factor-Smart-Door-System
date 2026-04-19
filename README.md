# Door Alarm Dashboard

## Overview
This dashboard is the frontend interface for the **Door Alarm System** project.  
It is built with **React** and **Vite**, and provides a simple web-based control panel for monitoring and operating the door intrusion alarm system. The frontend communicates with the backend API running at `http://127.0.0.1:3000`. 

The dashboard is designed to:
- display the current system state
- send control commands to the backend
- show recent logs
- provide a clear visual interface for demonstration and testing

---

## Features
The dashboard supports the following functions:

- view system status in real time
- arm and disarm the alarm system
- lock and unlock the door
- open and close the door
- trigger and clear the alarm
- authorize access using **NFC** or **APP**
- display access and system logs
- show subsystem health information such as camera, NFC, GPIO, and API status :contentReference[oaicite:1]{index=1}

---

## Tech Stack
- **React 18**
- **Vite 5**
- **JavaScript (ES Modules)**
- **CSS** for layout and styling 

---

## Project Structure
```text id="oycsj9"
dashboard/
├── index.html
├── package.json
├── package-lock.json
├── vite.config.js
└── src/
    ├── App.jsx
    ├── main.jsx
    ├── api.js
    └── styles.css
Main Files
index.html

This is the root HTML file for the dashboard.
It sets the page title to Door Intrusion Alarm Dashboard and mounts the React app into the root element.

src/api.js

This file handles all communication with the backend API.
It defines helper functions for sending GET and POST requests and provides methods such as:

getStatus()
getLogs()
armSystem()
disarmSystem()
lockDoor()
unlockDoor()
triggerAlarm()
clearAlarm()
openDoor()
closeDoor()
authorizeAccess()
src/styles.css

This file defines the visual design of the dashboard.
It includes:

page and container layout
responsive card grids
top bar styling
control buttons
alert boxes
log panels
mobile-friendly layout adjustment with media queries
vite.config.js

This file configures Vite and sets the development server port to 5173.

Backend Connection

The dashboard is configured to connect to the backend API at:

http://127.0.0.1:3000

This address is defined in src/api.js.

If the backend is not running, the dashboard will not be able to fetch status data or send control commands.

Status Information Displayed

The dashboard reads and displays a range of backend status fields, including:

systemArmed
alarmActive
doorOpen
lockState
lastAuthorizedMethod
lastAuthorizedUser
lastAuthorizationTime
pendingVerification
intrusionDetected
buzzerOn
ledStatus
cameraOnline
nfcOnline
gpioOnline
apiOnline
uptimeSeconds

This allows the user to observe both the security state and the operating condition of the system.

API Endpoints Used

The frontend interacts with these backend endpoints:

GET
/api/status
/api/logs?limit=20
POST
/api/system/arm
/api/system/disarm
/api/system/lock
/api/system/unlock
/api/alarm/trigger
/api/alarm/clear
/api/door/open
/api/door/close
/api/access/authorize
Installation

Make sure you have Node.js installed first.

Then run:

npm install

This installs all frontend dependencies listed in package.json, including React, React DOM, Vite, and the React plugin for Vite.

Running the Dashboard
Development Mode
npm run dev

This starts the Vite development server.

By default, the dashboard runs on:

http://localhost:5173

because the Vite config explicitly sets port 5173.

Build for Production
npm run build
Preview Production Build
npm run preview

These scripts are defined in package.json.

Example Usage
Scenario 1: Authorized Access
Arm the system
Open the door
Authorize access using NFC or APP
Close the door

Expected result:

the system accepts authorization
no alarm is triggered
status updates are reflected on the dashboard
Scenario 2: Unauthorized Access
Arm the system
Open the door
Do not authorize access
Wait for timeout

Expected result:

intrusion is detected
alarm becomes active
buzzer and warning-related states are shown
Scenario 3: Manual Alarm Control
Trigger the alarm manually
Observe dashboard status and logs
Clear the alarm

This is useful for testing the API and frontend integration.

UI Design

The dashboard uses a clean card-based interface with:

top summary area
information panels
action buttons
alert boxes
log panels
responsive grid layout

The CSS is designed so that the interface also adapts to smaller screen widths using a media query below 1100px.

Notes
The dashboard depends on the backend API being available at 127.0.0.1:3000.
If CORS or connection issues occur, check whether the backend server is running correctly.
This dashboard is mainly intended for coursework demonstration, testing, and system monitoring.
Conclusion

The Door Alarm Dashboard provides a simple and effective frontend for controlling and observing the Door Alarm System.
It demonstrates how a React-based interface can be integrated with a backend security control system through API calls, offering both usability and clear system visibility.
