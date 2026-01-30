# ESP32-S3 SuperMini HID Controller
# Author: symonty

This project transforms a **SuperMini ESP32-S3** into a powerful, web-controllable USB HID Keyboard. It features a premium web interface to send individual keys or custom strings with full modifier support (including Mac Command/Cmd). Originally designed to use as a method to wake up a sleeping computer from a remote location. but can be used for other purposes.

## 🚀 Features

- **Native USB HID Emulation**: Acts as a physical keyboard directly recognized by the host.
- **Advanced Mac Support**: Includes `Cmd` (GUI/Win) modifier key specifically for macOS workflows.
- **Custom Text Entry**: Send entire strings of text with combinations of **Ctrl**, **Shift**, **Alt**, and **Cmd**.
- **WiFi Auto-Config**: Automatically enters Access Point mode if WiFi fails, allowing setup via a captive portal.
- **RGB Status LED**: Real-time feedback via the onboard WS2812 (Pin 48).
- **Responsive Web Dashboard**: A sleek, dark-mode glassmorphism interface.

## 🚥 RGB Status Codes (WS2812 - Pin 48)

| Animation | State | Description |
| :--- | :--- | :--- |
| **Flashing Red** | 🔴 Connecting | Attempting to join the WiFi network. |
| **Pulsing Purple** | 🟣 AP Mode | Connection failed. Hotspot active at `192.168.4.1` for WiFi setup. |
| **Rainbow Cycle** | 🌈 Idle | WiFi connected and waiting for commands. |
| **Flashing Blue** | 🔵 Sending | HID command is currently being transmitted to host. |

## 🛠 Hardware Configuration used for this project

- **MCU**: SuperMini ESP32-S3 with 4MB Flash and 2MB PSRAM. https://www.espboards.dev/esp32/esp32-s3-super-mini/
- **Native USB**: Connected via the USB-C port (S3 internal logic).
- **RGB LED**: WS2812 connected to **GPIO 48**.

## 🔌 Serial Debugging & Monitoring

The Serial port (used for logging and viewing the IP address) follows a specific lifecycle to ensure compatibility with the Native USB HID mode:

1. **Pre-Connection**: Serial is active immediately on boot at **115200 baud**.
2. **Monitoring**: Run `pio device monitor` to see the connection logs and retrieve the allocated **IP Address**.
3. **HID Activation**: Once WiFi connects, the board activates the native HID keyboard mode. 
4. **Port Reset**: **Note:** Your Serial monitor will likely disconnect or the port will vanish briefly when HID activates as the ESP32 re-enumerates as a Composite Device (HID + CDC). You may need to restart the monitor to continue seeing logs.

## 💾 Firmware Updates & Recovery

If the board becomes unresponsive or fails to appear as a serial port for uploading:

1. **Enter Bootloader Mode**:
   - Press and hold the **BOOT** button.
   - Press the **RESET** button briefly.
   - Release the **BOOT** button.
2. **Upload**: Use PlatformIO to upload the new firmware while in this state. The port will typically show up as `USB JTAG/serial debug unit`.

## 🌐 Web Interface

Navigate to the IP address shown in the serial logs (or `http://esp32-hid.local` via mDNS) to access the controller:

- **Send Space**: Instant large button for spacebar.
- **Send Text**: Input field for custom strings.
- **Modifiers**: Checkboxes for Ctrl, Shift, Alt, and Cmd to hold down while the text is "typed".

### 📶 WiFi Setup (First Time / Failure)

If the device cannot connect to a saved network (or it's the first run):
1. The LED will pulse **Purple**.
2. Connect to the WiFi hotspot named **`ESP32-HID-Config`**.
3. A portal should open automatically (if not, go to `192.168.4.1`).
4. Enter your SSID and Password, then click **Save & Reboot**.
5. The device will store these settings permanently and reconnect.

---
*Developed with ❤️ by **symonty** for ESP32-S3 SuperMini.*
