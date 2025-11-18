#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include "esp_wifi.h"

// Client detection structures
struct ClientDevice {
  uint8_t mac[6];
  int8_t rssi;
  uint8_t channel;
  unsigned long lastSeen;
  unsigned long firstSeen;
  int probeCount;
  String manufacturer;
  bool isRandom;
};

std::vector<ClientDevice> detectedClients;
uint8_t currentChannel = 1;
unsigned long lastChannelHop = 0;
int channelHopInterval = 1000; // ms
bool clientDetectionEnabled = true;

// OUI database (top manufacturers)
struct OUI {
  const char* prefix;
  const char* manufacturer;
};

const OUI ouiDatabase[] = {
  {"00:00:5E", "IANA"},
  {"00:03:93", "Apple"},
  {"00:0D:93", "Apple"},
  {"00:17:F2", "Apple"},
  {"00:1B:63", "Apple"},
  {"00:1E:C2", "Apple"},
  {"00:1F:F3", "Apple"},
  {"00:21:E9", "Apple"},
  {"00:23:12", "Apple"},
  {"00:23:32", "Apple"},
  {"00:23:6C", "Apple"},
  {"00:24:36", "Apple"},
  {"00:25:00", "Apple"},
  {"00:25:4B", "Apple"},
  {"00:25:BC", "Apple"},
  {"00:26:08", "Apple"},
  {"00:26:4A", "Apple"},
  {"00:26:B0", "Apple"},
  {"00:26:BB", "Apple"},
  {"04:0C:CE", "Apple"},
  {"04:15:52", "Apple"},
  {"04:26:65", "Apple"},
  {"04:48:9A", "Apple"},
  {"04:4B:ED", "Apple"},
  {"04:54:53", "Apple"},
  {"04:69:F8", "Apple"},
  {"04:DB:56", "Apple"},
  {"04:E5:36", "Apple"},
  {"04:F1:3E", "Apple"},
  {"04:F7:E4", "Apple"},
  {"08:00:07", "Apple"},
  {"08:66:98", "Apple"},
  {"08:70:45", "Apple"},
  {"08:74:02", "Apple"},
  {"0C:3E:9F", "Apple"},
  {"0C:4D:E9", "Apple"},
  {"0C:74:C2", "Apple"},
  {"10:40:F3", "Apple"},
  {"10:9A:DD", "Apple"},
  {"10:DD:B1", "Apple"},
  {"14:10:9F", "Apple"},
  {"14:20:5E", "Apple"},
  {"14:5A:05", "Apple"},
  {"14:8F:C6", "Apple"},
  {"14:BD:61", "Apple"},
  {"18:20:32", "Apple"},
  {"18:34:51", "Apple"},
  {"18:3D:A2", "Apple"},
  {"18:AF:61", "Apple"},
  {"18:E7:F4", "Apple"},
  {"18:F1:D8", "Apple"},
  {"1C:1A:C0", "Apple"},
  {"1C:36:BB", "Apple"},
  {"1C:AB:A7", "Apple"},
  {"20:3C:AE", "Apple"},
  {"20:78:F0", "Apple"},
  {"20:A2:E4", "Apple"},
  {"20:AB:37", "Apple"},
  {"20:C9:D0", "Apple"},
  {"24:A0:74", "Apple"},
  {"24:A2:E1", "Apple"},
  {"24:AB:81", "Apple"},
  {"24:F0:94", "Apple"},
  {"24:F6:77", "Apple"},
  {"28:37:37", "Apple"},
  {"28:5A:EB", "Apple"},
  {"28:6A:B8", "Apple"},
  {"28:6A:BA", "Apple"},
  {"28:A0:2B", "Apple"},
  {"28:CF:DA", "Apple"},
  {"28:CF:E9", "Apple"},
  {"28:E0:2C", "Apple"},
  {"28:E1:4C", "Apple"},
  {"28:ED:6A", "Apple"},
  {"28:F0:76", "Apple"},
  {"2C:1F:23", "Apple"},
  {"2C:33:11", "Apple"},
  {"2C:36:F8", "Apple"},
  {"2C:3A:E8", "Apple"},
  {"2C:B4:3A", "Apple"},
  {"2C:BE:08", "Apple"},
  {"2C:F0:A2", "Apple"},
  {"2C:F0:EE", "Apple"},
  {"30:35:AD", "Apple"},
  {"30:90:AB", "Apple"},
  {"30:F7:C5", "Apple"},
  {"34:12:98", "Apple"},
  {"34:15:9E", "Apple"},
  {"34:36:3B", "Apple"},
  {"34:51:C9", "Apple"},
  {"34:A3:95", "Apple"},
  {"34:AB:37", "Apple"},
  {"34:C0:59", "Apple"},
  {"34:E2:FD", "Apple"},
  {"38:0F:4A", "Apple"},
  {"38:48:4C", "Apple"},
  {"38:89:2C", "Apple"},
  {"38:B5:4D", "Apple"},
  {"38:C9:86", "Apple"},
  {"3C:07:54", "Apple"},
  {"3C:15:C2", "Apple"},
  {"3C:2E:F9", "Apple"},
  {"3C:A9:F4", "Apple"},
  {"3C:D0:F8", "Apple"},
  {"40:30:04", "Apple"},
  {"40:33:1A", "Apple"},
  {"40:3C:FC", "Apple"},
  {"40:4D:7F", "Apple"},
  {"40:6C:8F", "Apple"},
  {"40:83:1D", "Apple"},
  {"40:A6:D9", "Apple"},
  {"40:B3:95", "Apple"},
  {"40:CB:C0", "Apple"},
  {"40:D3:2D", "Apple"},
  {"44:00:10", "Apple"},
  {"44:2A:60", "Apple"},
  {"44:4C:0C", "Apple"},
  {"44:D8:84", "Apple"},
  {"44:FB:42", "Apple"},
  {"48:3B:38", "Apple"},
  {"48:43:7C", "Apple"},
  {"48:60:BC", "Apple"},
  {"48:74:6E", "Apple"},
  {"48:A1:95", "Apple"},
  {"48:BF:6B", "Apple"},
  {"48:D7:05", "Apple"},
  {"4C:32:75", "Apple"},
  {"4C:57:CA", "Apple"},
  {"4C:7C:5F", "Apple"},
  {"4C:8D:79", "Apple"},
  {"4C:B1:99", "Apple"},
  {"50:32:37", "Apple"},
  {"50:3D:E5", "Apple"},
  {"50:EA:D6", "Apple"},
  {"50:ED:3C", "Apple"},
  {"54:26:96", "Apple"},
  {"54:4E:90", "Apple"},
  {"54:72:4F", "Apple"},
  {"54:80:1D", "Apple"},
  {"54:9F:13", "Apple"},
  {"54:AE:27", "Apple"},
  {"54:E4:3A", "Apple"},
  {"54:EA:A8", "Apple"},
  {"58:1F:AA", "Apple"},
  {"58:40:4E", "Apple"},
  {"58:55:CA", "Apple"},
  {"58:B0:35", "Apple"},
  {"58:E2:8F", "Apple"},
  {"5C:59:48", "Apple"},
  {"5C:95:AE", "Apple"},
  {"5C:96:9D", "Apple"},
  {"5C:97:F3", "Apple"},
  {"5C:F9:38", "Apple"},
  {"60:03:08", "Apple"},
  {"60:33:4B", "Apple"},
  {"60:69:44", "Apple"},
  {"60:8C:4A", "Apple"},
  {"60:92:17", "Apple"},
  {"60:A3:7D", "Apple"},
  {"60:C5:47", "Apple"},
  {"60:D9:C7", "Apple"},
  {"60:F4:45", "Apple"},
  {"60:F8:1D", "Apple"},
  {"60:FA:CD", "Apple"},
  {"60:FB:42", "Apple"},
  {"60:FE:C5", "Apple"},
  {"64:20:0C", "Apple"},
  {"64:76:BA", "Apple"},
  {"64:9A:BE", "Apple"},
  {"64:A3:CB", "Apple"},
  {"64:A5:C3", "Apple"},
  {"64:B0:A6", "Apple"},
  {"64:B9:E8", "Apple"},
  {"64:E6:82", "Apple"},
  {"68:09:27", "Apple"},
  {"68:5B:35", "Apple"},
  {"68:64:4B", "Apple"},
  {"68:96:7B", "Apple"},
  {"68:A8:6D", "Apple"},
  {"68:AB:1E", "Apple"},
  {"68:AE:20", "Apple"},
  {"68:D9:3C", "Apple"},
  {"68:DB:F5", "Apple"},
  {"68:FE:F7", "Apple"},
  {"6C:19:C0", "Apple"},
  {"6C:3E:6D", "Apple"},
  {"6C:40:08", "Apple"},
  {"6C:4D:73", "Apple"},
  {"6C:70:9F", "Apple"},
  {"6C:72:E7", "Apple"},
  {"6C:8D:C1", "Apple"},
  {"6C:94:66", "Apple"},
  {"6C:96:CF", "Apple"},
  {"6C:AB:31", "Apple"},
  {"6C:C2:6B", "Apple"},
  {"70:11:24", "Apple"},
  {"70:14:A6", "Apple"},
  {"70:3E:AC", "Apple"},
  {"70:48:0F", "Apple"},
  {"70:56:81", "Apple"},
  {"70:73:CB", "Apple"},
  {"70:A2:B3", "Apple"},
  {"70:CD:60", "Apple"},
  {"70:DE:E2", "Apple"},
  {"70:EC:E4", "Apple"},
  {"74:1B:B2", "Apple"},
  {"74:81:14", "Apple"},
  {"74:E1:B6", "Apple"},
  {"74:E2:F5", "Apple"},
  {"78:31:C1", "Apple"},
  {"78:3A:84", "Apple"},
  {"78:67:D7", "Apple"},
  {"78:7B:8A", "Apple"},
  {"78:88:6D", "Apple"},
  {"78:A3:E4", "Apple"},
  {"78:CA:39", "Apple"},
  {"78:D7:5F", "Apple"},
  {"78:FD:94", "Apple"},
  {"7C:01:91", "Apple"},
  {"7C:04:D0", "Apple"},
  {"7C:11:BE", "Apple"},
  {"7C:50:49", "Apple"},
  {"7C:6D:62", "Apple"},
  {"7C:6D:F8", "Apple"},
  {"7C:C3:A1", "Apple"},
  {"7C:C5:37", "Apple"},
  {"7C:D1:C3", "Apple"},
  {"7C:F0:5F", "Apple"},
  {"7C:FA:DF", "Apple"},
  {"80:00:6E", "Apple"},
  {"80:49:71", "Apple"},
  {"80:92:9F", "Apple"},
  {"80:BE:05", "Apple"},
  {"80:E6:50", "Apple"},
  {"84:38:35", "Apple"},
  {"84:78:8B", "Apple"},
  {"84:85:06", "Apple"},
  {"84:89:AD", "Apple"},
  {"84:FC:FE", "Apple"},
  {"88:1F:A1", "Apple"},
  {"88:53:95", "Apple"},
  {"88:63:DF", "Apple"},
  {"88:66:5A", "Apple"},
  {"88:6B:6E", "Apple"},
  {"88:AE:07", "Apple"},
  {"88:CB:87", "Apple"},
  {"88:E8:7F", "Apple"},
  {"8C:00:6D", "Apple"},
  {"8C:2D:AA", "Apple"},
  {"8C:58:77", "Apple"},
  {"8C:7A:AA", "Apple"},
  {"8C:7B:9D", "Apple"},
  {"8C:7C:92", "Apple"},
  {"8C:85:90", "Apple"},
  {"8C:8E:F2", "Apple"},
  {"8C:FA:BA", "Apple"},
  {"90:27:E4", "Apple"},
  {"90:72:40", "Apple"},
  {"90:84:0D", "Apple"},
  {"90:8D:6C", "Apple"},
  {"90:9C:4A", "Apple"},
  {"90:B0:ED", "Apple"},
  {"90:B2:1F", "Apple"},
  {"90:B9:31", "Apple"},
  {"90:FD:61", "Apple"},
  {"94:94:26", "Apple"},
  {"94:BF:2D", "Apple"},
  {"94:E9:6A", "Apple"},
  {"94:F6:A3", "Apple"},
  {"98:03:D8", "Apple"},
  {"98:0D:2E", "Apple"},
  {"98:5A:EB", "Apple"},
  {"98:B8:E3", "Apple"},
  {"98:CA:33", "Apple"},
  {"98:D6:BB", "Apple"},
  {"98:E0:D9", "Apple"},
  {"98:F0:AB", "Apple"},
  {"98:FE:94", "Apple"},
  {"9C:04:EB", "Apple"},
  {"9C:20:7B", "Apple"},
  {"9C:28:EF", "Apple"},
  {"9C:35:EB", "Apple"},
  {"9C:84:BF", "Apple"},
  {"9C:B6:D0", "Apple"},
  {"9C:D2:1E", "Apple"},
  {"9C:D3:5B", "Apple"},
  {"9C:E6:5E", "Apple"},
  {"9C:F3:87", "Apple"},
  {"A0:3B:E3", "Apple"},
  {"A0:99:9B", "Apple"},
  {"A0:D7:95", "Apple"},
  {"A0:ED:CD", "Apple"},
  {"A4:31:35", "Apple"},
  {"A4:5E:60", "Apple"},
  {"A4:67:06", "Apple"},
  {"A4:83:E7", "Apple"},
  {"A4:B1:97", "Apple"},
  {"A4:B8:05", "Apple"},
  {"A4:C3:61", "Apple"},
  {"A4:D1:8C", "Apple"},
  {"A4:D1:D2", "Apple"},
  {"A4:F1:E8", "Apple"},
  {"A8:20:66", "Apple"},
  {"A8:5B:78", "Apple"},
  {"A8:60:B6", "Apple"},
  {"A8:66:7F", "Apple"},
  {"A8:86:DD", "Apple"},
  {"A8:96:8A", "Apple"},
  {"A8:BB:CF", "Apple"},
  {"A8:FA:D8", "Apple"},
  {"AC:1F:74", "Apple"},
  {"AC:29:3A", "Apple"},
  {"AC:3C:0B", "Apple"},
  {"AC:61:EA", "Apple"},
  {"AC:7F:3E", "Apple"},
  {"AC:87:A3", "Apple"},
  {"AC:BC:32", "Apple"},
  {"AC:CF:5C", "Apple"},
  {"AC:E4:B5", "Apple"},
  {"AC:FD:EC", "Apple"},
  {"B0:34:95", "Apple"},
  {"B0:65:BD", "Apple"},
  {"B0:9F:BA", "Apple"},
  {"B0:CA:68", "Apple"},
  {"B4:18:D1", "Apple"},
  {"B4:8B:19", "Apple"},
  {"B4:F0:AB", "Apple"},
  {"B4:F6:1C", "Apple"},
  {"B8:09:8A", "Apple"},
  {"B8:17:C2", "Apple"},
  {"B8:41:A4", "Apple"},
  {"B8:44:D9", "Apple"},
  {"B8:53:AC", "Apple"},
  {"B8:63:4D", "Apple"},
  {"B8:78:2E", "Apple"},
  {"B8:C1:11", "Apple"},
  {"B8:C7:5D", "Apple"},
  {"B8:E8:56", "Apple"},
  {"B8:F6:B1", "Apple"},
  {"B8:FF:61", "Apple"},
  {"BC:3B:AF", "Apple"},
  {"BC:52:B7", "Apple"},
  {"BC:67:1C", "Apple"},
  {"BC:6C:21", "Apple"},
  {"BC:92:6B", "Apple"},
  {"BC:9F:EF", "Apple"},
  {"BC:A9:20", "Apple"},
  {"BC:D0:74", "Apple"},
  {"BC:EC:5D", "Apple"},
  {"BC:F5:AC", "Apple"},
  {"C0:1A:DA", "Apple"},
  {"C0:63:94", "Apple"},
  {"C0:84:7D", "Apple"},
  {"C0:9A:D0", "Apple"},
  {"C0:B6:58", "Apple"},
  {"C0:CC:F8", "Apple"},
  {"C0:CE:CD", "Apple"},
  {"C0:D0:12", "Apple"},
  {"C0:F2:FB", "Apple"},
  {"C4:2C:03", "Apple"},
  {"C4:61:8B", "Apple"},
  {"C4:B3:01", "Apple"},
  {"C8:1E:E7", "Apple"},
  {"C8:33:4B", "Apple"},
  {"C8:3C:85", "Apple"},
  {"C8:69:CD", "Apple"},
  {"C8:6F:1D", "Apple"},
  {"C8:85:50", "Apple"},
  {"C8:89:F3", "Apple"},
  {"C8:B5:B7", "Apple"},
  {"C8:BC:C8", "Apple"},
  {"C8:D0:83", "Apple"},
  {"C8:E0:EB", "Apple"},
  {"CC:08:E0", "Apple"},
  {"CC:25:EF", "Apple"},
  {"CC:29:F5", "Apple"},
  {"CC:2D:B7", "Apple"},
  {"CC:44:63", "Apple"},
  {"CC:78:5F", "Apple"},
  {"CC:C7:60", "Apple"},
  {"D0:03:4B", "Apple"},
  {"D0:23:DB", "Apple"},
  {"D0:25:98", "Apple"},
  {"D0:33:11", "Apple"},
  {"D0:4F:7E", "Apple"},
  {"D0:81:7A", "Apple"},
  {"D0:A6:37", "Apple"},
  {"D0:C5:F3", "Apple"},
  {"D0:D2:B0", "Apple"},
  {"D0:E1:40", "Apple"},
  {"D4:61:9D", "Apple"},
  {"D4:90:9C", "Apple"},
  {"D4:9A:20", "Apple"},
  {"D4:A3:3D", "Apple"},
  {"D4:DC:CD", "Apple"},
  {"D4:F4:6F", "Apple"},
  {"D8:00:4D", "Apple"},
  {"D8:1D:72", "Apple"},
  {"D8:30:62", "Apple"},
  {"D8:3B:BF", "Apple"},
  {"D8:96:95", "Apple"},
  {"D8:9E:3F", "Apple"},
  {"D8:A2:5E", "Apple"},
  {"D8:BB:2C", "Apple"},
  {"D8:CF:9C", "Apple"},
  {"D8:D1:CB", "Apple"},
  {"DC:0C:2D", "Apple"},
  {"DC:2B:2A", "Apple"},
  {"DC:2B:61", "Apple"},
  {"DC:37:18", "Apple"},
  {"DC:3F:32", "Apple"},
  {"DC:41:E5", "Apple"},
  {"DC:56:E7", "Apple"},
  {"DC:86:D8", "Apple"},
  {"DC:9B:9C", "Apple"},
  {"DC:A4:CA", "Apple"},
  {"DC:A9:04", "Apple"},
  {"DC:C3:2C", "Apple"},
  {"DC:D3:A2", "Apple"},
  {"E0:33:8E", "Apple"},
  {"E0:5F:45", "Apple"},
  {"E0:66:78", "Apple"},
  {"E0:89:9D", "Apple"},
  {"E0:AC:CB", "Apple"},
  {"E0:B5:2D", "Apple"},
  {"E0:B9:A5", "Apple"},
  {"E0:C7:67", "Apple"},
  {"E0:F5:C6", "Apple"},
  {"E0:F8:47", "Apple"},
  {"E4:25:E7", "Apple"},
  {"E4:8B:7F", "Apple"},
  {"E4:98:D6", "Apple"},
  {"E4:9A:79", "Apple"},
  {"E4:C6:3D", "Apple"},
  {"E4:CE:8F", "Apple"},
  {"E8:04:0B", "Apple"},
  {"E8:06:88", "Apple"},
  {"E8:40:F2", "Apple"},
  {"E8:80:2E", "Apple"},
  {"E8:B2:AC", "Apple"},
  {"EC:35:86", "Apple"},
  {"EC:85:2F", "Apple"},
  {"EC:A8:6B", "Apple"},
  {"F0:18:98", "Apple"},
  {"F0:24:75", "Apple"},
  {"F0:5C:19", "Apple"},
  {"F0:72:8C", "Apple"},
  {"F0:98:9D", "Apple"},
  {"F0:B4:79", "Apple"},
  {"F0:C3:71", "Apple"},
  {"F0:CB:A1", "Apple"},
  {"F0:D1:A9", "Apple"},
  {"F0:DB:E2", "Apple"},
  {"F0:DB:F8", "Apple"},
  {"F0:DC:E2", "Apple"},
  {"F0:F6:1C", "Apple"},
  {"F4:0F:24", "Apple"},
  {"F4:1B:A1", "Apple"},
  {"F4:37:B7", "Apple"},
  {"F4:5C:89", "Apple"},
  {"F4:F1:5A", "Apple"},
  {"F4:F9:51", "Apple"},
  {"F8:1E:DF", "Apple"},
  {"F8:27:93", "Apple"},
  {"F8:2D:7C", "Apple"},
  {"F8:6F:C1", "Apple"},
  {"F8:95:C7", "Apple"},
  {"F8:DB:7F", "Apple"},
  {"F8:E9:4E", "Apple"},
  {"F8:FF:C2", "Apple"},
  {"FC:25:3F", "Apple"},
  {"FC:2A:9C", "Apple"},
  {"FC:64:BA", "Apple"},
  {"FC:E9:98", "Apple"},
  {"FC:FC:48", "Apple"},
  {"00:50:F2", "Microsoft"},
  {"00:15:5D", "Microsoft"},
  {"28:18:78", "Microsoft"},
  {"7C:1E:52", "Microsoft"},
  {"00:0C:76", "Samsung"},
  {"00:12:FB", "Samsung"},
  {"00:13:77", "Samsung"},
  {"00:15:99", "Samsung"},
  {"00:16:32", "Samsung"},
  {"00:16:6B", "Samsung"},
  {"00:16:6C", "Samsung"},
  {"00:16:DB", "Samsung"},
  {"00:17:C9", "Samsung"},
  {"00:17:D5", "Samsung"},
  {"00:18:AF", "Samsung"},
  {"00:1A:8A", "Samsung"},
  {"00:1B:98", "Samsung"},
  {"00:1C:43", "Samsung"},
  {"00:1D:25", "Samsung"},
  {"00:1D:F6", "Samsung"},
  {"00:1E:7D", "Samsung"},
  {"00:1E:E1", "Samsung"},
  {"00:1E:E2", "Samsung"},
  {"00:1F:CC", "Samsung"},
  {"00:21:19", "Samsung"},
  {"00:21:4C", "Samsung"},
  {"00:21:D1", "Samsung"},
  {"00:21:D2", "Samsung"},
  {"00:23:39", "Samsung"},
  {"00:23:99", "Samsung"},
  {"00:23:D6", "Samsung"},
  {"00:23:D7", "Samsung"},
  {"00:24:54", "Samsung"},
  {"00:24:90", "Samsung"},
  {"00:24:91", "Samsung"},
  {"00:24:E9", "Samsung"},
  {"00:25:38", "Samsung"},
  {"00:25:66", "Samsung"},
  {"00:25:67", "Samsung"},
  {"00:26:37", "Samsung"},
  {"00:26:5D", "Samsung"},
  {"00:26:5F", "Samsung"},
  {"00:E0:64", "Samsung"},
  {"34:23:BA", "Samsung"},
  {"38:AA:3C", "Samsung"},
  {"40:0E:85", "Samsung"},
  {"44:4E:6D", "Samsung"},
  {"50:32:75", "Samsung"},
  {"50:B7:C3", "Samsung"},
  {"54:88:0E", "Samsung"},
  {"5C:0A:5B", "Samsung"},
  {"60:6B:BD", "Samsung"},
  {"68:EB:AE", "Samsung"},
  {"6C:2F:2C", "Samsung"},
  {"78:1F:DB", "Samsung"},
  {"78:47:1D", "Samsung"},
  {"78:59:5E", "Samsung"},
  {"78:9E:D0", "Samsung"},
  {"78:A8:73", "Samsung"},
  {"78:D6:F0", "Samsung"},
  {"7C:61:93", "Samsung"},
  {"7C:B0:C2", "Samsung"},
  {"80:57:19", "Samsung"},
  {"84:25:DB", "Samsung"},
  {"84:38:38", "Samsung"},
  {"88:32:9B", "Samsung"},
  {"8C:77:12", "Samsung"},
  {"8C:C8:CD", "Samsung"},
  {"90:18:7C", "Samsung"},
  {"94:63:D1", "Samsung"},
  {"98:52:B1", "Samsung"},
  {"A0:07:98", "Samsung"},
  {"A0:21:95", "Samsung"},
  {"A0:75:91", "Samsung"},
  {"A4:EB:D3", "Samsung"},
  {"AC:36:13", "Samsung"},
  {"AC:5F:3E", "Samsung"},
  {"B4:07:F9", "Samsung"},
  {"B4:79:A7", "Samsung"},
  {"B8:5E:7B", "Samsung"},
  {"BC:14:85", "Samsung"},
  {"BC:20:BA", "Samsung"},
  {"BC:44:86", "Samsung"},
  {"BC:72:B1", "Samsung"},
  {"BC:8C:CD", "Samsung"},
  {"BC:B1:F3", "Samsung"},
  {"C0:BD:D1", "Samsung"},
  {"C4:42:02", "Samsung"},
  {"C4:57:6E", "Samsung"},
  {"C8:14:79", "Samsung"},
  {"C8:19:F7", "Samsung"},
  {"C8:A8:23", "Samsung"},
  {"CC:3A:61", "Samsung"},
  {"CC:D9:09", "Samsung"},
  {"CC:FE:3C", "Samsung"},
  {"D0:17:6A", "Samsung"},
  {"D0:22:BE", "Samsung"},
  {"D0:59:E4", "Samsung"},
  {"D0:66:7B", "Samsung"},
  {"D0:87:E2", "Samsung"},
  {"D4:87:D8", "Samsung"},
  {"D4:E8:B2", "Samsung"},
  {"D8:57:EF", "Samsung"},
  {"D8:90:E8", "Samsung"},
  {"DC:71:44", "Samsung"},
  {"E4:12:1D", "Samsung"},
  {"E4:40:E2", "Samsung"},
  {"E4:92:FB", "Samsung"},
  {"E8:03:9A", "Samsung"},
  {"E8:11:32", "Samsung"},
  {"E8:50:8B", "Samsung"},
  {"EC:1D:8B", "Samsung"},
  {"F0:08:D1", "Samsung"},
  {"F0:25:B7", "Samsung"},
  {"F0:5A:09", "Samsung"},
  {"F0:6B:CA", "Samsung"},
  {"F4:09:D8", "Samsung"},
  {"F4:7B:5E", "Samsung"},
  {"F4:D9:FB", "Samsung"},
  {"F8:04:2E", "Samsung"},
  {"F8:D0:BD", "Samsung"},
  {"FC:00:12", "Samsung"},
  {"FC:A1:3E", "Samsung"},
  {"FC:C7:34", "Samsung"},
  {"00:1A:11", "Google"},
  {"00:1E:C1", "Google"},
  {"3C:5A:B4", "Google"},
  {"54:60:09", "Google"},
  {"68:C4:4D", "Google"},
  {"6C:AD:F8", "Google"},
  {"74:E5:43", "Google"},
  {"84:1B:5E", "Google"},
  {"AC:63:BE", "Google"},
  {"B4:F0:AB", "Google"},
  {"D8:80:39", "Google"},
  {"DC:A6:32", "Google"},
  {"F4:F5:E8", "Google"},
  {"F8:8F:CA", "Google"},
  {"00:0A:95", "Intel"},
  {"00:02:B3", "Intel"},
  {"00:03:47", "Intel"},
  {"00:04:23", "Intel"},
  {"00:07:E9", "Intel"},
  {"00:0C:F1", "Intel"},
  {"00:0E:0C", "Intel"},
  {"00:0E:35", "Intel"},
  {"00:11:11", "Intel"},
  {"00:12:F0", "Intel"},
  {"00:13:02", "Intel"},
  {"00:13:20", "Intel"},
  {"00:13:CE", "Intel"},
  {"00:13:E8", "Intel"},
  {"00:15:00", "Intel"},
  {"00:15:17", "Intel"},
  {"00:16:6F", "Intel"},
  {"00:16:76", "Intel"},
  {"00:16:EA", "Intel"},
  {"00:16:EB", "Intel"},
  {"00:18:DE", "Intel"},
  {"00:19:D1", "Intel"},
  {"00:19:D2", "Intel"},
  {"00:1B:21", "Intel"},
  {"00:1B:77", "Intel"},
  {"00:1C:BF", "Intel"},
  {"00:1D:E0", "Intel"},
  {"00:1D:E1", "Intel"},
  {"00:1E:64", "Intel"},
  {"00:1E:65", "Intel"},
  {"00:1E:67", "Intel"},
  {"00:1F:3A", "Intel"},
  {"00:1F:3B", "Intel"},
  {"00:1F:3C", "Intel"},
  {"00:20:E0", "Intel"},
  {"00:21:5C", "Intel"},
  {"00:21:5D", "Intel"},
  {"00:21:6A", "Intel"},
  {"00:21:6B", "Intel"},
  {"00:22:FA", "Intel"},
  {"00:22:FB", "Intel"},
  {"00:23:14", "Intel"},
  {"00:23:15", "Intel"},
  {"00:24:D6", "Intel"},
  {"00:24:D7", "Intel"},
  {"00:25:D3", "Intel"},
  {"00:27:0E", "Intel"},
  {"00:27:10", "Intel"},
  {"00:50:F2", "Intel"},
  {"04:0E:3C", "Intel"},
  {"08:11:96", "Intel"},
  {"08:ED:B9", "Intel"},
  {"0C:8B:FD", "Intel"},
  {"10:0B:A9", "Intel"},
  {"18:3D:A2", "Intel"},
  {"1C:3E:84", "Intel"},
  {"1C:4B:D6", "Intel"},
  {"1C:65:9D", "Intel"},
  {"1C:87:2C", "Intel"},
  {"24:0A:64", "Intel"},
  {"28:16:AD", "Intel"},
  {"28:D2:44", "Intel"},
  {"28:E3:47", "Intel"},
  {"2C:44:FD", "Intel"},
  {"2C:FD:A1", "Intel"},
  {"30:3A:64", "Intel"},
  {"34:02:86", "Intel"},
  {"34:13:E8", "Intel"},
  {"34:E6:AD", "Intel"},
  {"38:2C:4A", "Intel"},
  {"3C:A9:F4", "Intel"},
  {"40:16:7E", "Intel"},
  {"44:85:00", "Intel"},
  {"48:45:20", "Intel"},
  {"48:51:B7", "Intel"},
  {"4C:34:88", "Intel"},
  {"4C:80:93", "Intel"},
  {"50:2D:A2", "Intel"},
  {"54:35:30", "Intel"},
  {"5C:51:4F", "Intel"},
  {"5C:C5:D4", "Intel"},
  {"5C:D2:E4", "Intel"},
  {"60:36:DD", "Intel"},
  {"60:57:18", "Intel"},
  {"60:67:20", "Intel"},
  {"60:6C:66", "Intel"},
  {"64:00:F1", "Intel"},
  {"64:27:37", "Intel"},
  {"64:5A:04", "Intel"},
  {"64:80:99", "Intel"},
  {"64:D9:54", "Intel"},
  {"68:05:CA", "Intel"},
  {"68:17:29", "Intel"},
  {"68:5D:43", "Intel"},
  {"6C:0B:84", "Intel"},
  {"6C:29:95", "Intel"},
  {"6C:88:14", "Intel"},
  {"70:1C:E7", "Intel"},
  {"70:5A:0F", "Intel"},
  {"70:77:81", "Intel"},
  {"74:27:EA", "Intel"},
  {"74:29:AF", "Intel"},
  {"74:2F:68", "Intel"},
  {"74:40:BB", "Intel"},
  {"74:E5:43", "Intel"},
  {"74:E6:E2", "Intel"},
  {"78:0C:B8", "Intel"},
  {"78:24:AF", "Intel"},
  {"78:31:C1", "Intel"},
  {"78:45:C4", "Intel"},
  {"78:92:9C", "Intel"},
  {"78:AC:C0", "Intel"},
  {"78:E3:B5", "Intel"},
  {"7C:7A:91", "Intel"},
  {"7C:B0:C2", "Intel"},
  {"80:19:34", "Intel"},
  {"80:86:F2", "Intel"},
  {"84:3A:4B", "Intel"},
  {"84:A6:C8", "Intel"},
  {"88:53:2E", "Intel"},
  {"88:78:73", "Intel"},
  {"8C:A9:82", "Intel"},
  {"8C:DC:D4", "Intel"},
  {"90:48:9A", "Intel"},
  {"90:61:AE", "Intel"},
  {"90:E2:BA", "Intel"},
  {"94:65:9C", "Intel"},
  {"94:E9:79", "Intel"},
  {"98:4F:EE", "Intel"},
  {"9C:4E:36", "Intel"},
  {"9C:B6:D0", "Intel"},
  {"9C:D2:1E", "Intel"},
  {"A0:88:69", "Intel"},
  {"A0:A8:CD", "Intel"},
  {"A4:02:B9", "Intel"},
  {"A4:34:D9", "Intel"},
  {"A4:4E:31", "Intel"},
  {"A4:7B:9C", "Intel"},
  {"A8:6D:AA", "Intel"},
  {"AC:2B:6E", "Intel"},
  {"AC:72:89", "Intel"},
  {"AC:7B:A1", "Intel"},
  {"AC:D1:B8", "Intel"},
  {"AC:E0:10", "Intel"},
  {"B0:05:94", "Intel"},
  {"B0:0C:D1", "Intel"},
  {"B4:6B:FC", "Intel"},
  {"B4:96:91", "Intel"},
  {"B4:B6:76", "Intel"},
  {"B8:08:CF", "Intel"},
  {"B8:6B:23", "Intel"},
  {"B8:8A:60", "Intel"},
  {"B8:CA:3A", "Intel"},
  {"BC:77:37", "Intel"},
  {"BC:83:85", "Intel"},
  {"BC:A8:A6", "Intel"},
  {"C0:3F:D5", "Intel"},
  {"C4:85:08", "Intel"},
  {"C8:21:58", "Intel"},
  {"C8:34:8E", "Intel"},
  {"C8:54:4B", "Intel"},
  {"C8:D3:FF", "Intel"},
  {"C8:F7:33", "Intel"},
  {"CC:2D:21", "Intel"},
  {"CC:3D:82", "Intel"},
  {"D0:50:99", "Intel"},
  {"D0:7E:28", "Intel"},
  {"D0:DF:9A", "Intel"},
  {"D4:3B:04", "Intel"},
  {"D4:6A:6A", "Intel"},
  {"D4:BE:D9", "Intel"},
  {"D8:0D:17", "Intel"},
  {"D8:9E:F3", "Intel"},
  {"DC:41:A9", "Intel"},
  {"DC:53:60", "Intel"},
  {"E0:94:67", "Intel"},
  {"E4:02:9B", "Intel"},
  {"E4:A7:A0", "Intel"},
  {"E4:B3:18", "Intel"},
  {"E4:F4:C6", "Intel"},
  {"E8:2A:EA", "Intel"},
  {"E8:6A:64", "Intel"},
  {"EC:0E:C4", "Intel"},
  {"EC:A8:6B", "Intel"},
  {"F0:1F:AF", "Intel"},
  {"F0:4D:A2", "Intel"},
  {"F0:76:1C", "Intel"},
  {"F0:D5:BF", "Intel"},
  {"F4:06:69", "Intel"},
  {"F4:30:B9", "Intel"},
  {"F4:6D:04", "Intel"},
  {"F8:16:54", "Intel"},
  {"F8:63:3F", "Intel"},
  {"F8:94:C2", "Intel"},
  {"FC:4D:D4", "Intel"},
  {"FC:77:74", "Intel"},
  {"FC:F8:AE", "Intel"}
};

String lookupManufacturer(const uint8_t* mac) {
  char prefix[9];
  snprintf(prefix, sizeof(prefix), "%02X:%02X:%02X", mac[0], mac[1], mac[2]);
  
  for (size_t i = 0; i < sizeof(ouiDatabase) / sizeof(OUI); i++) {
    if (strcasecmp(prefix, ouiDatabase[i].prefix) == 0) {
      return String(ouiDatabase[i].manufacturer);
    }
  }
  
  return "Unknown";
}

bool isRandomMAC(const uint8_t* mac) {
  // Check if locally administered bit is set (bit 1 of first byte)
  return (mac[0] & 0x02) != 0;
}

void IRAM_ATTR promiscuous_rx_callback(void* buf, wifi_promiscuous_pkt_type_t type) {
  if (!clientDetectionEnabled) return;
  if (type != WIFI_PKT_MGMT) return;
  
  wifi_promiscuous_pkt_t *pkt = (wifi_promiscuous_pkt_t*)buf;
  const wifi_pkt_rx_ctrl_t *ctrl = &pkt->rx_ctrl;
  const uint8_t *frame = pkt->payload;
  const uint16_t frameLen = ctrl->sig_len;
  
  if (frameLen < 24) return;
  
  // Check if it's a probe request (type 0, subtype 4)
  uint8_t frameType = (frame[0] & 0x0C) >> 2;
  uint8_t frameSubtype = (frame[0] & 0xF0) >> 4;
  
  if (frameType == 0 && frameSubtype == 4) {
    // Extract source MAC address (bytes 10-15)
    uint8_t srcMAC[6];
    memcpy(srcMAC, &frame[10], 6);
    
    // Check if we already have this client
    bool found = false;
    for (auto &client : detectedClients) {
      if (memcmp(client.mac, srcMAC, 6) == 0) {
        client.rssi = ctrl->rssi;
        client.lastSeen = millis();
        client.probeCount++;
        found = true;
        break;
      }
    }
    
    if (!found && detectedClients.size() < 100) {
      ClientDevice newClient;
      memcpy(newClient.mac, srcMAC, 6);
      newClient.rssi = ctrl->rssi;
      newClient.channel = ctrl->channel;
      newClient.lastSeen = millis();
      newClient.firstSeen = millis();
      newClient.probeCount = 1;
      newClient.manufacturer = lookupManufacturer(srcMAC);
      newClient.isRandom = isRandomMAC(srcMAC);
      detectedClients.push_back(newClient);
    }
  }
}

void setupClientDetection() {
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_rx_cb(promiscuous_rx_callback);
  esp_wifi_set_channel(currentChannel, WIFI_SECOND_CHAN_NONE);
}

void hopChannel() {
  if (millis() - lastChannelHop > channelHopInterval) {
    currentChannel = (currentChannel % 13) + 1;
    esp_wifi_set_channel(currentChannel, WIFI_SECOND_CHAN_NONE);
    lastChannelHop = millis();
  }
}

void cleanupOldClients() {
  unsigned long now = millis();
  detectedClients.erase(
    std::remove_if(detectedClients.begin(), detectedClients.end(),
      [now](const ClientDevice& client) {
        return (now - client.lastSeen) > 60000; // Remove after 60 seconds
      }),
    detectedClients.end()
  );
}


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


void handleClientsAPI() {
  JsonDocument doc;
  JsonArray clients = doc["clients"].to<JsonArray>();
  
  cleanupOldClients();
  
  for (const auto &client : detectedClients) {
    JsonObject c = clients.add<JsonObject>();
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
             client.mac[0], client.mac[1], client.mac[2],
             client.mac[3], client.mac[4], client.mac[5]);
    c["mac"] = macStr;
    c["rssi"] = client.rssi;
    c["channel"] = client.channel;
    c["lastSeen"] = (millis() - client.lastSeen) / 1000;
    c["probeCount"] = client.probeCount;
    c["manufacturer"] = client.manufacturer;
    c["isRandom"] = client.isRandom;
  }
  
  doc["count"] = detectedClients.size();
  doc["currentChannel"] = currentChannel;
  
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
        <div style="position: absolute; top: 10px; left: 10px; background: rgba(0,51,0,0.8); padding: 10px; border: 1px solid #0f0; border-radius: 5px; z-index: 100;">
          <div style="font: bold 14px monospace; color: #0f0; margin-bottom: 5px;">TRACK MODE</div>
          <label style="color: #0f0; font: 12px monospace; margin-right: 15px; cursor: pointer;">
            <input type="radio" name="trackMode" value="aps" checked onchange="updateTrackMode(this.value)"> APs
          </label>
          <label style="color: #0f0; font: 12px monospace; margin-right: 15px; cursor: pointer;">
            <input type="radio" name="trackMode" value="clients" onchange="updateTrackMode(this.value)"> Clients
          </label>
          <label style="color: #0f0; font: 12px monospace; cursor: pointer;">
            <input type="radio" name="trackMode" value="both" onchange="updateTrackMode(this.value)"> Both
          </label>
        </div>
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
  if (taggedWarning && (Date.now() - taggedWarning.timestamp < 10000)) {
    ctx.fillStyle = '#f00';
    ctx.font = 'bold 16px monospace';
    const warningLine1 = '⚠ TAGGED CONTACT ⚠';
    const warningLine2 = taggedWarning.name.substring(0, 19);
    // Calculate width based on actual text length (warning symbols might render wider)
    const line1Text = 'TAGGED CONTACT';
    const line2Text = warningLine2;
    const maxTextWidth = Math.max(line1Text.length, line2Text.length);
    const warningWidth = maxTextWidth + 4; // Add space for symbols
    const warningX = canvas.width - (warningWidth + 4) * 9.6 - 10;
    ctx.fillText('╔' + '═'.repeat(warningWidth + 2) + '╗', warningX, 20);
    ctx.fillText('║ ' + warningLine1.padEnd(warningWidth, ' ') + ' ║', warningX, 40);
    ctx.fillText('║ ' + warningLine2.padEnd(warningWidth, ' ') + ' ║', warningX, 60);
    ctx.fillText('╚' + '═'.repeat(warningWidth + 2) + '╝', warningX, 80);

    ctx.fillText('╚' + '═'.repeat(warningWidth + 2) + '╝', warningX, 80);
  } else if (taggedWarning && (Date.now() - taggedWarning.timestamp >= 10000)) {
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


// Client detection
let clientTracks = new Map();
let trackMode = 'aps'; // 'aps', 'clients', or 'both'
let clientData = [];

function updateTrackMode(mode) {
  trackMode = mode;
  console.log('Track mode changed to:', mode);
}

async function fetchClients() {
  try {
    const response = await fetch('/api/clients');
    const data = await response.json();
    clientData = data.clients || [];
    
    // Update stats
    document.getElementById('client-count').textContent = data.count || 0;
    document.getElementById('current-channel').textContent = data.currentChannel || 1;
    
    // Count random MACs
    const randomCount = clientData.filter(c => c.isRandom).length;
    document.getElementById('random-mac-count').textContent = randomCount;
    
    // Update client tracks
    updateClientTracks(clientData);
    
    // Update clients table
    updateClientsTable(clientData);
    
  } catch (error) {
    console.error('Failed to fetch clients:', error);
  }
}

function updateClientTracks(clients) {
  const now = Date.now();
  
  clients.forEach(client => {
    if (!clientTracks.has(client.mac)) {
      clientTracks.set(client.mac, {
        mac: client.mac,
        manufacturer: client.manufacturer,
        history: [],
        firstSeen: now,
        lastSeen: now,
        validationCount: 0,
        isConfirmed: false,
        isRandom: client.isRandom
      });
    }
    
    const track = clientTracks.get(client.mac);
    track.rssi = client.rssi;
    track.channel = client.channel;
    track.lastSeen = now;
    track.manufacturer = client.manufacturer;
    track.probeCount = client.probeCount;
    
    // Validation logic
    if (!track.isConfirmed) {
      track.validationCount++;
      if (track.validationCount >= config.validationThreshold) {
        track.isConfirmed = true;
        logMessage(`NEW CLIENT DETECTED: ${client.mac} (${client.manufacturer})`, 'new');
      }
    }
    
    // Add to history
    const distance = rssiToDistance(client.rssi);
    const angle = Math.random() * 2 * Math.PI; // Random angle for now
    const x = Math.cos(angle) * distance;
    const y = Math.sin(angle) * distance;
    
    track.history.push({ rssi: client.rssi, timestamp: now, x, y });
    if (track.history.length > config.trackHistory) {
      track.history.shift();
    }
  });
  
  // Remove old clients
  for (const [mac, track] of clientTracks.entries()) {
    if (now - track.lastSeen > 60000) {
      clientTracks.delete(mac);
    }
  }
}

function updateClientsTable(clients) {
  const tbody = document.getElementById('clients-tbody');
  
  if (clients.length === 0) {
    tbody.innerHTML = '<tr><td colspan="7" style="padding: 20px; text-align: center; color: #0ff;">No clients detected yet...</td></tr>';
    return;
  }
  
  // Sort by RSSI (strongest first)
  clients.sort((a, b) => b.rssi - a.rssi);
  
  tbody.innerHTML = clients.map(client => {
    const macColor = client.isRandom ? '#f80' : '#0f0';
    const typeText = client.isRandom ? 'Random' : 'Real';
    const typeColor = client.isRandom ? '#f80' : '#0ff';
    
    return `
      <tr style="border-bottom: 1px solid rgba(0,255,0,0.2);">
        <td style="padding: 8px; color: ${macColor}; font-weight: bold;">${client.mac}</td>
        <td style="padding: 8px;">${client.manufacturer}</td>
        <td style="padding: 8px; text-align: center; color: ${getRSSIColor(client.rssi)};">${client.rssi} dBm</td>
        <td style="padding: 8px; text-align: center;">${client.channel}</td>
        <td style="padding: 8px; text-align: center;">${client.probeCount}</td>
        <td style="padding: 8px; text-align: center;">${client.lastSeen}s ago</td>
        <td style="padding: 8px; text-align: center; color: ${typeColor};">${typeText}</td>
      </tr>
    `;
  }).join('');
}

function getRSSIColor(rssi) {
  if (rssi > -50) return '#0f0';
  if (rssi > -70) return '#ff0';
  return '#f00';
}

// Update HUD to show client count
const originalUpdateHUD = updateHUD;
updateHUD = function() {
  originalUpdateHUD();
  
  // Add client count to HUD if in client or both mode
  if (trackMode === 'clients' || trackMode === 'both') {
    const clientCount = clientTracks.size;
    const confirmedClients = Array.from(clientTracks.values()).filter(t => t.isConfirmed).length;
    // This will be displayed in the corner boxes
  }
};

// Modify drawRadar to include clients
const originalDrawTracks = drawTracks;
function drawTracks() {
  if (trackMode === 'aps' || trackMode === 'both') {
    originalDrawTracks();
  }
  
  if (trackMode === 'clients' || trackMode === 'both') {
    drawClientTracks();
  }
}

function drawClientTracks() {
  const canvas = document.getElementById('radar-canvas');
  const ctx = canvas.getContext('2d');
  const centerX = canvas.width / 2;
  const centerY = canvas.height / 2;
  const maxRadius = Math.min(centerX, centerY) - 50;
  
  clientTracks.forEach(track => {
    if (!track.isConfirmed) return; // Only show confirmed clients
    
    const history = track.history;
    if (history.length === 0) return;
    
    // Draw track trail (last 5 positions)
    ctx.strokeStyle = 'rgba(0,255,255,0.3)';
    ctx.lineWidth = 1;
    ctx.setLineDash([2, 2]);
    ctx.beginPath();
    for (let i = 0; i < history.length; i++) {
      const pos = history[i];
      const distance = rssiToDistance(pos.rssi);
      const normalizedDist = (distance / 100) * maxRadius;
      const angle = Math.atan2(pos.y, pos.x);
      const x = centerX + Math.cos(angle) * normalizedDist;
      const y = centerY + Math.sin(angle) * normalizedDist;
      
      if (i === 0) {
        ctx.moveTo(x, y);
      } else {
        ctx.lineTo(x, y);
      }
    }
    ctx.stroke();
    ctx.setLineDash([]);
    
    // Draw current position as diamond
    const latest = history[history.length - 1];
    const distance = rssiToDistance(latest.rssi);
    const normalizedDist = (distance / 100) * maxRadius;
    const angle = Math.atan2(latest.y, latest.x);
    const x = centerX + Math.cos(angle) * normalizedDist;
    const y = centerY + Math.sin(angle) * normalizedDist;
    
    // Diamond symbol
    const size = 8;
    ctx.fillStyle = track.isRandom ? '#f80' : '#0ff';
    ctx.beginPath();
    ctx.moveTo(x, y - size);
    ctx.lineTo(x + size, y);
    ctx.lineTo(x, y + size);
    ctx.lineTo(x - size, y);
    ctx.closePath();
    ctx.fill();
    
    // Label
    ctx.fillStyle = track.isRandom ? '#f80' : '#0ff';
    ctx.font = '10px monospace';
    const label = `${track.mac.substring(0, 8)}... (${track.manufacturer})`;
    ctx.fillText(label, x + 10, y - 10);
  });
}

// Start client fetching when on clients tab
setInterval(() => {
  if (document.getElementById('clients-view').classList.contains('active') || trackMode !== 'aps') {
    fetchClients();
  }
}, 3000);

// Initial fetch
fetchClients();

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
  setupClientDetection();
  
  Serial.println("Wi-Dar AP started");
  Serial.print("IP: ");
  Serial.println(WiFi.softAPIP());
  
  server.on("/", handleRoot);
  server.on("/api/scan", handleScanAPI);
  server.on("/api/clients", handleClientsAPI);
  server.on("/api/config", HTTP_GET, handleConfigAPI);
  server.on("/api/config", HTTP_POST, handleConfigAPI);
  server.on("/api/tags", HTTP_GET, handleTagsAPI);
  server.on("/api/tags", HTTP_POST, handleTagsAPI);
  server.on("/api/tags", HTTP_DELETE, handleTagsAPI);
  
  server.begin();
  Serial.println("Server started");
}

void loop() {
  hopChannel();
  server.handleClient();
}
