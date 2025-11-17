# Spec5 Wi-Dar

**Air Force-Grade WiFi Tracking and Threat Assessment System**

A professional tactical WiFi surveillance platform for ESP32-S3 hardware featuring real-time threat classification, track management, and comprehensive alerting capabilities.

---

## 🎯 Features

### **Tactical Radar Display**
- **Full-screen polar radar** with 360° coverage
- **IFF symbols**: Triangles (hostile), circles (neutral), squares (friendly)
- **Track trails**: Dotted lines showing last 5 positions
- **Bearing & range display**: BRG 045° RNG 12m format
- **Distance estimation**: RSSI-to-distance conversion using path loss model
- **Threat classification**: Hostile, Suspicious, Neutral, Friendly, Passive
- **Animated sweep line**: Authentic radar feel with scanline effect
- **Range rings**: 25%, 50%, 75%, 100% markers with labels
- **Azimuth markers**: N/E/S/W compass points

### **Track Management**
- **5-track history** per AP (configurable 1-10)
- **Visual fade**: Opacity 100% → 20% over history
- **Radius shrink**: 8px → 2px as tracks age
- **3-iteration validation**: Prevents false positives (configurable 1-5)
- **Automatic track cleanup**: Removes APs lost for >30 seconds

### **Threat Assessment**
- **Auto-classification** based on behavior patterns:
  - **HOSTILE** (Red): Tagged threats with strong signal
  - **SUSPICIOUS** (Orange): Spoofed SSIDs, high signal variance (movement)
  - **NEUTRAL** (Yellow): Unknown but stable APs
  - **FRIENDLY** (Green): Tagged as safe
  - **PASSIVE** (Gray): Weak distant signals
- **Signal variance detection**: Identifies moving APs
- **Proximity alerts**: Configurable distance threshold (default 5m)

### **Air Force-Style Message Log**
- **Scrolling message area** with 3-5 visible lines
- **Color-coded messages**:
  - Green: Normal operations
  - Yellow: New transponders
  - Red: Tagged contacts
  - Gray: Lost tracks
- **Timestamps**: HH:MM:SS format
- **Auto-scroll**: Newest messages at bottom
- **Message types**:
  - New transponder identified
  - Tagged contact appeared
  - Transponder lost
  - Proximity alerts

### **Tagged Contacts System**
- **Wildcard pattern matching**: `Linksys*`, `*5G*`, case-insensitive
- **10 color options**: Red, Orange, Yellow, Green, Cyan, Blue, Purple, Magenta, White, Pink
- **Audio alerts**: Configurable per tag (600-1000 Hz beeps)
- **Visual alerts**: Screen flash on tagged contact appearance
- **Table view**: Add, edit, delete tagged contacts
- **Persistent storage**: Saved to ESP32 Preferences

### **Signal Strength Graph**
- **Time-series display**: RSSI over time for all APs
- **Multi-AP tracking**: Up to 7 APs shown simultaneously
- **Color-coded lines**: Each AP in different color
- **Auto-scaling Y-axis**: -100 to -30 dBm typical range
- **Grid overlay**: Horizontal lines for readability
- **Live legend**: Shows current RSSI for each AP

### **Configuration Panel**
- **Track Settings**:
  - Track history count (1-10)
  - Validation threshold (1-5 iterations)
  - Scan interval (1-10 seconds)
  - Proximity alert distance (1-50 meters)
- **Mission Profiles**:
  - Perimeter Security: High sensitivity, focus on new contacts
  - Network Survey: Maximum data collection, minimal alerts
  - Threat Hunting: Aggressive alerting, suspicious pattern focus
  - Training Mode: Reduced sensitivity, explanatory mode
- **System Diagnostics**:
  - Free heap memory
  - ESP32 core temperature
  - Scan success rate

### **Session Logging**
- **JSON export**: Complete session data with tracks, config, tags, and log
- **Downloadable**: One-click export to file
- **Forensic analysis**: Timestamp, RSSI history, first/last seen
- **1000-entry buffer**: Rolling log with automatic cleanup

### **Keyboard Shortcuts**
- **SPACE**: Pause/Resume scanning
- **R**: Reset all tracks
- **M**: Mute/Unmute audio alerts
- **F**: Toggle fullscreen
- **1-4**: Switch between tabs (Radar, Signal, Config, Tags)
- **?**: Toggle keyboard shortcuts help

### **Heads-Up Display (HUD)**
- **Contact counts**: Total, Hostile, Suspicious, Neutral, Friendly
- **System uptime**: HH:MM:SS format
- **Mission profile**: Current active profile
- **Last scan**: Time since last WiFi scan
- **Color-coded threat levels**: Visual threat summary

---

## 📡 Technical Specifications

### **Hardware Requirements**
- **ESP32-S3** (TDongle S3 recommended)
- **WiFi**: 2.4 GHz 802.11 b/g/n
- **Memory**: 327 KB RAM, 1.28 MB Flash

### **Software**
- **Arduino IDE** or **arduino-cli**
- **ESP32 Board Package** v3.3.4+
- **Libraries**: WiFi, WebServer, ArduinoJson, Preferences (all built-in)

### **Performance**
- **Program size**: 940 KB (71% of 1.28 MB)
- **RAM usage**: 44 KB (13% of 327 KB)
- **Scan rate**: 1-10 seconds (configurable, default 3s)
- **Track capacity**: 20+ simultaneous APs
- **Update rate**: 30 FPS radar animation

### **Distance Estimation Algorithm**
```
Distance (m) = 10 ^ ((Tx_Power - RSSI) / (10 * Path_Loss_Exponent))
```
- **Tx Power**: -30 dBm (typical WiFi AP)
- **Path Loss Exponent**: 2.5 (indoor environment)
- **Range bands**:
  - Near: 0-5m (RSSI > -50 dBm)
  - Medium: 5-15m (RSSI -50 to -70 dBm)
  - Far: 15-40m (RSSI -70 to -85 dBm)
  - Extreme: 40m+ (RSSI < -85 dBm)

---

## 🚀 Installation

### **1. Hardware Setup**
1. Connect ESP32-S3 TDongle to computer via USB-C
2. Ensure drivers are installed (CP210x or CH340)

### **2. Arduino IDE**
```bash
# Install ESP32 board support
# Boards Manager → esp32 by Espressif Systems → Install

# Open Wi-Dar.ino
# Tools → Board → ESP32S3 Dev Module
# Tools → Upload Speed → 921600
# Tools → Port → Select your device

# Click Upload
```

### **3. Arduino CLI**
```bash
# Clone repository
git clone https://github.com/ahtx/Spec5-Wi-Dar.git
cd Wi-Dar

# Compile
arduino-cli compile --fqbn esp32:esp32:esp32s3 Wi-Dar.ino

# Upload
arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32s3 Wi-Dar.ino
```

---

## 🎮 Usage

### **1. Power On**
- ESP32 creates WiFi access point: **Wi-Dar**
- Password: **tactical123**

### **2. Connect**
- Connect device to **Wi-Dar** network
- Navigate to: **http://192.168.4.1**

### **3. Interface**

#### **Tab 1: Radar**
- Full-screen tactical radar display
- Watch for new transponders (yellow)
- Monitor tagged contacts (red)
- Observe track movement via trails

#### **Tab 2: Signal Strength**
- Time-series graph of all APs
- Monitor RSSI trends
- Identify signal fluctuations

#### **Tab 3: Configuration**
- Adjust track settings
- Change mission profile
- View system diagnostics
- Export session log

#### **Tab 4: Tagged Contacts**
- Add new tagged contacts
- Specify wildcard patterns
- Configure colors and sounds
- Delete unwanted tags

### **4. Mission Profiles**

**Perimeter Security** (Default)
- High sensitivity to new contacts
- Proximity alerts enabled
- Audio alerts for tagged contacts
- Recommended for security monitoring

**Network Survey**
- Maximum data collection
- Minimal alerting
- All APs logged
- Recommended for site surveys

**Threat Hunting**
- Aggressive alerting
- Focus on suspicious patterns
- High validation threshold
- Recommended for counter-surveillance

**Training Mode**
- Reduced sensitivity
- Explanatory tooltips
- Lower alert volume
- Recommended for learning

---

## 🎯 Use Cases

### **Security Monitoring**
- Detect rogue access points
- Monitor for unauthorized WiFi devices
- Perimeter intrusion detection
- Facility security sweeps

### **Counter-Surveillance**
- Identify hidden cameras with WiFi
- Detect wireless bugs
- Find tracking devices
- Privacy protection

### **Network Analysis**
- Site surveys for WiFi deployment
- Channel utilization analysis
- Signal strength mapping
- Interference detection

### **SIGINT Operations**
- Tactical WiFi reconnaissance
- Target tracking via WiFi signatures
- Movement pattern analysis
- Electronic warfare support

---

## 📊 API Endpoints

### **GET /api/scan**
Returns current WiFi scan results
```json
{
  "networks": [
    {"ssid": "...", "rssi": -45, "channel": 6, "bssid": "...", "encryption": "..."}
  ],
  "timestamp": 123456,
  "count": 12,
  "uptime": 3600,
  "freeHeap": 250000,
  "temperature": 52.3
}
```

### **GET /api/config**
Returns current configuration
```json
{
  "trackHistory": 5,
  "validationThreshold": 3,
  "scanInterval": 3,
  "proximityAlert": 5,
  "missionProfile": "perimeter"
}
```

### **POST /api/config**
Updates configuration (same JSON structure as GET)

### **GET /api/tags**
Returns all tagged contacts
```json
{
  "tags": [
    {"name": "Home Router", "pattern": "Linksys*", "color": "green", "sound": true}
  ]
}
```

### **POST /api/tags**
Adds new tagged contact
```json
{"name": "...", "pattern": "...", "color": "...", "sound": true}
```

### **DELETE /api/tags**
Deletes tagged contact
```json
{"name": "..."}
```

---

## 🔧 Customization

### **Adjust Distance Algorithm**
Edit `estimateDistance()` function in Wi-Dar.ino:
```cpp
const txPower = -30;        // Adjust based on your APs
const pathLossExp = 2.5;    // 2.0 (free space) to 4.0 (dense indoor)
```

### **Change Threat Thresholds**
Edit `getThreatLevel()` function:
```cpp
if (track.rssi > -50) return 'hostile';  // Adjust RSSI thresholds
if (variance > 100) return 'suspicious'; // Adjust variance threshold
```

### **Modify Colors**
Edit CSS in HTML section:
```css
.threat-hostile{color:#f00}      /* Red */
.threat-suspicious{color:#f80}   /* Orange */
.threat-neutral{color:#ff0}      /* Yellow */
.threat-friendly{color:#0f0}     /* Green */
```

---

## 📝 License

Copyright © 2025 SpecFive  
All Rights Reserved

This software is provided for educational and research purposes only.

---

## 🤝 Credits

**Developed by:** SpecFive  
**Platform:** ESP32-S3  
**Framework:** Arduino  
**Repository:** https://github.com/ahtx/Spec5-Wi-Dar

---

## ⚠️ Disclaimer

This tool is designed for **authorized security assessments and network analysis only**. Users are responsible for compliance with all applicable laws and regulations regarding WiFi monitoring and electronic surveillance in their jurisdiction.

**Do not use for:**
- Unauthorized network access
- Privacy violations
- Illegal surveillance
- Interference with critical infrastructure

---

## 📞 Support

For issues, feature requests, or questions:
- **GitHub Issues**: https://github.com/ahtx/Spec5-Wi-Dar/issues
- **Website**: https://www.specfive.com

---

**SPEC5 WI-DAR - TACTICAL WIFI TRACKING ACTIVE**
