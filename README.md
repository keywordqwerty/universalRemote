# 📡 ESP32 Universal IR Web Cloner & Blaster

> **Project Status: Alpha Prototype**  
> An embedded IoT signal-capture system designed to capture arbitrary infrared packets in real time and re-transmit them on-demand via a local web interface.

---

## 🎯 The "Why"

Consumer electronics still depend heavily on proprietary, fragmented infrared remote protocols. Off-the-shelf universal remotes frequently fail with modern appliances like HVACs and split-system air conditioners, which transmit their entire operational state (temperature, fan profile, vane oscillation, timers) in massive data bursts rather than a simple 32-bit toggle key.

This project was built to solve two main problems:
1. **Ditch hardcoded protocol dictionaries:** Instead of maintaining lookup tables for every TV, stereo, and AC manufacturer (NEC, RC5, Sony, Gree, Daikin), this prototype dynamically captures raw pulse-width-modulated (PWM) timing arrays directly off the air.
2. **Bridge legacy appliances to the local network:** By serving a zero-dependency web interface straight from the ESP32 microcontroller, any smartphone or browser on the local subnet can trigger IR transmissions without proprietary apps or cloud dependencies.

---

## 🏗️ Architecture & Stack

### Hardware Architecture
* **Compute / Network:** ESP32-WROOM-32 (Dual-core Xtensa 32-bit LX6 @ 240MHz).
* **IR Demodulator / Receiver:** Active 38kHz IR Receiver module wired to GPIO `15`.
* **IR Blaster / Emitter:** 940nm High-Output Infrared LED driven from GPIO `5`.

### Software Stack
* **Framework:** Arduino Core on ESP32 / PlatformIO.
* **Protocol & Signal Processing:** `IRremoteESP8266` / `IRrecv` / `IRsend`.
* **Networking & HTTP Layer:** Native `WebServer.h` and `WiFi.h`.

### Key Architectural Decisions

| Decision | Alternative Evaluated | Why This Choice Was Made |
| :--- | :--- | :--- |
| **Raw Pulse Buffering (`sendRaw`)** | Explicit Protocol Decoders (`sendNEC`, `sendRC5`) | Protocol decoders fail on variable-length air conditioner packets. Storing raw pulse mark/space intervals allows capturing unknown or hybrid protocols without knowing the underlying protocol grammar. |
| **ESP32 over ATmega328P (Arduino Uno)** | Arduino Uno / Nano | Raw HVAC packets frequently exceed 400–800 sample transitions. The ATmega328P's 2KB SRAM exhausts immediately under capture buffer pressure, whereas the ESP32 offers 520KB SRAM and integrated 2.4GHz 802.11 b/g/n. |
| **On-Chip Embedded HTTP Server** | MQTT / External Cloud Broker | Keeps latency minimal, requires zero external cloud infrastructure or broker setup, and guarantees offline local functionality. |

---

## 🔬 Technical Challenges & Debugging Journey

### 1. The HVAC Buffer Overflow & Truncated Packets
* **Symptom:** Standard TV remote codes cloned cleanly, but AC remotes either registered as garbage or failed to trigger the target appliance.
* **Root Cause:** Standard consumer remotes (NEC/RC5) send 12- to 32-bit packets requiring buffer lengths of ~70 samples. Split-system AC remotes transmit comprehensive state snapshots spanning 200–600+ transitions. The default library buffer size truncated incoming sequences.
* **Resolution:** Re-architected buffer parameters:
  ```cpp
  const uint16_t kCaptureBufferSize = 1024;
  const uint8_t kTimeout = 50; // Increased window for extended AC burst pauses
  IRrecv irrecv(kRecvPin, kCaptureBufferSize, kTimeout, true);
  ```
  Allocating a fixed `uint16_t[1024]` persistent buffer allowed capture of complex multi-burst signals.

### 2. Receiver Blindness & Self-Feedback Loops
* **Symptom:** When triggering an outgoing pulse burst, the onboard receiver module immediately detected its own blast, corrupting the stored code with an echo of the transmission.
* **Resolution:** Implemented explicit receiver squelching during transmission cycles:
  ```cpp
  irrecv.disableIRIn(); // Squelch receiver
  irsend.sendRaw(storedRawData, storedRawLength, 38);
  delay(100);
  irrecv.enableIRIn();  // Restore listening state
  ```

### 3. Dynamic Heap Management
* **Symptom:** Intermittent crashes and memory fragmentation after repeatedly cloning remotes.
* **Root Cause:** `resultToRawArray(&results)` dynamically allocates memory on the heap.
* **Resolution:** Explicitly freed the allocated memory right after deep-copying into the static storage array:
  ```cpp
  delete[] rawArray; // Prevent heap leaks
  ```

---

## ⚠️ Known Limitations & Hardware Roadmap

* [ ] **Signal Range & LED Drive Circuit (Current Alpha Bottleneck):**  
  The IR LED is currently driven directly via an ESP32 GPIO pin. ESP32 pins are current-limited to ~12–20mA, yielding weak optical output (effective range < 1 meter).  
  * **Fix in Progress:** Integrating an NPN switching transistor (e.g., 2N2222) with a dedicated current-limiting resistor to drive the diode at ~100–150mA pulse peaks.
* [ ] **Multi-Key Signal Slots:** Expand storage beyond a single volatile slot using LittleFS or an onboard EEPROM-backed key-value store.
* [ ] **Credential Hardcoding:** Migrate hardcoded Wi-Fi credentials to a captive portal (`WiFiManager`) for production security.
* [ ] **TV Carrier Tuning:** Expand support to 36kHz and 40kHz protocols for full television compatibility.

---

## 🚀 Getting Started

### Prerequisites
* [VS Code](https://code.visualstudio.com/) + [PlatformIO IDE Extension](https://platformio.org/)
* ESP32 Development Board (NodeMCU / DOIT DevKit v1)
* 38kHz IR Receiver (VS1838B or similar)
* 940nm IR Emitter Diode

### Pin Configuration
| Component | ESP32 GPIO |
| :--- | :--- |
| IR Receiver OUT | `GPIO 15` |
| IR Transmitter Anode | `GPIO 5` |