/* * DPSI AUTONOMOUS BELL - V3.23 PRO-FAST */ 
#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <LiquidCrystal_I2C.h>
#include <ESPmDNS.h>
#include "time.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
const char* ssid = "******";
const char* password = "*****";
const char* www_username = "admin";
const char* www_password = "admin";
const char* ntpServer = "pool.ntp.org";
#define RELAY_PIN 26
#define BUTTON_PIN 4
LiquidCrystal_I2C lcd(0x27, 16, 2);
WebServer server(80);
String scheduleJson = "[]";
bool holidayMode = false;
bool bellActive = false;
unsigned long bellStartTime = 0;
unsigned long currentBellDuration = 0;
String lastTriggeredMin = "";
unsigned long wifiConnectedMillis = 0;
bool showIP = true;
volatile int pendingRingDur = 0;
void logEvent(String msg) {
 struct tm ti;
 if(getLocalTime(&ti)) Serial.printf("[%02d:%02d:%02d] %s\n", ti.tm_hour, ti.tm_min, ti.tm_sec, msg.c_str());
 else Serial.println("[SYS] " + msg);
}
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
<title>DPSI Autonomous Bell</title>
<meta name="viewport" content="width=device-width, initial-scale=1">
<style>
  :root { --p: #1a237e; --a: #ffc107; --d: #d32f2f; --s: #2e7d32; --h: #9c27b0; }
  body { font-family: sans-serif; background: #f4f7f6; padding: 20px; display: flex; justify-content: center; }
  .card { width: 100%; max-width: 500px; background: white; padding: 25px; border-radius: 20px; box-shadow: 0 10px 30px rgba(0,0,0,0.1); border-top: 10px solid var(--p); text-align: center; }
  #clock { font-size: 3.5rem; font-weight: bold; color: var(--d); margin: 10px 0; }
  .holiday-box { background: #f3e5f5; padding: 12px; border-radius: 12px; margin: 15px 0; border: 2px solid var(--h); display: flex; justify-content: space-between; align-items: center; }
  .btn { padding: 15px; border: none; border-radius: 12px; cursor: pointer; width: 100%; font-weight: bold; margin: 8px 0; color: white; transition: transform 0.1s; }
  .btn:active { transform: scale(0.95); opacity: 0.8; }
  .row { border: 1px solid #ddd; border-radius: 12px; padding: 15px; margin-bottom: 12px; background: #fafafa; }
  .days-grid { display: grid; grid-template-columns: repeat(7, 1fr); gap: 4px; margin-top: 10px; }
  input[type="time"] { font-size: 1.2rem; border: 1px solid #ccc; border-radius: 5px; }
  .switch { position: relative; display: inline-block; width: 50px; height: 24px; }
  .switch input { opacity: 0; width: 0; height: 0; }
  .slider { position: absolute; cursor: pointer; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; transition: .4s; border-radius: 24px; }
  input:checked + .slider { background-color: var(--h); }
</style>
</head><body>
<div class="card">
  <h2 style="color:var(--p)">DPSI AUTONOMOUS BELL</h2>
  <div id="clock">00:00:00</div>
  <div id="date-disp" style="color:#666; margin-bottom:10px;"></div>
  <div class="holiday-box">
    <span style="color:var(--h); font-weight:bold;">HOLIDAY MODE</span>
    <label class="switch"><input type="checkbox" id="holidayToggle" onclick="toggleHoliday()"><span class="slider"></span></label>
  </div>
  <button class="btn" style="background:var(--a); color:black;" onclick="fetch('/ring?dur=3')"> INSTANT RING [ 3 Sec ]</button>
  <hr style="opacity:0.1;"><div id="bellList"></div>
  <button onclick="addRow()" style="background:#e8eaf6; color:var(--p); padding:10px; border-radius:10px; border:none; width:100%; cursor:pointer; font-weight:bold; margin-bottom:10px;">+ ADD BELL</button>
  <button class="btn" style="background:var(--s);" onclick="save()">SAVE & SYNC SYSTEM</button>
</div>
<script>
function toggleHoliday() { fetch('/setHoliday?val=' + (document.getElementById('holidayToggle').checked ? 1 : 0)); }
setInterval(() => {
  const n = new Date();
  document.getElementById('clock').innerText = n.toLocaleTimeString('en-GB', {hour12:false});
  document.getElementById('date-disp').innerText = n.toLocaleDateString('en-GB', {weekday:'long', day:'numeric', month:'short'});
}, 1000);
function addRow(data = {t:"08:00", d:[1,1,1,1,1,0,0], s:5}) {
  const div = document.createElement('div');
  div.className = "row";
  let days = ['M','T','W','T','F','S','S'];
  let chks = '';
  days.forEach((day, i) => { chks += `<div style="text-align:center; font-size:10px;">${day}<br><input type="checkbox" class="d${i}" ${data.d[i]?'checked':''}></div>`; });
  div.innerHTML = `<div style="display:flex; justify-content:space-between; align-items:center;"><input type="time" class="time-val" value="${data.t}"><span><input type="number" class="dur-val" value="${data.s}" style="width:35px">s</span><button onclick="this.parentElement.parentElement.remove()" style="color:red; border:none; background:none; font-weight:bold; cursor:pointer;">DEL</button></div><div class="days-grid">${chks}</div>`;
  document.getElementById('bellList').appendChild(div);
}
function save() {
  let t = []; document.querySelectorAll('.row').forEach(row => {
    let d = []; for(let i=0; i<7; i++) d.push(row.querySelector('.d'+i).checked ? 1 : 0);
    t.push({t:row.querySelector('.time-val').value, d:d, s:parseInt(row.querySelector('.dur-val').value)});
  });
  fetch('/save?data=' + encodeURIComponent(JSON.stringify(t))).then(() => { alert("System Synced!"); location.reload(); });
}
window.onload = () => {
  fetch('/list').then(r => r.json()).then(data => { if(data && data.length > 0) data.forEach(item => addRow(item)); else addRow(); });
  fetch('/getHoliday').then(r => r.text()).then(val => { document.getElementById('holidayToggle').checked = (val == "1"); });
};
</script></body></html>)rawliteral";

void triggerBell(int durSec, String source) {
 if (durSec < 1) durSec = 1;
 digitalWrite(RELAY_PIN, HIGH); // 3.3V ON
 bellStartTime = millis();
 currentBellDuration = (unsigned long)durSec * 1000;
 bellActive = true;
 logEvent("RING START | Source: " + source);
}
void setup() {
 WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
 Serial.begin(115200);
 pinMode(RELAY_PIN, OUTPUT);
 digitalWrite(RELAY_PIN, LOW); // 0V Idle
 pinMode(BUTTON_PIN, INPUT_PULLUP);
 lcd.init(); lcd.backlight();
 lcd.setCursor(2, 0); lcd.print("AUTONOMOUS");
 lcd.setCursor(6, 1); lcd.print("BELL");
 WiFi.begin(ssid, password);
 int attempt = 0;
 while (WiFi.status() != WL_CONNECTED && attempt < 20) { delay(400); attempt++; }
 MDNS.begin("dpsi-bell");
 configTime(19800, 0, ntpServer, "time.google.com");
 wifiConnectedMillis = millis();
 if (LittleFS.begin(true)) {
   if (LittleFS.exists("/sched.json")) {
     File f = LittleFS.open("/sched.json", "r");
     scheduleJson = f.readString();
     f.close();
   }
   if (LittleFS.exists("/holiday.txt")) {
     File f = LittleFS.open("/holiday.txt", "r");
     holidayMode = (f.readString() == "1");
     f.close();
   }
 }
 server.on("/ring", []() {
   int d = server.arg("dur").toInt();
   triggerBell((d > 0 ? d : 3), "WEB_INSTANT"); // Trigger immediately inside the request
   server.send(200, "text/plain", "OK");
 });

 server.on("/", []() {
   if(server.authenticate(www_username, www_password)) server.send_P(200, "text/html", index_html);
   else server.requestAuthentication();
 });

 server.on("/list", []() { server.send(200, "application/json", scheduleJson); });
 server.on("/save", []() {
   scheduleJson = server.arg("data");
   File f = LittleFS.open("/sched.json", "w");
   if (f) { f.print(scheduleJson); f.close(); }
   server.send(200, "text/plain", "OK");
 });
 server.on("/setHoliday", []() {
   holidayMode = (server.arg("val") == "1");
   File f = LittleFS.open("/holiday.txt", "w");
   if (f) { f.print(server.arg("val")); f.close(); }
   server.send(200, "text/plain", "OK");
 });
 server.on("/getHoliday", []() { server.send(200, "text/plain", holidayMode ? "1" : "0"); });
 server.begin();
 lcd.clear();
}

void loop() {
 server.handleClient();
 unsigned long cm = millis();
 if (bellActive && (cm - bellStartTime >= currentBellDuration)) {
   digitalWrite(RELAY_PIN, LOW); // 0V OFF
   bellActive = false;
   logEvent("RING STOP.");
 }
 static unsigned long lastSec = 0;
 if (cm - lastSec >= 1000) {
   struct tm ti;
   if(getLocalTime(&ti)) {
     if (showIP) {
       lcd.setCursor(0,0); lcd.print("IP:" + WiFi.localIP().toString());
       lcd.setCursor(0,1); lcd.print("WEB: dpsi-bell  ");
       if (cm - wifiConnectedMillis > 30000) { showIP = false; lcd.clear(); }
     } else {
       char top[17]; strftime(top, 17, "TIME: %H:%M:%S", &ti);
       lcd.setCursor(0,0); lcd.print(top);
       lcd.setCursor(0,1);
       if(holidayMode) lcd.print("MODE: HOLIDAY   ");
       else {
          lcd.print("NEXT: " + getNextBell(ti) + "      ");
       }
     }
     char curM[6]; sprintf(curM, "%02d:%02d", ti.tm_hour, ti.tm_min);
     if (String(curM) != lastTriggeredMin) {
       if (!holidayMode) {
         StaticJsonDocument<2048> doc;
         if (!deserializeJson(doc, scheduleJson)) {
           JsonArray arr = doc.as<JsonArray>();
           int dayIdx = (ti.tm_wday == 0) ? 6 : ti.tm_wday - 1;
           for (JsonObject item : arr) {
             if (String(curM) == item["t"].as<String>() && item["d"][dayIdx] == 1) {
               triggerBell(item["s"].as<int>(), "SCHEDULE");
               break;
             }
           }
         }
       }
       lastTriggeredMin = String(curM);
     }
   }
   lastSec = cm;
 }
 if (digitalRead(BUTTON_PIN) == LOW && !bellActive) {
   triggerBell(3, "PHYSICAL_BUTTON");
   delay(300);
 }
}
String getNextBell(struct tm ti) {
 StaticJsonDocument<1500> doc;
 if (deserializeJson(doc, scheduleJson)) return "--:--";
 int curT = (ti.tm_hour * 60) + ti.tm_min;
 int dayIdx = (ti.tm_wday == 0) ? 6 : ti.tm_wday - 1;
 int minDiff = 1500; String next = "--:--";
 JsonArray arr = doc.as<JsonArray>();
 for (JsonObject item : arr) {
   if (item["d"][dayIdx] == 1) {
     String t = item["t"].as<String>();
     int bellT = (t.substring(0, 2).toInt() * 60) + t.substring(3, 5).toInt();
     int diff = bellT - curT;
     if (diff > 0 && diff < minDiff) { minDiff = diff; next = t; }
   }
 }
 return next;
}
