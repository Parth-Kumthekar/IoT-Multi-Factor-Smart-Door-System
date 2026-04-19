# Door Alarm API (C++)

## Overview
This module is the **backend API service** for the Door Alarm System project.  
It is implemented in **C++17** and exposes a lightweight HTTP interface for controlling and monitoring the intrusion alarm workflow. The service is built as a standalone executable named `door_alarm_api`. :contentReference[oaicite:0]{index=0}

The API provides endpoints for:
- system arm / disarm
- door lock / unlock
- access authorization
- door open / close simulation
- alarm trigger / clear
- device health updates
- system log retrieval
- test email alerts

It is intended to act as the bridge between the core alarm logic and the frontend dashboard.

---

## Main Features
- lightweight HTTP server based on `httplib`
- JSON API responses for frontend integration
- CORS support for browser-based dashboard access
- in-memory door alarm state management
- recent event logging
- access authorization by **nfc**, **face**, or **api**
- automatic alarm triggering after a verification timeout
- device status update endpoint
- optional email alert support using **libcurl SMTP** 

---

## Tech Stack
- **C++17**
- **cpp-httplib** style HTTP server integration via `httplib.h`
- **libcurl** for email sending
- **CMake** for build configuration
- standard multithreading (`std::thread`, `std::mutex`) 

---

## Project Structure
```text id="4ks2gj"
api/
├── CMakeLists.txt
├── include/
│   └── EmailAlert.h
├── src/
│   ├── main.cpp
│   └── EmailAlert.cpp
└── vendor/
    └── httplib.h

The CMake file builds the API executable from src/main.cpp and src/EmailAlert.cpp, and includes headers from both vendor/ and include/.

Build Configuration

The project uses CMake and defines an executable called:

door_alarm_api

The build:

requires C++17
enables warnings with /W4 on MSVC or -Wall -Wextra -Wpedantic on non-MSVC compilers
links against Threads
links against CURL for email support
API Server Behaviour

The API server:

starts an HTTP server
listens on 0.0.0.0:3000
prints a startup message to the console
enables CORS headers for GET, POST, and OPTIONS
returns JSON responses for all supported routes
returns a JSON 404 message for unknown endpoints

Startup output:

Door Alarm C++ API running at http://localhost:3000

API Endpoints
Root
GET /

Returns service metadata and the list of supported routes. The response includes:

service name
version
available endpoints
Health and Status
GET /api/health

Returns:

service name
current time
uptime in seconds
device health information for:
GPIO
NFC
Camera
API
GET /api/status

Returns the current alarm system state in JSON format. The frontend uses this endpoint to display system status.

GET /api/logs

Returns recent logs.
Supports a limit query parameter. If the requested limit is greater than 200, it is capped at 200. The default limit is 50.

Example:

GET /api/logs?limit=20
System Control
POST /api/system/arm

Arms the system and updates LED status. A system log entry is added.

POST /api/system/disarm

Disarms the system, clears pending verification, clears any active alarm, updates LED status, and adds a log entry.

POST /api/system/lock

Sets the door state to locked and adds a log entry.

POST /api/system/unlock

Sets the door state to unlocked and adds a log entry.

Access Control
POST /api/access/authorize

Authorizes access using a method and user identifier.

Accepted methods:

nfc
face
api

If the method is invalid, the endpoint returns HTTP 400.
If the user field is empty, it defaults to "unknown".

Example request:

{
  "method": "nfc",
  "user": "student_01"
}
Door Events
POST /api/door/open

Marks the door as open and adds a door log entry.
If the system is armed, the API:

starts a pending verification window
updates LED status
adds a log entry showing a 5-second verification window
launches a detached timeout thread

If the door remains open while armed and still pending verification after the timeout, the alarm is triggered automatically.

POST /api/door/close

Marks the door as closed, clears pending verification, updates LED status, and adds a log entry.

Alarm Control
POST /api/alarm/trigger

Triggers the alarm manually.
If a reason is not supplied, the default reason is "Manual alarm trigger".

Example request:

{
  "reason": "Manual dashboard trigger"
}
POST /api/alarm/clear

Clears the active alarm and returns updated state data.

Device Updates
POST /api/devices/update

Updates device availability flags. The endpoint supports:

gpioOnline
nfcOnline
cameraOnline

This is useful for simulating or reporting subsystem health status.

Example request:

{
  "gpioOnline": true,
  "nfcOnline": true,
  "cameraOnline": false
}
Email Testing
POST /api/email/test

Sends a test email in a detached thread using the email alert component.
A log entry is added indicating whether the test email succeeded or failed.

Request / Response Style

All routes use JSON responses and set:

Content-Type: application/json
Access-Control-Allow-Origin: *
Access-Control-Allow-Headers: Content-Type
Access-Control-Allow-Methods: GET, POST, OPTIONS

A typical success response looks like:

{
  "ok": true,
  "message": "System armed successfully",
  "data": {
    "...": "..."
  }
}

Error example:

{
  "ok": false,
  "message": "Route not found"
}
Internal Logic Summary
1. Shared State

The API maintains a shared global system state representing:

whether the system is armed
whether the alarm is active
whether the door is open
lock state
pending verification status
device online status
buzzer / LED state
latest authorization details
uptime and logs

The frontend dashboard reads this state through /api/status.

2. Thread Safety

Request handlers protect shared state with std::mutex and std::lock_guard.
Detached worker threads are also used for:

delayed alarm activation after door-open timeout
background test email sending
3. Verification Window

When the door is opened while the system is armed:

the API sets pendingVerification = true
starts a 5-second timeout window
waits for valid authorization
triggers the alarm if authorization does not arrive in time

This is the core intrusion detection behaviour used by the demo system.

Email Alert Module

The email alert feature is implemented in EmailAlert.cpp and uses libcurl SMTP.
The EmailAlert object is considered configured only when all of the following are present:

SMTP server
SMTP port
sender email
sender password
recipient email

The email module:

builds a simple SMTP payload with To, From, Subject, and body
connects using an smtp://host:port URL
uses STARTTLS / SSL mode
uploads the email body through libcurl callbacks
returns success/failure to the caller
Build Instructions
Requirements

You need:

CMake 3.16+
a C++17 compiler
libcurl development package
threading support
Build
mkdir build
cd build
cmake ..
cmake --build .

This produces the executable:

door_alarm_api

Run Instructions

After building, run:

./door_alarm_api

On Windows:

door_alarm_api.exe

The service will listen on:

http://localhost:3000
Example Test Commands
Health check
curl http://localhost:3000/api/health
Get current status
curl http://localhost:3000/api/status
Arm system
curl -X POST http://localhost:3000/api/system/arm
Open door
curl -X POST http://localhost:3000/api/door/open
Authorize access
curl -X POST http://localhost:3000/api/access/authorize \
  -H "Content-Type: application/json" \
  -d "{\"method\":\"nfc\",\"user\":\"student_01\"}"
Trigger alarm
curl -X POST http://localhost:3000/api/alarm/trigger \
  -H "Content-Type: application/json" \
  -d "{\"reason\":\"manual test\"}"
Clear alarm
curl -X POST http://localhost:3000/api/alarm/clear
Update device state
curl -X POST http://localhost:3000/api/devices/update \
  -H "Content-Type: application/json" \
  -d "{\"gpioOnline\":true,\"nfcOnline\":true,\"cameraOnline\":false}"
Send test email
curl -X POST http://localhost:3000/api/email/test
Frontend Integration

The dashboard frontend is configured to call this API using:

const API_BASE = "http://127.0.0.1:3000";

It uses the API for status, logs, system controls, alarm controls, and access authorization.

Notes
This API currently uses simple string-based JSON field extraction for request parsing rather than a full JSON library.
State is stored in memory and is reset when the server restarts. This is an inference based on the current code structure, because no persistence layer appears in the provided files.
The service is suitable for coursework demos, local testing, and dashboard integration, but would need stronger validation, authentication, and persistence for production use. This is an engineering judgment based on the current implementation.
Conclusion

The Door Alarm API is a lightweight C++ backend service that exposes the alarm system through simple HTTP endpoints. It demonstrates:

backend service design in C++
browser-compatible JSON APIs
alarm workflow control
device status management
email alert integration
clean integration with a React dashboard

It is well suited for demonstrating backend integration in a smart security coursework project.
