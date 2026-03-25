# 🔔 DPSI Autonomous Smart Bell System
An IoT-based automated scheduling solution designed for modern school campuses.

## 📌 Project Overview
The **DPSI Autonomous Smart Bell** is a C++ powered IoT system that automates school period transitions. By leveraging **NTP (Network Time Protocol)**, the system ensures millisecond-level accuracy without manual intervention. It features a localized web dashboard for administrators to manage schedules, toggle holiday modes, and trigger instant rings.

## 🚀 Key Features
* **Precision Timing:** Synchronizes with global NTP servers via WiFi.
* **Web Dashboard:** Secure admin panel (`admin`) for remote scheduling.
* **Manual Overrides:** Physical button (GPIO 4) and digital "Instant Ring" functionality.
* **Holiday Mode:** One-click toggle to suspend the schedule during breaks.
* **Real-time Feedback:** 16x2 LCD integration to display current time and next bell.

## 🛠️ Hardware Requirements
* **Microcontroller:** ESP32 Dev Module (or Arduino Nano/Uno R4)
* **Relay:** SSR-40DA (Solid State Relay) for silent, reliable switching
* **Display:** 16x2 LCD with I2C Module
* **Clock:** DS1307 RTC (Backup for offline scenarios) (IF REQUIRE- OFFLINE)
* **Inputs:** Physical momentary push button

## 💻 Software & Libraries
Built with **C++** using the Arduino framework. Key libraries used:
* `WiFi.h` - Network connectivity
* `NTPClient.h` - Time synchronization
* `WebServer.h` - Hosting the admin dashboard
* `LiquidCrystal_I2C.h` - Display management

## ⚙️ Installation & Setup
1.  **Clone the Repo:** ```bash
    git clone [https://github.com/ShashankJangid/DPSI_School_Bell.git](https://github.com/ShashankJangid/DPSI_School_Bell.git)
    ```
2.  **Configure WiFi:** Create a `config.h` file and define your `SSID` and `PASSWORD` or Direct add your Wifi Configuration in the main C++ code.
3.  **Upload:** Use VS Code (PlatformIO) or Arduino IDE to flash the code to your ESP32.
4.  **Access Dashboard:** Open your browser and navigate to the ESP32's IP address.

## 🔒 Security Note
The default admin credentials are set to:
* **ID:** `admin`
* **Password:** `admin`
*Note: Ensure you change these before deploying in a live school environment.*

---
**Developed for DPSI School Projects**
