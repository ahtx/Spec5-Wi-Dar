# ESP32-S3 T-Dongle Terminal System

## Quick Start Guide

### Option 1: Using Arduino IDE (Recommended)
1. Install Arduino IDE from https://arduino.cc
2. Add ESP32 boards: File → Preferences → Additional Board Manager URLs:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. Tools → Board Manager → Search "ESP32" → Install
4. Open `ESP32_Terminal.ino` 
5. Select Board: Tools → Board → ESP32 Arduino → ESP32S3 Dev Module
6. Select Port: Tools → Port → COM3 (your ESP32-S3 T-Dongle)
7. Upload: Sketch → Upload

### Option 2: Using PlatformIO
1. Install PlatformIO: `pip install platformio`
2. Navigate to `esp32-fabgl-terminal` folder
3. Run: `pio run -t upload`

## Hardware Setup
- **Device**: ESP32-S3 T-Dongle (detected as COM3)
- **Connection**: USB to computer
- **No additional wiring required!**

## Usage
1. Flash the firmware to your ESP32-S3 T-Dongle
2. Open Putty (or any serial terminal)
3. Configure:
   - Connection Type: Serial
   - Serial Line: COM3  
   - Speed: 115200
4. Click "Open"
5. Type commands at the ESP32-S3> prompt

## Available Commands
- `help` - Show all commands
- `cls` - Clear screen
- `dir` - Show flash memory contents
- `sysinfo` - Hardware information
- `games` - List available games
- `snake` - Play Snake (use WASD, Q to quit)
- `tetris` - Play Tetris simulation
- `calc` - Calculator (supports +, -, *, /, ^)
- `demo` - Run demo sequence
- `reboot` - Restart device

## Features
✅ Serial terminal access via USB
✅ DOS-like command interface  
✅ Built-in retro games
✅ System information display
✅ Calculator functionality
✅ ANSI escape code support
✅ Real-time hardware monitoring

## Troubleshooting
- **Can't find COM3**: Check Device Manager, install CP210x drivers
- **Upload fails**: Hold BOOT button while uploading
- **No response**: Check baud rate is 115200
- **Garbled text**: Verify terminal encoding is UTF-8

## What Makes This Cool
Your ESP32-S3 T-Dongle becomes a **pocket retro computer** that you can:
- Connect to via any computer with Putty
- Play games on over serial terminal
- Use as a development tool
- Run as a DOS-like system
- Access system information
- Perform calculations

Perfect for demos, learning, or just having a tiny computer in your pocket!