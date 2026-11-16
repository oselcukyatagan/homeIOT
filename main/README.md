# homeIOT
IOT applications for my home. An ESP32-S3 controls the motorized blinds and the RGB LED strip in the living room. Both can be controlled from Alexa / Google Home through SinricPro and from a local web page.

## Features
* **Time-Based Position Control:** The travel time of the motor is mapped to a 0–100% scale, no limit switches are needed.
* **Position Memory:** The blinds position is saved to flash every 5% and when the blinds stop, so it is kept after a reboot or a power cut.
* **Relay Interlocking:** A 1-second delay is enforced during direction changes to prevent back-EMF spikes and contact welding.
* **Mechanical Settling Delay:** A 50ms pause is added after the direction is selected so the relay contacts are seated before power is applied.
* **SinricPro Integration:** Blinds position, light power, color and brightness can be controlled remotely.
* **Music Mode:** The LEDs follow the volume picked up by the microphone. DC offset tracking and a low-pass filter are used to clean the signal.
* **Web Dashboard:** Blinds and music mode can be controlled from `http://<board-ip>/`.
* **OTA Updates:** The code can be uploaded over WiFi after the first USB upload.
* **Telnet Debug:** Reset reason, blinds position and free heap are printed on port 23.
* **Watchdog and WiFi Reconnect:** The board is restarted if the loop hangs and the connection is restored automatically.

## Pins
| Function | GPIO |
|---|---|
| Relay: direction | 18 |
| Relay: power | 17 |
| LED red / green / blue | 4 / 5 / 6 |
| Microphone (analog) | 9 |

GPIO19 and GPIO20 should not be used since they are the USB data lines of the ESP32-S3. The microphone should be connected to one of GPIO1–10 since only these pins can be read while WiFi is on.

## Setup

### 1. Arduino IDE
The **esp32 by Espressif Systems** (3.x) board package and the **SinricPro** library need to be installed. The board package URL is added from **File → Preferences**:
`https://espressif.github.io/arduino-esp32/package_esp32_index.json`

### 2. Secrets
Before flashing the code, secrets.h file needs to be created by copying the `envVariablesTemplate.h` file and the variables should be completed with your information.  `secrets.h` is git-ignored.

```cpp
#define WIFI_SSID     "..."
#define WIFI_PASS     "..."
#define APP_KEY       "..."   // SinricPro
#define APP_SECRET    "..."
#define BLINDS_ID     "..."
#define LIGHT_ID      "..."
#define OTA_PASSWORD  "..."
```

### 3. Board Settings
| Setting | Value |
|---|---|
| Board | ESP32S3 Dev Module |
| Flash Size | 16MB (128Mb) |
| Partition Scheme | 16M Flash (3MB APP/9.9MB FATFS) |
| USB CDC On Boot | Disabled |
| Erase All Flash Before Sketch Upload | Disabled |

### 4. Upload
The first upload should be done over USB from the port labelled **COM** on the board. After that, the code can be uploaded over WiFi by selecting the `HomeIOT-ESP32` network port. `espota.exe` needs to be allowed in Windows Firewall for OTA to work.

## Notes
If the blinds position does not match the saved position, the blinds should be moved fully up or down once. The motor stops at its own end limit and the saved position is corrected.
