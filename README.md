This document outlines the wiring and pin configuration for the Pico W and Raspberry Pi 5 based access control system.
installing the image to raspberry pi5 - boot image
<img width="802" height="611" alt="image" src="https://github.com/user-attachments/assets/22a490d6-1376-447a-9073-a51a4f36e3f4" />
Raspberry Pi Imaging Utility

To install on Raspberry Pi OS, use sudo apt update && sudo apt install rpi-imager.
Download the latest version for Windows, macOS and Ubuntu from the Raspberry Pi downloads page.


<img width="2123" height="1307" alt="image" src="https://github.com/user-attachments/assets/1123f9a0-3b3b-4554-b481-ba6fd65ae820" />
<img width="3275" height="1069" alt="image" src="https://github.com/user-attachments/assets/19d3fb02-0561-4b75-b1c3-ce83f117d835" />

(1) 1.3” Display
(2) NFC Module
(3) Joystick
(4) GPIOs breakout
(5) Battery Connector
(6) TF card slot
(7) Buzzer
(8) Pico W


2. Install Dependencies
Bash
sudo apt install -y build-essential cmake libgpiod-dev libcurl4-openssl-dev libssl-dev

4. Serial Port Configuration (Raspberry Pi 5)
Since this project uses the hardware UART (ttyAMA0 / ttyAMA10), you must disable the Linux serial console to prevent data corruption:

Run sudo raspi-config.

Navigate to Interface Options -> Serial Port.

Select No for "Would you like a login shell to be accessible over serial?".

Select Yes for "Would you like the serial port hardware to be enabled?".

Reboot your Raspberry Pi.

4. Permissions
Ensure your user has permission to access the serial and GPIO hardware:

Bash
sudo usermod -a -G dialout $USER
sudo usermod -a -G gpio $USER
(Note: You must log out and back in for group changes to take effect.)

🚀 Building the Project
Once the dependencies are installed, use the following commands to compile the system:

Bash
./build.sh


## 1. ReadPi and Raspberry Pi 5 Interfacing (NFC UID Transfer)
| Device | ReadPi Pin | RPi 5 Physical Pin | RPi 5 BCM/GPIO | Function |
| :--- | :--- | :--- | :--- | :--- |
| **ReadPi** | GPIO 0 (TX) | **Pin 10** | **GPIO 15 (RXD0)** | Serial UART connection |
| **ReadPi** | GPIO 1 (RX) | **Pin 8** | **GPIO 14 (TXD0)** | Serial UART connection |
| **Ground** | GND | **Pin 6** | **Ground** | Common Ground |

## 2. Pico W and RFID module interfacing
| Pico W Pin | NFC Module Pin | Function |
| :--- | :--- | :--- |
| **GP4** | RX | Serial UART connection |
| **GP5** | TX | Serial UART connection |

## 3. Pico W and Display interfacing
| Pico W Pin | Display Pin | Function |
| :--- | :--- | :--- |
| **GP10** | SCLK | Clock pin of SPI interface for display |
| **GP11** | DIN | MOSI (Master OUT Slave IN) data pin of SPI interface |
| **GP8** | DC | Data/Command pin of SPI interface |
| **GP9** | CS | Chip Select pin of SPI interface for display |
| **GP12** | Reset | Display Reset Pin |

## 4. Pico W and micro SD card interfacing
| Pico W Pin | microSD Card | Function |
| :--- | :--- | :--- |
| **GP18** | SCLK | Clock pin of SPI interface for microSD card |
| **GP19** | DIN | MOSI (Master OUT Slave IN) data pin of SPI interface |
| **GP16** | DOUT | MISO (Master IN Slave OUT) data pin of SPI interface |
| **GP17** | CS | Chip Select pin of SPI interface for SDcard |

## 5. Security & Feedback Interfacing (Raspberry Pi 5)
| Component | RPi 5 Physical Pin | RPi 5 BCM/GPIO | Function |
| :--- | :--- | :--- | :--- |
| **Reed Switch** | Pin 37 | **GPIO 26** | Monitor door status (0: Closed, 1: Open) |
| **Red LED** | Pin 11 | **GPIO 17** | Access Denied indicator |
| **Green LED** | Pin 15 | **GPIO 22** | Access Granted indicator |
| **Buzzer** | Pin 13 | **GPIO 27** | Alarm for Denied/Force entry |

## 6. Camera Module
| Device | Port | Connection |
| :--- | :--- | :--- |
| **Camera Module** | **CAM0** | Plugged into the Raspberry Pi 5 CAM0 port |

## 7. Joystick, Buzzer and LED Interfacing (Pico W)
| Pico W Pin | Component | Function |
| :--- | :--- | :--- |
| **GP2** | Joystick X | Analog Horizontal Input |
| **GP3** | Joystick Y | Analog Vertical Input |
| **GP22** | Joystick SW | Button Press |
| **GP14** | Buzzer | Audio Feedback |
| **GP15** | Status LED | System Status Indicator |
