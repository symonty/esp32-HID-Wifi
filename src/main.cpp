/*
 * ESP32-S3 SuperMini HID Controller
 * Original Author: symonty
 */

#include "USB.h"
#include "USBHIDKeyboard.h"
#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include <WiFi.h>

// --- CONFIGURATION ---
const char *ssid = "";     // Insert your SSID here
const char *password = ""; // Insert your Password here

#ifndef PIN_LED
#define PIN_LED 48
#endif

USBHIDKeyboard Keyboard;
WebServer server(80);
Adafruit_NeoPixel pixels(1, PIN_LED, NEO_GRB + NEO_KHZ800);

// Global status variables for LED
uint32_t lastPixelUpdate = 0;
uint16_t hue = 0;
bool isSending = false;
uint32_t sendFlashEnd = 0;

// --- HTML CONTENT ---
const char INDEX_HTML[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 HID Controller</title>
    <style>
        :root {
            --bg: #0f172a;
            --card: #1e293b;
            --primary: #38bdf8;
            --primary-hover: #0ea5e9;
            --text: #f8fafc;
        }
        body {
            font-family: 'Inter', -apple-system, sans-serif;
            background-color: var(--bg);
            color: var(--text);
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
            margin: 0;
            overflow: hidden;
        }
        .container {
            background-color: var(--card);
            padding: 3rem;
            border-radius: 1.5rem;
            box-shadow: 0 25px 50px -12px rgba(0, 0, 0, 0.5);
            text-align: center;
            width: 100%;
            max-width: 400px;
            border: 1px solid rgba(255, 255, 255, 0.1);
            transition: transform 0.3s ease;
        }
        .container:hover {
            transform: translateY(-5px);
        }
        h1 {
            font-size: 1.875rem;
            font-weight: 700;
            margin-bottom: 0.5rem;
            background: linear-gradient(to right, #38bdf8, #818cf8);
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
        }
        p {
            color: #94a3b8;
            margin-bottom: 2rem;
            font-size: 0.875rem;
        }
        .btn {
            background-color: var(--primary);
            color: white;
            border: none;
            padding: 1rem 2rem;
            font-size: 1.125rem;
            font-weight: 600;
            border-radius: 0.75rem;
            cursor: pointer;
            width: 100%;
            transition: all 0.2s cubic-bezier(0.4, 0, 0.2, 1);
            box-shadow: 0 4px 6px -1px rgba(56, 189, 248, 0.3);
            display: flex;
            align-items: center;
            justify-content: center;
            gap: 0.5rem;
        }
        .btn:hover {
            background-color: var(--primary-hover);
            transform: scale(1.02);
            box-shadow: 0 10px 15px -3px rgba(56, 189, 248, 0.4);
        }
        .btn:active {
            transform: scale(0.98);
        }
        .status {
            margin-top: 1.5rem;
            font-size: 0.75rem;
            color: #64748b;
        }
        .indicator {
            display: inline-block;
            width: 8px;
            height: 8px;
            border-radius: 50%;
            background-color: #10b981;
            margin-right: 0.5rem;
            box-shadow: 0 0 10px #10b981;
        }
        .input-group {
            margin-top: 2rem;
            text-align: left;
        }
        input[type="text"] {
            width: 100%;
            padding: 0.75rem;
            border-radius: 0.5rem;
            border: 1px solid rgba(255, 255, 255, 0.1);
            background: rgba(0, 0, 0, 0.2);
            color: white;
            margin-bottom: 1rem;
            box-sizing: border-box;
        }
        .modifiers {
            display: flex;
            gap: 1rem;
            margin-bottom: 1.5rem;
            font-size: 0.875rem;
            color: #94a3b8;
        }
        .modifiers label {
            display: flex;
            align-items: center;
            gap: 0.25rem;
            cursor: pointer;
        }
        .divider {
            height: 1px;
            background: rgba(255, 255, 255, 0.1);
            margin: 2rem 0;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>HID Controller</h1>
        <p>ESP32-S3 Virtual keyboard</p>
        
        <button class="btn" onclick="sendSpace()">
            <svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
                <path d="M18 13v6a2 2 0 0 1-2 2H8a2 2 0 0 1-2-2v-6"></path>
                <path d="M6 9h12"></path>
            </svg>
            Send Space
        </button>

        <div class="divider"></div>

        <div class="input-group">
            <input type="text" id="textInput" placeholder="Enter text to send...">
            <div class="modifiers">
                <label><input type="checkbox" id="ctrl"> Ctrl</label>
                <label><input type="checkbox" id="shift"> Shift</label>
                <label><input type="checkbox" id="alt"> Alt</label>
                <label><input type="checkbox" id="cmd"> Cmd</label>
            </div>
            <button class="btn" onclick="sendText()">
                <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
                    <polyline points="9 10 4 15 9 20"></polyline>
                    <path d="M20 4v7a4 4 0 0 1-4 4H4"></path>
                </svg>
                Send Text
            </button>
        </div>

        <div class="status">
            <span class="indicator"></span> Connected via USB
        </div>
    </div>

    <script>
        function sendSpace() {
            const btn = event.currentTarget;
            btn.style.opacity = '0.5';
            btn.disabled = true;

            fetch('/send_space')
                .finally(() => {
                    setTimeout(() => {
                        btn.style.opacity = '1';
                        btn.disabled = false;
                    }, 100);
                });
        }

        function sendText() {
            const btn = event.currentTarget;
            const text = document.getElementById('textInput').value;
            const ctrl = document.getElementById('ctrl').checked;
            const shift = document.getElementById('shift').checked;
            const alt = document.getElementById('alt').checked;
            const cmd = document.getElementById('cmd').checked;

            if (!text && !ctrl && !shift && !alt && !cmd) return;

            btn.style.opacity = '0.5';
            btn.disabled = true;

            const url = `/send_text?t=${encodeURIComponent(text)}&c=${ctrl}&s=${shift}&a=${alt}&m=${cmd}`;
            
            fetch(url)
                .finally(() => {
                    setTimeout(() => {
                        btn.style.opacity = '1';
                        btn.disabled = false;
                    }, 100);
                });
        }
    </script>
</body>
</html>
)=====";

// --- HANDLERS ---
void handleRoot() { server.send(200, "text/html", INDEX_HTML); }

void handleSendSpace() {
  isSending = true;
  sendFlashEnd = millis() + 300;
  Keyboard.write(' '); // Send Space key
  server.send(200, "text/plain", "OK");
  Serial.println("Space Bar Sent!");
}

void handleSendText() {
  isSending = true;
  sendFlashEnd = millis() + 300;
  String text = server.arg("t");
  bool ctrl = server.arg("c") == "true";
  bool shift = server.arg("s") == "true";
  bool alt = server.arg("a") == "true";
  bool cmd = server.arg("m") == "true";

  Serial.print("Sending Text: ");
  if (ctrl) {
    Serial.print("[Ctrl] ");
    Keyboard.press(KEY_LEFT_CTRL);
  }
  if (shift) {
    Serial.print("[Shift] ");
    Keyboard.press(KEY_LEFT_SHIFT);
  }
  if (alt) {
    Serial.print("[Alt] ");
    Keyboard.press(KEY_LEFT_ALT);
  }
  if (cmd) {
    Serial.print("[Cmd] ");
    Keyboard.press(KEY_LEFT_GUI);
  }

  Serial.println(text);

  if (text.length() > 0) {
    Keyboard.print(text);
  }

  Keyboard.releaseAll();
  server.send(200, "text/plain", "OK");
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n--- ESP32-S3 HID Starting ---");

  pixels.begin();
  pixels.setBrightness(50);

  // Connect to WiFi
  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);

  bool ledOn = false;
  while (WiFi.status() != WL_CONNECTED) {
    // Flash RED while connecting
    ledOn = !ledOn;
    pixels.setPixelColor(0, ledOn ? pixels.Color(255, 0, 0) : 0);
    pixels.show();

    delay(500);
    Serial.print(".");

    if (millis() > 30000) {
      Serial.println("\nWiFi connection failed! Restarting...");
      ESP.restart();
    }
  }

  Serial.println("\nWiFi Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Initialize HID ONLY after WiFi is connected
  Serial.println("Activating HID Mode...");
  Keyboard.begin();
  USB.begin();

  // Setup Server
  server.on("/", handleRoot);
  server.on("/send_space", handleSendSpace);
  server.on("/send_text", handleSendText);
  server.begin();
  Serial.println("HTTP Server Started");

  if (MDNS.begin("esp32-hid")) {
    Serial.println("mDNS responder started: http://esp32-hid.local");
  }
}

void loop() {
  server.handleClient();

  uint32_t now = millis();

  if (isSending) {
    if (now < sendFlashEnd) {
      pixels.setPixelColor(0, pixels.Color(0, 0, 255)); // Flash Blue
      pixels.show();
    } else {
      isSending = false;
    }
  } else {
    // Rainbow cycle when idle
    if (now - lastPixelUpdate > 20) {
      lastPixelUpdate = now;
      hue += 256;
      pixels.setPixelColor(0, pixels.gamma32(pixels.ColorHSV(hue)));
      pixels.show();
    }
  }
}
