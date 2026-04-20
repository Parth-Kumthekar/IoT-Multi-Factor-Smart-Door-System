This document outlines the wiring and pin configuration for the Pico W and Raspberry Pi 5 based access control system.

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
