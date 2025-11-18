# Spec5 Wi-Dar
## Tactical WiFi Surveillance System
### User Manual - Version 1.0

---

### **specfive.com**

---

## 1. Introduction

The **Spec5 Wi-Dar** is an air force-grade, portable WiFi intelligence and surveillance system integrated into a compact USB dongle. Designed for security professionals, network analysts, and government agencies, the Wi-Dar provides real-time situational awareness of the 2.4 GHz WiFi spectrum.

Its powerful web-based interface, inspired by tactical military systems, delivers a clear and immediate operational picture, allowing for rapid detection, identification, and tracking of WiFi access points (APs). The system is entirely self-contained, requiring only a standard 5V USB power source to create its own secure WiFi network for interface access.

This manual provides a comprehensive overview of the Spec5 Wi-Dar device, its features, and operational procedures.

---

## 2. Use Cases

The Wi-Dar system is engineered for a variety of professional scenarios where WiFi intelligence is critical.

| Use Case             | Description                                                                                                                               |
|----------------------|-------------------------------------------------------------------------------------------------------------------------------------------|
| **Perimeter Security**   | Deploy the Wi-Dar to continuously monitor a secure area (e.g., a corporate office, SCIF, or event venue) for unauthorized or rogue WiFi access points. |
| **Counter-Surveillance** | Conduct sweeps to detect and identify potentially malicious APs used for eavesdropping or man-in-the-middle attacks.                       |
| **Network Analysis**     | Map the WiFi landscape of a given environment, analyze signal strength, and identify all active transponders for security audits.          |
| **Training & Simulation**| Provide a safe and effective platform for training personnel in WiFi surveillance and intelligence-gathering techniques without complex setup. |

---

## 3. Getting Started

Getting the Spec5 Wi-Dar operational takes less than a minute.

### 3.1. Powering On

Connect the **Spec5 Wi-Dar USB dongle** to any standard 5V USB port. This can be a laptop, a dedicated power bank, or a USB wall adapter. The device will automatically boot up and initialize its systems.

### 3.2. Connecting to the Device

The Wi-Dar operates in Access Point (AP) mode, creating its own secure WiFi network. Use a laptop, tablet, or smartphone to connect to it:

- **SSID (Network Name):** `Wi-Dar`
- **Password:** `tactical123`

### 3.3. Accessing the Web Interface

Once connected to the "Wi-Dar" WiFi network, open a web browser and navigate to the following address:

**http://192.168.4.1**

This will load the main Heads-Up Display (HUD) and tactical interface.

---

## 4. The Web Interface (HUD)

The Wi-Dar interface is designed for clarity and immediate data comprehension, featuring a Matrix-inspired green-on-black tactical theme.

```
┌─────────────────────────────────────────────────────┐
│ [Spec5 Logo] WI-DAR              [HUD Stats]        │
├─────────────────────────────────────────────────────┤
│ RADAR | SIGNAL | TAGS | CONFIG                      │
├─────────────────────────────────────────────────────┤
│                                                      │
│           [Main Content Area]                        │
│                                                      │
├─────────────────────────────────────────────────────┤
│ ┌─ LOG SEARCH ─────────────────────────────────┐   │
│ │ SEARCH: [_______________] ◀ ▶ [2 of 5]       │   │
│ ├───────────────────────────────────────────────┤   │
│ │ [12:34:56] SYSTEM INITIALIZED                 │   │
│ │ [scrollable log area...]                      │   │
│ └───────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────┘
```

- **Header:** Displays the Spec5 logo, system name, and key operational statistics (Total Contacts, Threat Levels, Uptime, etc.).
- **Navigation Tabs:** Switch between the four main operational views: RADAR, SIGNAL, TAGS, and CONFIG.
- **Log Viewer:** A persistent log at the bottom of the screen provides real-time updates on system events and detections.

---

## 5. Features & Operation

### 5.1. RADAR View

The primary operational view. It features an 8-slice radar display that visualizes all detected WiFi APs in real-time. The position on the radar is determined by signal strength (RSSI), with stronger signals appearing closer to the center.

- **IFF Symbols (Identify Friend or Foe):** APs are marked with symbols based on their threat level, as defined in the TAGS view.
  - **○ Circle:** Neutral/Unknown
  - **△ Triangle:** Suspicious
  - **□ Square:** Hostile

### 5.2. SIGNAL View

This view provides real-time signal strength graphs for all tracked APs. Each AP has its own dedicated graph plotting its RSSI value over time. This is useful for:

- **Monitoring Proximity:** A consistently strengthening signal indicates the AP is getting closer.
- **Identifying Movement:** Fluctuations can indicate a moving transponder.

### 5.3. TAGS (Tagged Contacts) View

This is the intelligence hub of the Wi-Dar. Here, you can classify and manage contacts.

- **Adding a Tag:** Define a contact by giving it a **Name**, an **SSID Pattern**, a **Color**, and a **Threat Level** (Hostile, Suspicious, Neutral, Friendly).
- **SSID Patterns:** Use wildcards (`*`) to tag multiple APs. For example, `Guest*` will tag any AP whose SSID starts with "Guest".
- **Alerts:** When a tagged contact appears on the network, a prominent ASCII art alert box is displayed for 10 seconds, ensuring you never miss a critical event.

### 5.4. CONFIG View

Customize the system's operational parameters.

- **Mission Profiles:** Select a pre-configured profile to optimize the system for a specific task (Surveillance, Recon, Counter-Intel, Training).
- **System Settings:** Fine-tune the scanner's behavior:
  - **Track History:** Number of historical data points to keep for signal graphs.
  - **Validation Threshold:** Number of scans an AP must be seen in to be considered "active".
  - **Scan Interval:** Time in seconds between each WiFi scan.
  - **Proximity Alert:** Distance in meters at which to trigger a proximity alert for tagged contacts.
- **Session Management:**
  - **Save Configuration:** Persistently save all current settings and tagged contacts to the device's memory.
  - **Export Session Log:** Download a JSON file containing all events from the current session for offline analysis.

### 5.5. Log Search

Located at the bottom of the interface, the log search allows for efficient analysis of the session log.

- **Search:** Type a term to instantly highlight all matching log entries.
- **Navigate:** Use the **◀** and **▶** buttons or press **Enter** to jump between matches.
- **Match Counter:** Displays the total number of matches and your current position (e.g., "3 of 10").

---

## 6. Keyboard Shortcuts

For rapid, heads-up operation, the interface can be controlled via keyboard shortcuts.

| Key   | Action                  |
|-------|-------------------------|
| `SPACE` | Pause/Resume live scanning |
| `R`     | Reset all tracked contacts |
| `T`     | Toggle threat overlay on radar |
| `M`     | Mute/Unmute all audio alerts |
| `F`     | Enter/Exit Fullscreen mode |
| `1`-`4` | Switch between tabs (1=RADAR, 2=SIGNAL, etc.) |
| `?`     | Toggle the shortcuts help display |

---

## 7. Troubleshooting

- **Web Page Does Not Load:** Ensure you are connected to the "Wi-Dar" WiFi network and are using the correct IP address: `http://192.168.4.1`.
- **Page is Blank or Broken:** Your browser may have cached an old version. Perform a hard refresh (Ctrl+Shift+R or Cmd+Shift+R) or clear your browser's cache.
- **No Networks Detected:** Ensure you are in an area with 2.4 GHz WiFi networks. Check the `Scan Interval` in the CONFIG view to make sure it is not set too high.

---

## 8. Disclaimer

The Spec5 Wi-Dar is a professional tool intended for authorized use only. Users are responsible for complying with all applicable laws and regulations regarding electronic surveillance and privacy. SpecFive, Inc. assumes no liability for misuse of this product.

For more information, support, and other tactical solutions, visit **[specfive.com](https://www.specfive.com)**.
