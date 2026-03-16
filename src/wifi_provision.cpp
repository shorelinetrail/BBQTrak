#include "wifi_provision.h"
#include <WiFi.h>
#include <DNSServer.h>
#include <Preferences.h>

static const char PORTAL_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html><head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>BBQTrak WiFi Setup</title>
<style>
* { margin:0; padding:0; box-sizing:border-box; }
body { font-family:-apple-system,sans-serif; background:#1a1a2e; color:#e0e0e0; min-height:100vh;
    display:flex; align-items:center; justify-content:center; }
.card { background:#16213e; border-radius:16px; padding:32px; max-width:400px; width:90%;
    border:1px solid #0f3460; }
h1 { color:#e74c3c; font-size:1.4rem; text-align:center; margin-bottom:4px; }
.sub { color:#7f8c8d; text-align:center; font-size:0.8rem; margin-bottom:24px; }
label { display:block; font-size:0.85rem; color:#7f8c8d; margin-bottom:4px; margin-top:16px; }
input { width:100%; padding:10px 12px; border:1px solid #1a4a8a; border-radius:8px;
    background:#0f3460; color:#e0e0e0; font-size:1rem; }
button { width:100%; padding:12px; border:none; border-radius:8px; background:#27ae60;
    color:#fff; font-size:1rem; font-weight:600; cursor:pointer; margin-top:24px; }
button:hover { opacity:0.9; }
.msg { color:#27ae60; text-align:center; margin-top:16px; font-size:0.9rem; display:none; }
.nets { margin-top:12px; max-height:150px; overflow-y:auto; }
.net { padding:8px 12px; border-radius:6px; cursor:pointer; font-size:0.9rem; }
.net:hover { background:#0f3460; }
.rssi { float:right; color:#7f8c8d; font-size:0.8rem; }
</style>
</head><body>
<div class="card">
    <h1>BBQTrak</h1>
    <div class="sub">WiFi Configuration</div>
    <div id="nets" class="nets"></div>
    <form id="f" method="POST" action="/save">
        <label>WiFi Network (SSID)</label>
        <input type="text" name="ssid" id="ssid" required maxlength="32">
        <label>Password</label>
        <input type="password" name="pass" id="pass" maxlength="63">
        <button type="submit">Connect</button>
    </form>
    <div id="msg" class="msg">Saved! Rebooting...</div>
</div>
<script>
fetch('/scan').then(r=>r.json()).then(nets=>{
    let el=document.getElementById('nets');
    nets.forEach(n=>{
        let d=document.createElement('div');
        d.className='net';
        d.innerHTML=n.ssid+'<span class="rssi">'+n.rssi+' dBm</span>';
        d.onclick=()=>{document.getElementById('ssid').value=n.ssid;};
        el.appendChild(d);
    });
});
document.getElementById('f').onsubmit=function(e){
    e.preventDefault();
    let fd=new FormData(this);
    fetch('/save',{method:'POST',body:new URLSearchParams(fd)})
        .then(()=>{document.getElementById('msg').style.display='block';});
};
</script>
</body></html>
)rawliteral";

bool WiFiProvisioner::loadCredentials(String& ssid, String& password) {
    Preferences prefs;
    prefs.begin("bbqtrak", true);  // read-only
    ssid = prefs.getString("ssid", "");
    password = prefs.getString("pass", "");
    prefs.end();
    return ssid.length() > 0;
}

void WiFiProvisioner::saveCredentials(const String& ssid, const String& password) {
    Preferences prefs;
    prefs.begin("bbqtrak", false);
    prefs.putString("ssid", ssid);
    prefs.putString("pass", password);
    prefs.end();
    Serial.printf("WiFi credentials saved for: %s\n", ssid.c_str());
}

void WiFiProvisioner::clearCredentials() {
    Preferences prefs;
    prefs.begin("bbqtrak", false);
    prefs.remove("ssid");
    prefs.remove("pass");
    prefs.end();
    Serial.println("WiFi credentials cleared");
}

bool WiFiProvisioner::connectSaved(uint32_t timeoutMs) {
    String ssid, pass;
    if (!loadCredentials(ssid, pass)) {
        Serial.println("No saved WiFi credentials");
        return false;
    }

    Serial.printf("Connecting to saved WiFi: %s\n", ssid.c_str());
    WiFi.setHostname("bbqtrak");
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), pass.c_str());

    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - start) < timeoutMs) {
        delay(250);
        Serial.print(".");
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("Connected! IP: %s\n", WiFi.localIP().toString().c_str());
        return true;
    }

    Serial.println("Failed to connect with saved credentials");
    WiFi.disconnect();
    return false;
}

void WiFiProvisioner::startPortal(AsyncWebServer& server) {
    WiFi.mode(WIFI_AP);
    WiFi.softAP("BBQTrak-Setup");
    delay(100);
    Serial.printf("AP started: BBQTrak-Setup\n");
    Serial.printf("Portal at: http://%s\n", WiFi.softAPIP().toString().c_str());

    // Start DNS server to redirect all domains to our IP (captive portal)
    DNSServer* dns = new DNSServer();
    dns->start(53, "*", WiFi.softAPIP());
    _dns = dns;
    _portalActive = true;

    // Serve the config page on any path (captive portal behavior)
    server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send_P(200, "text/html", PORTAL_HTML);
    });

    // Captive portal detection endpoints
    server.on("/generate_204", HTTP_GET, [](AsyncWebServerRequest* r) {
        r->send_P(200, "text/html", PORTAL_HTML);
    });
    server.on("/hotspot-detect.html", HTTP_GET, [](AsyncWebServerRequest* r) {
        r->send_P(200, "text/html", PORTAL_HTML);
    });
    server.on("/connecttest.txt", HTTP_GET, [](AsyncWebServerRequest* r) {
        r->send_P(200, "text/html", PORTAL_HTML);
    });

    // WiFi scan endpoint
    server.on("/scan", HTTP_GET, [](AsyncWebServerRequest* request) {
        int n = WiFi.scanComplete();
        if (n == WIFI_SCAN_FAILED) {
            WiFi.scanNetworks(true);  // async scan
            request->send(200, "application/json", "[]");
            return;
        }
        String json = "[";
        for (int i = 0; i < n; i++) {
            if (i > 0) json += ",";
            json += "{\"ssid\":\"";
            json += WiFi.SSID(i);
            json += "\",\"rssi\":";
            json += String(WiFi.RSSI(i));
            json += "}";
        }
        json += "]";
        WiFi.scanDelete();
        WiFi.scanNetworks(true);  // Start next scan
        request->send(200, "application/json", json);
    });

    // Save credentials endpoint
    server.on("/save", HTTP_POST, [this](AsyncWebServerRequest* request) {
        if (request->hasParam("ssid", true) && request->hasParam("pass", true)) {
            String ssid = request->getParam("ssid", true)->value();
            String pass = request->getParam("pass", true)->value();
            saveCredentials(ssid, pass);
            request->send(200, "text/plain", "OK");
            _shouldReboot = true;
        } else {
            request->send(400, "text/plain", "Missing parameters");
        }
    });

    // Start initial scan
    WiFi.scanNetworks(true);

    server.begin();
}

void WiFiProvisioner::update() {
    if (_portalActive && _dns) {
        static_cast<DNSServer*>(_dns)->processNextRequest();
    }
}
