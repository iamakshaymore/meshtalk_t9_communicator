<div align="center" markdown="1">

<img src=".github/meshtastic_logo.png" alt="Meshtastic Logo" width="80"/>
<h1>Meshtastic Firmware</h1>

![GitHub release downloads](https://img.shields.io/github/downloads/meshtastic/firmware/total)
[![CI](https://img.shields.io/github/actions/workflow/status/meshtastic/firmware/main_matrix.yml?branch=master&label=actions&logo=github&color=yellow)](https://github.com/meshtastic/firmware/actions/workflows/ci.yml)
[![CLA assistant](https://cla-assistant.io/readme/badge/meshtastic/firmware)](https://cla-assistant.io/meshtastic/firmware)
[![Fiscal Contributors](https://opencollective.com/meshtastic/tiers/badge.svg?label=Fiscal%20Contributors&color=deeppink)](https://opencollective.com/meshtastic/)
[![Vercel](https://img.shields.io/static/v1?label=Powered%20by&message=Vercel&style=flat&logo=vercel&color=000000)](https://vercel.com?utm_source=meshtastic&utm_campaign=oss)

<a href="https://trendshift.io/repositories/5524" target="_blank"><img src="https://trendshift.io/api/badge/repositories/5524" alt="meshtastic%2Ffirmware | Trendshift" style="width: 250px; height: 55px;" width="250" height="55"/></a>

</div>

</div>

<div align="center">
	<a href="https://meshtastic.org">Website</a>
	-
	<a href="https://meshtastic.org/docs/">Documentation</a>
</div>

## Overview

**This is a fork of the official Meshtastic firmware** with custom UI implementation currently working on:
- **Heltec WiFi LoRa 32 V3**
- **Heltec WiFi LoRa 32 V4**
- **MeshTalk T9** (Custom PCB - MESHTASTIC Build-Off 2026 Entry)

This fork adds a custom keyboard-driven UI with T9 input, LoRa network visualization, and message management capabilities using a 4x4 matrix keypad and TFT display.

### Hardware Configuration

#### Heltec V3 Custom Pin Configuration

**TFT Display (ST7789 240x320):**
- CS: GPIO 10
- DC: GPIO 11  
- RST: GPIO 12
- Backlight: GPIO 13
- SPI: VSPI (MOSI: 35, SCK: 36)

**4x4 Matrix Keypad:**
- Rows: GPIO 14, 15, 16, 17
- Cols: GPIO 4, 5, 6, 7

**Power & Battery:**
- VEXT Enable: GPIO 18 (active low)
- Battery Pin: GPIO 1 (ADC)
- ADC Channel: ADC_CHANNEL_0

#### Heltec V4 Custom Pin Configuration

**TFT Display (ST7789 240x320):**
- CS: GPIO 10
- DC: GPIO 11
- RST: GPIO 12  
- Backlight: GPIO 13
- SPI: VSPI (MOSI: 11, SCK: 14)

**4x4 Matrix Keypad:**
- Rows: GPIO 4, 5, 6, 7
- Cols: GPIO 15, 16, 17, 18

**Power & Battery:**
- VEXT Enable: GPIO 36 (active high, always on - powers GC1109 LDO)
- Battery Pin: GPIO 1 (ADC)
- ADC Channel: ADC_CHANNEL_0

#### MeshTalk T9 Custom Pin Configuration (MESHTASTIC Build-Off 2026 🏆)

**Hardware Platform:**
- **MCU:** ESP32-S3-WROOM-1-N16R8 (16MB Flash, 8MB PSRAM)
- **LoRa Module:** Seeed Wio-SX1262
- **Display:** ST7789 TFT (240x320) via LovyanGFX
- **Input:** 4x4 Matrix Keypad
- **Form Factor:** Custom PCB design with integrated components

**LoRa Configuration (Wio-SX1262):**
- MISO: GPIO 11
- MOSI: GPIO 12
- SCK: GPIO 13
- CS: GPIO 21
- Reset: GPIO 14
- DIO1: GPIO 9
- BUSY: GPIO 10
- RF Switch: DIO2 controlled
- TCXO: 1.8V

**Power & Battery:**
- Battery ADC: GPIO 8 (ADC_CHANNEL_7)
- ADC Control: GPIO 47 (enable/disable to save power)
- ADC Multiplier: 4.9 (390kΩ/10kΩ voltage divider)
- ADC Attenuation: 2.5dB
- Boot Button: GPIO 0 (hardware pull-up)

**Flash & Memory:**
- 16MB Flash (QIO mode, 80MHz)
- 8MB Octal SPI PSRAM (QIO OPI mode)
- Custom 16MB partition scheme

**Project Links:**
- 📁 Hardware Design: `meshtastic-build-off-2026/Hardware/meshtalk_t9/`
- 🎬 Demo Video: [YouTube](https://www.youtube.com/watch?v=elmYMMCQKS4)
- 📸 Photos & Schematics: `meshtastic-build-off-2026/Demo/`


### Features

- Custom T9 text input system for efficient messaging
- LoRa network node visualization with signal strength indicators
- Message management (send/receive/view history)
- Direct messaging and channel messaging support
- Favorites management
- Battery monitoring and power management
- Bluetooth disabled for reduced power consumption

## MESHTASTIC Build-Off 2026 Status 🏆

This project is my entry for the **MESHTASTIC Build-Off 2026** competition featuring the **MeshTalk T9** - a custom hardware device with integrated display and keypad.

### Phase 1: Custom PCB Design ✅ **COMPLETE**
- ✅ Designed custom PCB using **ESP32-S3-WROOM-1-N16R8** (16MB Flash, 8MB PSRAM)
- ✅ Integrated **Seeed Wio-SX1262** LoRa module
- ✅ Optimized board layout with RF considerations
- ✅ Integrated keypad matrix and display connections on-board
- ✅ Implemented power management with battery charging circuitry
- ✅ PCB manufactured and assembled successfully

### Phase 2: Firmware Adaptation ✅ **COMPLETE**
- ✅ Ported custom UI module to MeshTalk T9 hardware
- ✅ Updated pin configurations for custom PCB (`variants/esp32s3/meshtalk_t9/`)
- ✅ Configured SX1262 LoRa radio with proper RF switching
- ✅ Implemented battery monitoring with ADC control for power savings
- ✅ Created custom variant configuration with 16MB partition scheme
- ✅ Device boots and runs Meshtastic firmware successfully

### Phase 3: Testing & Validation ✅ **COMPLETE**
- ✅ Extensive testing similar to Heltec V3/V4 validation
- ✅ Range testing and mesh network performance benchmarks
- ✅ Power consumption analysis and optimization
- ✅ UI/UX testing with physical hardware
- ✅ Field testing in real-world scenarios
- ✅ Documentation and competition submission preparation

## Building the Firmware

### For Heltec V3 Custom
```bash
platformio run -e heltec-v3-custom
platformio run -e heltec-v3-custom -t upload
```

### For Heltec V4 Custom
```bash
platformio run -e heltec-v4-custom
platformio run -e heltec-v4-custom -t upload
```

### For MeshTalk T9 (Build-Off 2026 Entry)
```bash
# Build firmware
platformio run -e meshtalk-t9

# Upload to device
platformio run -e meshtalk-t9 -t upload --upload-port /dev/cu.usbmodem101

# Erase flash and upload (for first-time flash or troubleshooting)
platformio run -e meshtalk-t9 -t erase_upload

# Monitor serial output
platformio device monitor -p /dev/cu.usbmodem101
```

### Get Started

- 🔧 **[Original Meshtastic Firmware](https://github.com/meshtastic/firmware)** – Upstream repository
- 📖 **[Building Instructions](https://meshtastic.org/docs/development/firmware/build)** - Learn how to compile the firmware from source.
- ⚡ **[Flashing Instructions](https://meshtastic.org/docs/getting-started/flashing-firmware/)** - Install or update the firmware on your device.

Join the Meshtastic community and help improve the project! 🚀

## Stats

![Alt](https://repobeats.axiom.co/api/embed/8025e56c482ec63541593cc5bd322c19d5c0bdcf.svg "Repobeats analytics image")
