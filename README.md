## Team Responsibilities
This project was developed as a group coursework project, with each member focusing on a different subsystem of the overall door alarm solution.

### Member Responsibilities
- **GPIO / Hardware Integration**
  - handled GPIO interrupt logic
  - read switch and door sensor input
  - controlled LED and buzzer output
  - implemented debounce handling
  - integrated the NFC reader module

- **Camera / Recognition Module**
  - handled camera integration
  - implemented face recognition related logic
  - supported identity verification workflow

- **Thread Design / Event Queue / FSM**
  - designed the multi-threaded control structure
  - implemented the event-driven queue mechanism
  - developed the finite state machine for alarm logic
  - coordinated timeout and state transition behaviour

- **API / Web Dashboard**
  - implemented the backend API endpoints
  - developed the web dashboard interface
  - connected frontend actions with backend system status
  - supported monitoring and remote control functions

This division of work allowed the team to build the project in a modular way while keeping each subsystem clearly separated and easier to maintain.

---

## How to Demonstrate in Presentation
During the presentation or viva, the project can be demonstrated in the following order:

### 1. Introduce the project purpose
Briefly explain that the project is a smart door alarm system combining:
- hardware-style control logic
- multi-threading
- event queue communication
- FSM-based decision making
- API and dashboard integration

### 2. Explain the system architecture
Show the overall structure:
- sensor / user action
- event queue
- control thread
- FSM state transition
- alarm response
- dashboard update

Emphasise that the system is event-driven and modular.

### 3. Explain the backend logic
Focus on:
- why multithreading is used
- how the event queue avoids tight coupling
- how the FSM makes alarm behaviour explicit
- how timeout triggers alarm activation

### 4. Demonstrate key scenarios
A good live demo order is:

#### Scenario A: Normal authorized access
1. arm the system
2. open the door
3. authorize by NFC or APP
4. close the door

Explain that:
- the system enters pending verification
- valid authorization is received
- no alarm is triggered
- state returns to normal

#### Scenario B: Intrusion / unauthorized access
1. arm the system
2. open the door
3. do not provide authorization
4. wait for timeout

Explain that:
- timeout event is generated
- FSM moves to alarm state
- alarm becomes active
- dashboard shows intrusion and buzzer warning

#### Scenario C: Alarm clearing / recovery
1. clear the alarm
2. disarm the system
3. return to safe state

### 5. Show the dashboard
Demonstrate that the dashboard can:
- arm/disarm the system
- open/close the door
- lock/unlock
- trigger/clear alarm
- show logs and system status

### 6. Highlight engineering value
At the end, mention that the design is:
- modular
- easy to extend
- suitable for Raspberry Pi deployment
- ready for future GPIO / NFC / camera integration

---

## Originality Statement
This submission is our own group work produced for coursework purposes.

We confirm that:
- the design, implementation, integration, and documentation submitted in this project are based on our own understanding and development work;
- all external libraries, frameworks, and tools used in the project have been appropriately acknowledged;
- any referenced open-source software, development frameworks, and publicly available resources remain the property of their original authors and are used in accordance with their respective licences;
- this submission has not been copied from another student, group, or external source.

Where collaboration took place within the group, each member contributed to their assigned subsystem and the final integrated system was produced collectively.

---

## Use of Generative AI
Generative AI tools were used in a limited and supportive manner during the development of this project.

These tools were used for purposes such as:
- improving code readability and comments
- refining documentation and README writing
- helping rephrase technical explanations
- assisting with debugging suggestions and structure improvement

All final design decisions, code integration, testing, and submitted content were reviewed and validated by the project members.  
We take full responsibility for the correctness, originality, and academic integrity of the final submission.

---

## Open-Source and Third-Party Acknowledgement
This project may include or depend on third-party tools and frameworks such as:
- **CMake** for backend build configuration
- **React** for frontend UI development
- **Vite** for frontend development server and bundling
- standard C++ and JavaScript libraries used during implementation

All such tools and frameworks remain the intellectual property of their respective owners and are used under their corresponding open-source licences.


如果你想更像正式交作业，我也可以继续帮你把这一版改成 更英式学术、老师一看就像 coursework submission 的语气。

单独dashboard的readme
已思考若干秒

下面给你一版 单独针对 dashboard 前端 的 README.md，可以直接放到 dashboard 文件夹里用。

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
