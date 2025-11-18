#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <Preferences.h>

WebServer server(80);
Preferences prefs;

const char* ssid = "Wi-Dar";
const char* password = "tactical123";

// Configuration
struct Config {
  int trackHistory = 5;
  int validationThreshold = 3;
  int scanInterval = 3;
  int proximityAlert = 5;
  String missionProfile = "perimeter";
} config;

// Tagged contacts storage
struct TaggedContact {
  String name;
  String pattern;
  String color;
  bool soundEnabled;
};
std::vector<TaggedContact> taggedContacts;

void loadConfig() {
  prefs.begin("widar", false);
  config.trackHistory = prefs.getInt("trackHist", 5);
  config.validationThreshold = prefs.getInt("valThresh", 3);
  config.scanInterval = prefs.getInt("scanInt", 3);
  config.proximityAlert = prefs.getInt("proxAlert", 5);
  config.missionProfile = prefs.getString("profile", "perimeter");
  
  // Load tagged contacts
  int tagCount = prefs.getInt("tagCount", 0);
  for (int i = 0; i < tagCount; i++) {
    TaggedContact tag;
    tag.name = prefs.getString(("tagN" + String(i)).c_str(), "");
    tag.pattern = prefs.getString(("tagP" + String(i)).c_str(), "");
    tag.color = prefs.getString(("tagC" + String(i)).c_str(), "red");
    tag.soundEnabled = prefs.getBool(("tagS" + String(i)).c_str(), true);
    if (tag.name.length() > 0) {
      taggedContacts.push_back(tag);
    }
  }
  prefs.end();
}

void saveConfig() {
  prefs.begin("widar", false);
  prefs.putInt("trackHist", config.trackHistory);
  prefs.putInt("valThresh", config.validationThreshold);
  prefs.putInt("scanInt", config.scanInterval);
  prefs.putInt("proxAlert", config.proximityAlert);
  prefs.putString("profile", config.missionProfile);
  
  // Save tagged contacts
  prefs.putInt("tagCount", taggedContacts.size());
  for (size_t i = 0; i < taggedContacts.size(); i++) {
    prefs.putString(("tagN" + String(i)).c_str(), taggedContacts[i].name);
    prefs.putString(("tagP" + String(i)).c_str(), taggedContacts[i].pattern);
    prefs.putString(("tagC" + String(i)).c_str(), taggedContacts[i].color);
    prefs.putBool(("tagS" + String(i)).c_str(), taggedContacts[i].soundEnabled);
  }
  prefs.end();
}

void handleScanAPI() {
  JsonDocument doc;
  JsonArray networks = doc["networks"].to<JsonArray>();
  
  int n = WiFi.scanNetworks(false, false, false, 300);
  
  for (int i = 0; i < n; i++) {
    JsonObject net = networks.add<JsonObject>();
    net["ssid"] = WiFi.SSID(i);
    net["rssi"] = WiFi.RSSI(i);
    net["channel"] = WiFi.channel(i);
    net["bssid"] = WiFi.BSSIDstr(i);
    net["encryption"] = (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "Open" : "Encrypted";
  }
  
  doc["timestamp"] = millis();
  doc["count"] = n;
  doc["uptime"] = millis() / 1000;
  doc["freeHeap"] = ESP.getFreeHeap();
  doc["temperature"] = temperatureRead();
  
  String json;
  serializeJson(doc, json);
  server.send(200, "application/json", json);
}

void handleConfigAPI() {
  if (server.method() == HTTP_GET) {
    JsonDocument doc;
    doc["trackHistory"] = config.trackHistory;
    doc["validationThreshold"] = config.validationThreshold;
    doc["scanInterval"] = config.scanInterval;
    doc["proximityAlert"] = config.proximityAlert;
    doc["missionProfile"] = config.missionProfile;
    
    String json;
    serializeJson(doc, json);
    server.send(200, "application/json", json);
  } else if (server.method() == HTTP_POST) {
    JsonDocument doc;
    deserializeJson(doc, server.arg("plain"));
    
    config.trackHistory = doc["trackHistory"] | 5;
    config.validationThreshold = doc["validationThreshold"] | 3;
    config.scanInterval = doc["scanInterval"] | 3;
    config.proximityAlert = doc["proximityAlert"] | 5;
    config.missionProfile = doc["missionProfile"] | "perimeter";
    
    saveConfig();
    server.send(200, "application/json", "{\"status\":\"ok\"}");
  }
}

void handleTagsAPI() {
  if (server.method() == HTTP_GET) {
    JsonDocument doc;
    JsonArray tags = doc["tags"].to<JsonArray>();
    
    for (const auto& tag : taggedContacts) {
      JsonObject t = tags.add<JsonObject>();
      t["name"] = tag.name;
      t["pattern"] = tag.pattern;
      t["color"] = tag.color;
      t["sound"] = tag.soundEnabled;
    }
    
    String json;
    serializeJson(doc, json);
    server.send(200, "application/json", json);
  } else if (server.method() == HTTP_POST) {
    JsonDocument doc;
    deserializeJson(doc, server.arg("plain"));
    
    TaggedContact tag;
    tag.name = doc["name"].as<String>();
    tag.pattern = doc["pattern"].as<String>();
    tag.color = doc["color"].as<String>();
    tag.soundEnabled = doc["sound"] | true;
    
    taggedContacts.push_back(tag);
    saveConfig();
    
    server.send(200, "application/json", "{\"status\":\"ok\"}");
  } else if (server.method() == HTTP_DELETE) {
    JsonDocument doc;
    deserializeJson(doc, server.arg("plain"));
    
    String name = doc["name"];
    taggedContacts.erase(
      std::remove_if(taggedContacts.begin(), taggedContacts.end(),
        [&name](const TaggedContact& t) { return t.name == name; }),
      taggedContacts.end()
    );
    
    saveConfig();
    server.send(200, "application/json", "{\"status\":\"ok\"}");
  }
}

void handleRoot() {
  server.send(200, "text/html", R"html(<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1.0">
<title>Spec5 Wi-Dar</title>
<style>
*{margin:0;padding:0;box-sizing:border-box}
body{background:#000;color:#0f0;font-family:'Courier New',monospace;overflow:hidden;user-select:none}
#hud{position:fixed;top:0;left:0;right:0;height:50px;background:linear-gradient(180deg,#001100,#000);border-bottom:2px solid #0f0;display:flex;justify-content:space-between;align-items:center;padding:0 20px;z-index:1000}
#hud-left{display:flex;gap:30px}
#hud-right{display:flex;gap:20px;font-size:12px}
.hud-stat{display:flex;flex-direction:column;align-items:center}
.hud-label{font-size:9px;color:#0a0}
.hud-value{font-size:16px;font-weight:bold}
.threat-hostile{color:#f00}
.threat-suspicious{color:#f80}
.threat-neutral{color:#ff0}
.threat-friendly{color:#0f0}
#tabs{position:fixed;top:50px;left:0;right:0;height:40px;background:#001100;border-bottom:1px solid #0f0;display:flex;z-index:999}
.tab{flex:1;display:flex;align-items:center;justify-content:center;cursor:pointer;border-right:1px solid #003300;transition:background .2s}
.tab:hover{background:#002200}
.tab.active{background:#003300;border-bottom:2px solid #0ff}
#content{position:fixed;top:90px;left:0;right:0;bottom:150px;overflow:hidden}
.view{display:none;width:100%;height:100%}
.view.active{display:block}
#radar-view{position:relative}
#radar-canvas{width:100%;height:100%;background:#000}
#signal-view{padding:20px}
#signal-canvas{width:100%;height:100%;background:#000;border:1px solid #0f0}
#config-view{padding:20px;overflow-y:auto}
.config-section{margin-bottom:30px}
.config-section h3{color:#0ff;margin-bottom:15px;border-bottom:1px solid #0f0;padding-bottom:5px}
.config-row{display:flex;justify-content:space-between;align-items:center;margin:10px 0;padding:8px;background:#001100;border:1px solid #003300}
.config-row label{flex:1}
.config-row input,.config-row select{background:#000;color:#0f0;border:1px solid #0f0;padding:5px 10px;font-family:inherit;width:150px}
.config-row button{background:#003300;color:#0f0;border:1px solid #0f0;padding:5px 15px;cursor:pointer;font-family:inherit}
.config-row button:hover{background:#005500}
#tags-view{padding:20px}
#tags-table{width:100%;border-collapse:collapse;margin-top:20px}
#tags-table th,#tags-table td{border:1px solid #0f0;padding:10px;text-align:left}
#tags-table th{background:#003300;color:#0ff}
#tags-table tr:hover{background:#001100}
.tag-color{width:20px;height:20px;border:1px solid #0f0;display:inline-block}
.btn-delete{background:#300;color:#f00;border:1px solid #f00;padding:3px 10px;cursor:pointer;font-family:inherit;font-size:11px}
.btn-delete:hover{background:#500}
#add-tag-form{background:#001100;border:2px solid #0f0;padding:20px;margin-bottom:20px}
#add-tag-form input,#add-tag-form select{background:#000;color:#0f0;border:1px solid #0f0;padding:8px;margin:5px;font-family:inherit}
#add-tag-form button{background:#003300;color:#0f0;border:1px solid #0f0;padding:8px 20px;cursor:pointer;font-family:inherit;margin:5px}
#add-tag-form button:hover{background:#005500}
#message-log{position:fixed;bottom:0;left:0;right:0;height:150px;background:rgba(0,10,0,0.95);border-top:2px solid #0f0;overflow-y:auto;padding:10px;font-size:12px;z-index:998}
.log-entry{margin:3px 0;padding:3px 5px;border-left:3px solid #0f0}
.log-new{border-left-color:#ff0;color:#ff0}
.log-tagged{border-left-color:#f00;color:#f00;font-weight:bold}
.log-lost{border-left-color:#666;color:#888}
.log-stable{border-left-color:#0f0}
#shortcuts-help{position:fixed;top:100px;right:20px;background:rgba(0,20,0,0.9);border:2px solid #0f0;padding:15px;display:none;z-index:2000;font-size:11px}
#shortcuts-help.show{display:block}
</style>
</head>
<body>
<div id="hud">
<div id="hud-left">
<div style="font-size:20px;font-weight:bold;color:#0ff">SPEC5 WI-DAR</div>
<div class="hud-stat"><span class="hud-label">CONTACTS</span><span class="hud-value" id="hud-total">0</span></div>
<div class="hud-stat"><span class="hud-label threat-hostile">HOSTILE</span><span class="hud-value threat-hostile" id="hud-hostile">0</span></div>
<div class="hud-stat"><span class="hud-label threat-suspicious">SUSPICIOUS</span><span class="hud-value threat-suspicious" id="hud-suspicious">0</span></div>
<div class="hud-stat"><span class="hud-label threat-neutral">NEUTRAL</span><span class="hud-value threat-neutral" id="hud-neutral">0</span></div>
<div class="hud-stat"><span class="hud-label threat-friendly">FRIENDLY</span><span class="hud-value threat-friendly" id="hud-friendly">0</span></div>
</div>
<div id="hud-right">
<div class="hud-stat"><span class="hud-label">UPTIME</span><span class="hud-value" id="hud-uptime">00:00:00</span></div>
<div class="hud-stat"><span class="hud-label">PROFILE</span><span class="hud-value" id="hud-profile">PERIMETER</span></div>
<div class="hud-stat"><span class="hud-label">LAST SCAN</span><span class="hud-value" id="hud-lastscan">--</span></div>
</div>
</div>
<div id="tabs">
<div class="tab active" data-tab="radar">RADAR</div>
<div class="tab" data-tab="signal">SIGNAL STRENGTH</div>
<div class="tab" data-tab="config">CONFIGURATION</div>
<div class="tab" data-tab="tags">TAGGED CONTACTS</div>
</div>
<div id="content">
<div id="radar-view" class="view active">
<canvas id="radar-canvas"></canvas>
</div>
<div id="signal-view" class="view">
<canvas id="signal-canvas"></canvas>
</div>
<div id="config-view" class="view">
<div class="config-section">
<h3>TRACK SETTINGS</h3>
<div class="config-row"><label>Track History Count:</label><input type="number" id="cfg-track-hist" min="1" max="10" value="5"></div>
<div class="config-row"><label>Validation Threshold:</label><input type="number" id="cfg-val-thresh" min="1" max="5" value="3"></div>
<div class="config-row"><label>Scan Interval (seconds):</label><input type="number" id="cfg-scan-int" min="1" max="10" value="3"></div>
<div class="config-row"><label>Proximity Alert (meters):</label><input type="number" id="cfg-prox-alert" min="1" max="50" value="5"></div>
</div>
<div class="config-section">
<h3>MISSION PROFILE</h3>
<div class="config-row">
<label>Active Profile:</label>
<select id="cfg-profile">
<option value="perimeter">Perimeter Security</option>
<option value="survey">Network Survey</option>
<option value="hunting">Threat Hunting</option>
<option value="training">Training Mode</option>
</select>
</div>
</div>
<div class="config-section">
<h3>SYSTEM DIAGNOSTICS</h3>
<div class="config-row"><label>Free Heap:</label><span id="diag-heap">--</span></div>
<div class="config-row"><label>Temperature:</label><span id="diag-temp">--</span></div>
<div class="config-row"><label>Scan Success Rate:</label><span id="diag-success">--</span></div>
</div>
<div class="config-section">
<button onclick="saveConfiguration()" style="background:#003300;color:#0f0;border:1px solid #0f0;padding:10px 30px;cursor:pointer;font-family:inherit;font-size:14px">SAVE CONFIGURATION</button>
<button onclick="exportSession()" style="background:#003300;color:#0ff;border:1px solid #0ff;padding:10px 30px;cursor:pointer;font-family:inherit;font-size:14px;margin-left:10px">EXPORT SESSION LOG</button>
</div>
</div>
<div id="tags-view" class="view">
<div id="add-tag-form">
<h3 style="color:#0ff;margin-bottom:10px">ADD TAGGED CONTACT</h3>
<input type="text" id="tag-name" placeholder="Name (e.g., Home Router)" style="width:200px">
<input type="text" id="tag-pattern" placeholder="Pattern (e.g., Linksys* or *5G*)" style="width:250px">
<select id="tag-color" style="width:120px">
<option value="red">Red</option>
<option value="orange">Orange</option>
<option value="yellow">Yellow</option>
<option value="green">Green</option>
<option value="cyan">Cyan</option>
<option value="blue">Blue</option>
<option value="purple">Purple</option>
<option value="magenta">Magenta</option>
<option value="white">White</option>
<option value="pink">Pink</option>
</select>
<label style="color:#0f0;margin-left:10px"><input type="checkbox" id="tag-sound" checked> Sound Alert</label>
<button onclick="addTag()">ADD TAG</button>
</div>
<table id="tags-table">
<thead><tr><th>NAME</th><th>PATTERN</th><th>COLOR</th><th>SOUND</th><th>ACTIONS</th></tr></thead>
<tbody id="tags-tbody"></tbody>
</table>
</div>
</div>
<div id="message-log"></div>
<div id="shortcuts-help">
<div style="color:#0ff;font-weight:bold;margin-bottom:10px">KEYBOARD SHORTCUTS</div>
<div>SPACE - Pause/Resume</div>
<div>R - Reset Tracks</div>
<div>T - Toggle Threat Overlay</div>
<div>M - Mute/Unmute</div>
<div>F - Fullscreen</div>
<div>1-4 - Switch Tabs</div>
<div>? - Toggle This Help</div>
</div>
<script>
// Global state
const tracks = new Map();
const sessionLog = [];
const signalHistory = new Map(); // Store signal history for graph
let config = {trackHistory:5,validationThreshold:3,scanInterval:3,proximityAlert:5,missionProfile:'perimeter'};
let taggedContacts = [];
let isPaused = false;
let isMuted = false;
let scanCount = 0;
let successCount = 0;
let lastScanTime = 0;
let totalUniqueSSIDs = new Set();
let taggedWarning = null; // {ssid, name, timestamp}

// Audio context for alerts
const audioCtx = new (window.AudioContext || window.webkitAudioContext)();

function playAlert(frequency = 600, duration = 200) {
  if (isMuted) return;
  const osc = audioCtx.createOscillator();
  const gain = audioCtx.createGain();
  osc.connect(gain);
  gain.connect(audioCtx.destination);
  osc.frequency.value = frequency;
  gain.gain.value = 0.3;
  osc.start();
  setTimeout(() => osc.stop(), duration);
}

function logMessage(msg, type = 'normal') {
  const log = document.getElementById('message-log');
  const time = new Date().toLocaleTimeString();
  const entry = document.createElement('div');
  entry.className = 'log-entry log-' + type;
  entry.textContent = `[${time}] ${msg}`;
  log.appendChild(entry);
  log.scrollTop = log.scrollHeight;
  
  sessionLog.push({timestamp: Date.now(), message: msg, type: type});
  if (sessionLog.length > 1000) sessionLog.shift();
}

function matchesPattern(ssid, pattern) {
  const regex = new RegExp('^' + pattern.replace(/\*/g, '.*') + '$', 'i');
  return regex.test(ssid);
}

function getThreatLevel(track) {
  for (const tag of taggedContacts) {
    if (matchesPattern(track.ssid, tag.pattern)) {
      return track.rssi > -50 ? 'hostile' : 'suspicious';
    }
  }
  
  const knownSSIDs = Array.from(tracks.values()).filter(t => t.isConfirmed).map(t => t.ssid);
  const duplicates = knownSSIDs.filter(s => s === track.ssid).length;
  if (duplicates > 1 && track.rssi > -40) return 'suspicious';
  
  if (track.history.length >= 3) {
    const rssis = track.history.map(h => h.rssi);
    const variance = rssis.reduce((sum, r) => sum + Math.pow(r - track.rssi, 2), 0) / rssis.length;
    if (variance > 100) return 'suspicious';
  }
  
  if (track.isConfirmed) {
    return track.rssi > -60 ? 'neutral' : 'passive';
  }
  return 'neutral';
}

function estimateDistance(rssi) {
  const txPower = -30;
  const pathLossExp = 2.5;
  const distance = Math.pow(10, (txPower - rssi) / (10 * pathLossExp));
  return Math.max(0.1, Math.min(100, distance));
}

function updateHUD() {
  const counts = {hostile:0, suspicious:0, neutral:0, friendly:0, passive:0};
  tracks.forEach(track => {
    if (track.isConfirmed) {
      const threat = getThreatLevel(track);
      counts[threat] = (counts[threat] || 0) + 1;
    }
  });
  
  document.getElementById('hud-total').textContent = tracks.size;
  document.getElementById('hud-hostile').textContent = counts.hostile || 0;
  document.getElementById('hud-suspicious').textContent = counts.suspicious || 0;
  document.getElementById('hud-neutral').textContent = counts.neutral || 0;
  document.getElementById('hud-friendly').textContent = counts.friendly || 0;
  
  const now = Date.now();
  if (lastScanTime > 0) {
    const elapsed = ((now - lastScanTime) / 1000).toFixed(1);
    document.getElementById('hud-lastscan').textContent = elapsed + 's';
  }
}

async function scanWiFi() {
  if (isPaused) return;
  
  scanCount++;
  try {
    const resp = await fetch('/api/scan');
    const data = await resp.json();
    successCount++;
    lastScanTime = Date.now();
    
    document.getElementById('diag-heap').textContent = (data.freeHeap / 1024).toFixed(1) + ' KB';
    document.getElementById('diag-temp').textContent = data.temperature.toFixed(1) + ' °C';
    document.getElementById('diag-success').textContent = ((successCount / scanCount) * 100).toFixed(1) + '%';
    
    const uptime = data.uptime;
    const hours = Math.floor(uptime / 3600);
    const mins = Math.floor((uptime % 3600) / 60);
    const secs = uptime % 60;
    document.getElementById('hud-uptime').textContent = 
      String(hours).padStart(2,'0') + ':' + 
      String(mins).padStart(2,'0') + ':' + 
      String(secs).padStart(2,'0');
    
    const currentSSIDs = new Set();
    const currentTime = Date.now();
    
    data.networks.forEach(net => {
      currentSSIDs.add(net.bssid);
      totalUniqueSSIDs.add(net.ssid);
      
      // Update signal history for graph
      if (!signalHistory.has(net.bssid)) {
        signalHistory.set(net.bssid, []);
      }
      const history = signalHistory.get(net.bssid);
      history.push({rssi: net.rssi, timestamp: currentTime, ssid: net.ssid});
      // Keep last 60 readings (about 3 minutes at 3s interval)
      if (history.length > 60) history.shift();
      
      if (!tracks.has(net.bssid)) {
        tracks.set(net.bssid, {
          ssid: net.ssid,
          rssi: net.rssi,
          channel: net.channel,
          bssid: net.bssid,
          history: [],
          firstSeen: Date.now(),
          lastSeen: Date.now(),
          validationCount: 1,
          isConfirmed: false,
          distance: estimateDistance(net.rssi)
        });
      } else {
        const track = tracks.get(net.bssid);
        track.rssi = net.rssi;
        track.lastSeen = Date.now();
        track.distance = estimateDistance(net.rssi);
        
        if (!track.isConfirmed) {
          track.validationCount++;
          if (track.validationCount >= config.validationThreshold) {
            track.isConfirmed = true;
            logMessage(`NEW TRANSPONDER IDENTIFIED: ${track.ssid} | RSSI: ${track.rssi} dBm | RNG: ${track.distance.toFixed(1)}m`, 'new');
            
            for (const tag of taggedContacts) {
              if (matchesPattern(track.ssid, tag.pattern)) {
                logMessage(`⚠ TAGGED CONTACT APPEARED: ${tag.name} (${track.ssid}) | RSSI: ${track.rssi} dBm`, 'tagged');
                taggedWarning = {ssid: track.ssid, name: tag.name, timestamp: Date.now()};
                if (tag.soundEnabled) {
                  playAlert(800, 300);
                  setTimeout(() => playAlert(800, 300), 400);
                }
                break;
              }
            }
            
            if (track.distance < config.proximityAlert) {
              logMessage(`⚠ PROXIMITY ALERT: ${track.ssid} within ${track.distance.toFixed(1)}m`, 'tagged');
              playAlert(1000, 200);
            }
          }
        }
        
        track.history.push({rssi: track.rssi, timestamp: Date.now()});
        if (track.history.length > config.trackHistory) {
          track.history.shift();
        }
      }
    });
    
    tracks.forEach((track, bssid) => {
      if (!currentSSIDs.has(bssid)) {
        const age = Date.now() - track.lastSeen;
        if (age > 30000) {
          logMessage(`TRANSPONDER LOST: ${track.ssid} | Last seen: ${new Date(track.lastSeen).toLocaleTimeString()}`, 'lost');
          tracks.delete(bssid);
          signalHistory.delete(bssid);
        }
      }
    });
    
    updateHUD();
    drawSignalGraph();
    
  } catch (e) {
    console.error('Scan failed:', e);
  }
  
  setTimeout(scanWiFi, config.scanInterval * 1000);
}

// Radar rendering with enhancements
function drawRadar() {
  const canvas = document.getElementById('radar-canvas');
  const ctx = canvas.getContext('2d');
  
  canvas.width = canvas.offsetWidth;
  canvas.height = canvas.offsetHeight;
  
  const cx = canvas.width / 2;
  const cy = canvas.height / 2;
  const radius = Math.min(cx, cy) - 80;
  
  ctx.fillStyle = '#000';
  ctx.fillRect(0, 0, canvas.width, canvas.height);
  
  // Draw range rings with glow
  ctx.shadowBlur = 15;
  ctx.shadowColor = '#0f0';
  ctx.strokeStyle = '#003300';
  ctx.lineWidth = 1;
  for (let i = 1; i <= 4; i++) {
    ctx.beginPath();
    ctx.arc(cx, cy, radius * i / 4, 0, Math.PI * 2);
    ctx.stroke();
    
    ctx.shadowBlur = 0;
    ctx.fillStyle = '#0a0';
    ctx.font = '10px monospace';
    ctx.fillText((i * 25) + '%', cx + radius * i / 4 + 5, cy);
    ctx.shadowBlur = 15;
  }
  
  // Draw 8-slice azimuth lines with glow
  ctx.strokeStyle = '#003300';
  for (let i = 0; i < 8; i++) {
    const angle = (i * 45) * Math.PI / 180;
    ctx.beginPath();
    ctx.moveTo(cx, cy);
    ctx.lineTo(cx + radius * Math.cos(angle - Math.PI/2), cy + radius * Math.sin(angle - Math.PI/2));
    ctx.stroke();
  }
  
  ctx.shadowBlur = 0;
  
  // Draw azimuth markers (8 directions)
  ctx.fillStyle = '#0f0';
  ctx.font = '12px monospace';
  const labels = ['N', 'NE', 'E', 'SE', 'S', 'SW', 'W', 'NW'];
  for (let i = 0; i < 8; i++) {
    const angle = (i * 45) * Math.PI / 180;
    const x = cx + (radius + 20) * Math.cos(angle - Math.PI/2);
    const y = cy + (radius + 20) * Math.sin(angle - Math.PI/2);
    ctx.fillText(labels[i], x - 10, y + 5);
  }
  
  // Draw sweep line with enhanced glow
  const sweepAngle = (Date.now() / 20) % 360;
  ctx.shadowBlur = 30;
  ctx.shadowColor = '#0f0';
  ctx.strokeStyle = 'rgba(0,255,0,0.6)';
  ctx.lineWidth = 3;
  ctx.beginPath();
  ctx.moveTo(cx, cy);
  const sweepX = cx + radius * Math.cos((sweepAngle - 90) * Math.PI / 180);
  const sweepY = cy + radius * Math.sin((sweepAngle - 90) * Math.PI / 180);
  ctx.lineTo(sweepX, sweepY);
  ctx.stroke();
  ctx.shadowBlur = 0;
  
  // Draw tracks
  tracks.forEach(track => {
    if (!track.isConfirmed) return;
    
    const normalizedRSSI = (track.rssi + 100) / 70;
    const distFromCenter = radius * (1 - Math.max(0, Math.min(1, normalizedRSSI)));
    
    const hash = track.bssid.split(':').reduce((sum, byte) => sum + parseInt(byte, 16), 0);
    const angle = (hash % 360) * Math.PI / 180;
    
    const x = cx + distFromCenter * Math.cos(angle);
    const y = cy + distFromCenter * Math.sin(angle);
    
    const threat = getThreatLevel(track);
    const colors = {hostile: '#f00', suspicious: '#f80', neutral: '#ff0', friendly: '#0f0', passive: '#888'};
    const color = colors[threat] || '#0f0';
    
    if (track.history.length > 1) {
      ctx.strokeStyle = color;
      ctx.lineWidth = 1;
      ctx.setLineDash([2, 2]);
      ctx.beginPath();
      for (let i = 0; i < track.history.length; i++) {
        const h = track.history[i];
        const hNorm = (h.rssi + 100) / 70;
        const hDist = radius * (1 - Math.max(0, Math.min(1, hNorm)));
        const hx = cx + hDist * Math.cos(angle);
        const hy = cy + hDist * Math.sin(angle);
        if (i === 0) ctx.moveTo(hx, hy);
        else ctx.lineTo(hx, hy);
      }
      ctx.stroke();
      ctx.setLineDash([]);
    }
    
    for (let i = 0; i < track.history.length; i++) {
      const h = track.history[i];
      const age = track.history.length - i;
      const opacity = 1 - (age / config.trackHistory) * 0.8;
      const size = 8 - age;
      
      const hNorm = (h.rssi + 100) / 70;
      const hDist = radius * (1 - Math.max(0, Math.min(1, hNorm)));
      const hx = cx + hDist * Math.cos(angle);
      const hy = cy + hDist * Math.sin(angle);
      
      ctx.fillStyle = color + Math.floor(opacity * 255).toString(16).padStart(2, '0');
      ctx.beginPath();
      ctx.arc(hx, hy, size, 0, Math.PI * 2);
      ctx.fill();
    }
    
    ctx.fillStyle = color;
    ctx.strokeStyle = color;
    ctx.lineWidth = 2;
    
    if (threat === 'hostile') {
      ctx.beginPath();
      ctx.moveTo(x, y - 8);
      ctx.lineTo(x - 7, y + 6);
      ctx.lineTo(x + 7, y + 6);
      ctx.closePath();
      ctx.fill();
    } else if (threat === 'friendly') {
      ctx.fillRect(x - 6, y - 6, 12, 12);
    } else {
      ctx.beginPath();
      ctx.arc(x, y, 6, 0, Math.PI * 2);
      ctx.fill();
    }
    
    ctx.fillStyle = color;
    ctx.font = '10px monospace';
    const bearing = Math.floor((angle * 180 / Math.PI + 90) % 360);
    const label = `${track.ssid.substring(0, 15)} | ${track.rssi}dBm | BRG ${bearing}° RNG ${track.distance.toFixed(1)}m`;
    ctx.fillText(label, x + 10, y - 10);
  });
  
  // ASCII art corner displays
  ctx.font = '14px monospace';
  ctx.fillStyle = '#0f0';
  
  // Top left: Contact count
  const contactCount = tracks.size;
  const contactStr = 'CONTACTS: ' + String(contactCount).padStart(3, '0');
  const contactWidth = contactStr.length + 2;
  ctx.fillText('╔' + '═'.repeat(contactWidth) + '╗', 10, 20);
  ctx.fillText('║ ' + contactStr + ' ║', 10, 35);
  ctx.fillText('╚' + '═'.repeat(contactWidth) + '╝', 10, 50);
  
  // Bottom left: Unique SSIDs
  const uniqueStr = 'UNIQUE: ' + String(totalUniqueSSIDs.size).padStart(5, '0');
  const uniqueWidth = uniqueStr.length + 2;
  ctx.fillText('╔' + '═'.repeat(uniqueWidth) + '╗', 10, canvas.height - 50);
  ctx.fillText('║ ' + uniqueStr + ' ║', 10, canvas.height - 35);
  ctx.fillText('╚' + '═'.repeat(uniqueWidth) + '╝', 10, canvas.height - 20);
  
  // Bottom right: Live time
  const now = new Date();
  const timeStr = 'TIME: ' + now.toLocaleTimeString();
  const timeWidth = timeStr.length + 2;
  const timeX = canvas.width - (timeWidth + 2) * 8.4 - 10;
  ctx.fillText('╔' + '═'.repeat(timeWidth) + '╗', timeX, canvas.height - 50);
  ctx.fillText('║ ' + timeStr + ' ║', timeX, canvas.height - 35);
  ctx.fillText('╚' + '═'.repeat(timeWidth) + '╝', timeX, canvas.height - 20);
  
  // Top right: Tagged warning (if active)
  if (taggedWarning && (Date.now() - taggedWarning.timestamp < 4000)) {
    ctx.fillStyle = '#f00';
    ctx.font = 'bold 16px monospace';
    const warningLine1 = '⚠ TAGGED CONTACT ⚠';
    const warningLine2 = taggedWarning.name.substring(0, 19);
    const warningWidth = Math.max(warningLine1.length, warningLine2.length) + 2;
    const warningX = canvas.width - (warningWidth + 2) * 9.6 - 10;
    ctx.fillText('╔' + '═'.repeat(warningWidth) + '╗', warningX, 20);
    ctx.fillText('║ ' + warningLine1.padEnd(warningWidth, ' ') + ' ║', warningX, 40);
    ctx.fillText('║ ' + warningLine2.padEnd(warningWidth, ' ') + ' ║', warningX, 60);
    ctx.fillText('╚' + '═'.repeat(warningWidth) + '╝', warningX, 80);
  } else if (taggedWarning && (Date.now() - taggedWarning.timestamp >= 4000)) {
    taggedWarning = null;
  }
  
  if (document.getElementById('radar-view').classList.contains('active')) {
    requestAnimationFrame(drawRadar);
  }
}

// Fixed signal strength graph
function drawSignalGraph() {
  const canvas = document.getElementById('signal-canvas');
  const ctx = canvas.getContext('2d');
  
  canvas.width = canvas.offsetWidth;
  canvas.height = canvas.offsetHeight;
  
  ctx.fillStyle = '#000';
  ctx.fillRect(0, 0, canvas.width, canvas.height);
  
  // Get all current signal histories
  const activeHistories = Array.from(signalHistory.entries())
    .filter(([bssid, history]) => history.length > 0)
    .slice(0, 7); // Limit to 7 for visibility
  
  if (activeHistories.length === 0) return;
  
  // Find min/max RSSI for dynamic scaling
  let minRSSI = -100;
  let maxRSSI = -30;
  activeHistories.forEach(([bssid, history]) => {
    history.forEach(h => {
      minRSSI = Math.min(minRSSI, h.rssi);
      maxRSSI = Math.max(maxRSSI, h.rssi);
    });
  });
  
  // Add padding
  const padding = (maxRSSI - minRSSI) * 0.1 || 10;
  minRSSI -= padding;
  maxRSSI += padding;
  
  const leftMargin = 60;
  const rightMargin = 20;
  const topMargin = 30;
  const bottomMargin = 40;
  const graphWidth = canvas.width - leftMargin - rightMargin;
  const graphHeight = canvas.height - topMargin - bottomMargin;
  
  // Draw grid
  ctx.strokeStyle = '#003300';
  ctx.lineWidth = 1;
  for (let i = 0; i <= 10; i++) {
    const y = topMargin + graphHeight * i / 10;
    ctx.beginPath();
    ctx.moveTo(leftMargin, y);
    ctx.lineTo(canvas.width - rightMargin, y);
    ctx.stroke();
  }
  
  // Y-axis labels
  ctx.fillStyle = '#0f0';
  ctx.font = '11px monospace';
  ctx.textAlign = 'right';
  for (let i = 0; i <= 10; i++) {
    const rssi = maxRSSI - (maxRSSI - minRSSI) * i / 10;
    const y = topMargin + graphHeight * i / 10;
    ctx.fillText(rssi.toFixed(0) + ' dBm', leftMargin - 10, y + 4);
  }
  
  // Title
  ctx.textAlign = 'center';
  ctx.fillStyle = '#0ff';
  ctx.font = 'bold 14px monospace';
  ctx.fillText('SIGNAL STRENGTH OVER TIME', canvas.width / 2, 20);
  
  // X-axis label
  ctx.fillStyle = '#0f0';
  ctx.font = '11px monospace';
  ctx.fillText('Time (last 60 readings)', canvas.width / 2, canvas.height - 10);
  
  // Draw signal lines
  const colors = ['#0f0', '#0ff', '#ff0', '#f80', '#f0f', '#0af', '#fa0'];
  
  activeHistories.forEach(([bssid, history], idx) => {
    const color = colors[idx % colors.length];
    
    ctx.strokeStyle = color;
    ctx.lineWidth = 2;
    ctx.beginPath();
    
    let started = false;
    for (let i = 0; i < history.length; i++) {
      const h = history[i];
      const x = leftMargin + graphWidth * i / Math.max(1, history.length - 1);
      const normalizedRSSI = (h.rssi - minRSSI) / (maxRSSI - minRSSI);
      const y = topMargin + graphHeight * (1 - normalizedRSSI);
      
      if (!started) {
        ctx.moveTo(x, y);
        started = true;
      } else {
        ctx.lineTo(x, y);
      }
    }
    ctx.stroke();
    
    // Legend
    ctx.fillStyle = color;
    ctx.textAlign = 'left';
    ctx.font = '11px monospace';
    const lastRSSI = history[history.length - 1].rssi;
    const ssid = history[history.length - 1].ssid;
    ctx.fillText(`${ssid.substring(0, 15)}: ${lastRSSI} dBm`, canvas.width - 220, topMargin + idx * 18);
  });
  
  ctx.textAlign = 'left';
}

// Configuration
async function loadConfig() {
  const resp = await fetch('/api/config');
  config = await resp.json();
  
  document.getElementById('cfg-track-hist').value = config.trackHistory;
  document.getElementById('cfg-val-thresh').value = config.validationThreshold;
  document.getElementById('cfg-scan-int').value = config.scanInterval;
  document.getElementById('cfg-prox-alert').value = config.proximityAlert;
  document.getElementById('cfg-profile').value = config.missionProfile;
  document.getElementById('hud-profile').textContent = config.missionProfile.toUpperCase();
}

async function saveConfiguration() {
  config.trackHistory = parseInt(document.getElementById('cfg-track-hist').value);
  config.validationThreshold = parseInt(document.getElementById('cfg-val-thresh').value);
  config.scanInterval = parseInt(document.getElementById('cfg-scan-int').value);
  config.proximityAlert = parseInt(document.getElementById('cfg-prox-alert').value);
  config.missionProfile = document.getElementById('cfg-profile').value;
  
  await fetch('/api/config', {
    method: 'POST',
    headers: {'Content-Type': 'application/json'},
    body: JSON.stringify(config)
  });
  
  document.getElementById('hud-profile').textContent = config.missionProfile.toUpperCase();
  logMessage('CONFIGURATION SAVED', 'normal');
}

// Tagged contacts
async function loadTags() {
  const resp = await fetch('/api/tags');
  const data = await resp.json();
  taggedContacts = data.tags;
  
  const tbody = document.getElementById('tags-tbody');
  tbody.innerHTML = '';
  
  taggedContacts.forEach(tag => {
    const tr = document.createElement('tr');
    tr.innerHTML = `
      <td>${tag.name}</td>
      <td>${tag.pattern}</td>
      <td><span class="tag-color" style="background:${tag.color}"></span> ${tag.color}</td>
      <td>${tag.sound ? 'Yes' : 'No'}</td>
      <td><button class="btn-delete" onclick="deleteTag('${tag.name}')">DELETE</button></td>
    `;
    tbody.appendChild(tr);
  });
}

async function addTag() {
  const name = document.getElementById('tag-name').value.trim();
  const pattern = document.getElementById('tag-pattern').value.trim();
  const color = document.getElementById('tag-color').value;
  const sound = document.getElementById('tag-sound').checked;
  
  if (!name || !pattern) {
    alert('Name and pattern are required');
    return;
  }
  
  await fetch('/api/tags', {
    method: 'POST',
    headers: {'Content-Type': 'application/json'},
    body: JSON.stringify({name, pattern, color, sound})
  });
  
  document.getElementById('tag-name').value = '';
  document.getElementById('tag-pattern').value = '';
  
  loadTags();
  logMessage(`TAGGED CONTACT ADDED: ${name} (${pattern})`, 'normal');
}

async function deleteTag(name) {
  await fetch('/api/tags', {
    method: 'DELETE',
    headers: {'Content-Type': 'application/json'},
    body: JSON.stringify({name})
  });
  
  loadTags();
  logMessage(`TAGGED CONTACT REMOVED: ${name}`, 'normal');
}

// Session export
function exportSession() {
  const data = {
    timestamp: new Date().toISOString(),
    config: config,
    tags: taggedContacts,
    tracks: Array.from(tracks.values()),
    log: sessionLog,
    signalHistory: Array.from(signalHistory.entries())
  };
  
  const blob = new Blob([JSON.stringify(data, null, 2)], {type: 'application/json'});
  const url = URL.createObjectURL(blob);
  const a = document.createElement('a');
  a.href = url;
  a.download = `widar-session-${Date.now()}.json`;
  a.click();
  
  logMessage('SESSION LOG EXPORTED', 'normal');
}

// Tab switching
document.querySelectorAll('.tab').forEach(tab => {
  tab.addEventListener('click', () => {
    document.querySelectorAll('.tab').forEach(t => t.classList.remove('active'));
    document.querySelectorAll('.view').forEach(v => v.classList.remove('active'));
    
    tab.classList.add('active');
    document.getElementById(tab.dataset.tab + '-view').classList.add('active');
    
    // Resume radar animation if switching to radar tab
    if (tab.dataset.tab === 'radar') {
      drawRadar();
    }
  });
});

// Keyboard shortcuts
document.addEventListener('keydown', e => {
  if (e.key === ' ') {
    e.preventDefault();
    isPaused = !isPaused;
    logMessage(isPaused ? 'SCANNING PAUSED' : 'SCANNING RESUMED', 'normal');
  } else if (e.key === 'r' || e.key === 'R') {
    tracks.clear();
    signalHistory.clear();
    logMessage('ALL TRACKS RESET', 'normal');
  } else if (e.key === 'm' || e.key === 'M') {
    isMuted = !isMuted;
    logMessage(isMuted ? 'AUDIO MUTED' : 'AUDIO UNMUTED', 'normal');
  } else if (e.key === 'f' || e.key === 'F') {
    if (!document.fullscreenElement) {
      document.documentElement.requestFullscreen();
    } else {
      document.exitFullscreen();
    }
  } else if (e.key >= '1' && e.key <= '4') {
    const tabs = ['radar', 'signal', 'config', 'tags'];
    const tab = document.querySelector(`.tab[data-tab="${tabs[e.key - 1]}"]`);
    if (tab) tab.click();
  } else if (e.key === '?') {
    const help = document.getElementById('shortcuts-help');
    help.classList.toggle('show');
  }
});

// Initialize
loadConfig();
loadTags();
scanWiFi();
drawRadar();
logMessage('SPEC5 WI-DAR SYSTEM INITIALIZED', 'normal');
logMessage('TACTICAL WIFI TRACKING ACTIVE', 'normal');
</script>
</body>
</html>
)html");
}

void setup() {
  Serial.begin(115200);
  
  loadConfig();
  
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(ssid, password);
  
  Serial.println("Wi-Dar AP started");
  Serial.print("IP: ");
  Serial.println(WiFi.softAPIP());
  
  server.on("/", handleRoot);
  server.on("/api/scan", handleScanAPI);
  server.on("/api/config", HTTP_GET, handleConfigAPI);
  server.on("/api/config", HTTP_POST, handleConfigAPI);
  server.on("/api/tags", HTTP_GET, handleTagsAPI);
  server.on("/api/tags", HTTP_POST, handleTagsAPI);
  server.on("/api/tags", HTTP_DELETE, handleTagsAPI);
  
  server.begin();
  Serial.println("Server started");
}

void loop() {
  server.handleClient();
}
