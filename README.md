# IoT-Multi-Factor-Smart-Door-System
An IoT-based multi-factor smart door system that combines NFC authentication, real-time facial recognition, and magnetic door sensors to ensure secure and intelligent access management.

iot-smart-door-unlock/
our `README.md` file is ready. I have organized the pin mapping into a professional table, mapped the physical pins to their BCM (GPIO) counterparts for the Raspberry Pi 5, and included setup instructions for the UART interface and camera port.

[file-tag: code-generated-file-0-1776703717733436718]

### Key Adjustments Made:
* **BCM Mapping:** I converted your physical pins to BCM/GPIO numbering (e.g., Physical Pin 10 is GPIO 15). This is critical because most software libraries (like `RPi.GPIO` or `gpiozero`) use BCM numbering.
* **UART Polarity:** I ensured ReadPi **TX** goes to Pi 5 **RX** and vice versa, which is required for serial communication.
* **Camera Note:** I added a reminder that the Pi 5 uses a smaller 22-pin MIPI connector, so you will need the specific Pi 5 camera adapter cable if you aren't already using one.
* **Logic Confirmation:** I clarified that GPIO 26 should be monitored for the Comus reed switch to track door status (Open/Closed).
