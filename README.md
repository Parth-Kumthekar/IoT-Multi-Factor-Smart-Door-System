# Hardware Wiring Guide: ReadPi to Raspberry Pi 5

This document provides the specific pin mapping for connecting a ReadPi (NFC Reader) to a Raspberry Pi 5 for door access control.

## 1. GPIO & UART Pin Mapping

| Device | Source Pin | RPi 5 Physical Pin | RPi 5 BCM / GPIO | Interface / Function |
| :--- | :--- | :--- | :--- | :--- |
| **ReadPi** | GPIO 0 (TX) | **Pin 10** | GPIO 15 (RXD0) | UART NFC UID Data |
| **ReadPi** | GPIO 1 (RX) | **Pin 8** | GPIO 14 (TXD0) | UART NFC Command |
| **ReadPi** | GND | **Pin 6** | Ground | Common Ground |
| **Reed Switch** | Comus Output | **Pin 37** | **GPIO 26** | Door Status (0:Close / 1:Open) |
| **Red LED** | Anode (+) | **Pin 11** | **GPIO 17** | Access Denied |
| **Green LED** | Anode (+) | **Pin 15** | **GPIO 22** | Access Granted |
| **Buzzer** | Signal | **Pin 13** | **GPIO 27** | Force Entry / Denied Alarm |

## 2. Peripheral Configuration

### Camera Module
* **Port:** Use **CAM0** (located near the USB-C power input).
* **Note:** Ensure you are using the 22-pin to 15-pin flexible adapter cable required for Pi 5.

### UART Interface (NFC Transfer)
To allow the ReadPi to send the UID to the Pi 5, you must enable Serial in the Pi configuration:
1. Run `sudo raspi-config`.
2. Go to **Interface Options** -> **Serial Port**.
3. Disable **Login Shell**.
4. Enable **Serial Port Hardware**.

## 3. Component Logic
* **Reed Switch (GPIO 26):** Monitor this pin. `0` indicates the door is closed (magnet present), `1` indicates the door is open (magnet moved).
* **Buzzer (GPIO 27):** Programmed to trigger on "Force Entry" (Door open without access grant) or "Access Denied".
