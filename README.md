# Thrust Test Rig with IoT

## Overview
This project is a **Thrust Test Rig with IoT capabilities** that measures and monitors thrust generated by a motor or propulsion system. It integrates sensors and WiFi connectivity for real-time data logging and remote monitoring.

## Features
- **Thrust Measurement**: Uses an **HX711 load cell** to measure force.
- **RPM Monitoring**: Tracks motor speed using an **RPM sensor**.
- **Vibration Analysis**: Utilizes an **ADXL335 accelerometer**.
- **Temperature Sensing**: Reads motor temperature via an **MLX90614 IR sensor**.
- **IoT Connectivity**: Sends real-time data via **WiFi** using an **ESP32**.
- **Web Dashboard**: Displays thrust, RPM, and temperature.

## Components Used
- **ESP32** (Microcontroller)
- **HX711** (Load cell amplifier)
- **RPM Sensor**
- **ADXL335** (Accelerometer)
- **MLX90614** (Infrared temperature sensor)
- **WiFi Module** (Built-in ESP32)

## Installation & Setup
1. Install **Arduino IDE** and necessary libraries:
   - `HX711` (Load cell driver)
   - `WiFi` (ESP32 network handling)
   - `WebServer` (For dashboard)
   - `ArduinoJson` (JSON handling)
   - `Adafruit_MLX90614` (Temperature sensor support)
2. Connect components as per circuit diagram.
3. Upload `thrust.ino` to ESP32.
4. Access data via the web interface.

## Usage
- Power the rig and ensure WiFi connectivity.
- Open the web dashboard to view real-time thrust, RPM, and temperature.
- Use data for analysis and system tuning.

## Future Enhancements
- Cloud data storage for analytics.
- Mobile app integration.
- AI-based performance predictions.

## License
This project is open-source under the **MIT License**.

## Diagram
https://github.com/DrkVip/Thrust-Test-Rig-With-IOT/blob/main/Diagram%202.jpg?raw=true

## Output
https://github.com/DrkVip/Thrust-Test-Rig-With-IOT/blob/main/Output%202.jpg?raw=true


