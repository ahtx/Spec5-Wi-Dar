# Spec5 Wi-Dar

A tactical WiFi detection and analysis system for ESP32 devices.

## Features

- Real-time WiFi network detection and monitoring
- Tactical HUD interface with radar visualization  
- Signal strength analysis and proximity alerts
- Tagged contact management with custom threat levels
- Mission profile configurations
- Web-based control interface

## Hardware

- ESP32-S3 based device (tested on XIAO ESP32S3)
- Displays as Wi-Fi Access Point: **Wi-Dar** (password: tactical123)

## Usage

1. Flash the Spec5-Wi-Dar.ino sketch to your ESP32 device
2. Connect to the "Wi-Dar" access point
3. Navigate to http://192.168.4.1 in your browser
4. Use the tactical interface to monitor WiFi networks

## Interface

The system features a military-style HUD with:
- **Spec5** branding with Kommon Grotesk Extended Heavy font
- Real-time contact counting and threat assessment
- Interactive radar display with signal visualization
- Configurable scanning parameters and mission profiles
- Tagged contact management system

Built for tactical network reconnaissance and security assessment.
