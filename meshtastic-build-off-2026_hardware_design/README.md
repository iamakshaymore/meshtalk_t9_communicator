# MESHTASTIC Build-Off 2026 - Hardware Design

## Project Overview

This directory contains the custom hardware design for the **MESHTASTIC Build-Off 2026** competition entry. The goal is to create a custom PCB-based Meshtastic device with integrated display and keypad interface.

## Competition Entry

- **Event:** MESHTASTIC Build-Off 2026
- **Category:** Custom Hardware Design
- **Project Status:** Phase 1 - Design & Development

## Hardware Specifications

### Core Components

- **Microcontroller:** ESP32-S3
  - Dual-core processor
  - Built-in WiFi support
  - Low power consumption
  - Ample GPIO for peripherals

- **LoRa Module:** Wio-SX1262 Wireless Module
  - Sub-GHz LoRa transceiver
  - Long-range mesh networking capability
  - Optimized for low power operation

### Peripheral Integration

- **Display:** TFT ST7789 (240x320 pixels)
  - Custom UI with LoRa network visualization
  - Message management interface
  - Battery and signal indicators

- **Input:** 4x4 Matrix Keypad
  - T9 text input system
  - Navigation and control keys
  - Efficient one-handed operation

- **Power Management:**
  - LiPo battery support with charging circuit
  - Power monitoring and low-battery alerts
  - Optimized sleep modes

## Design Goals

1. **Integration:** Single PCB design combining all components
2. **Efficiency:** Low power consumption for extended battery life
3. **Usability:** Intuitive keypad-based interface
4. **Reliability:** Robust design suitable for outdoor/field use
5. **Compliance:** Meet MESHTASTIC Build-Off 2026 requirements

## Development Phases

### Phase 1: PCB Design ⚡ (Current)
- [ ] Schematic design
- [ ] Component selection and sourcing
- [ ] PCB layout and routing
- [ ] Power circuit design
- [ ] Antenna integration planning

### Phase 2: Firmware Adaptation 🔧
- [ ] Port custom UI to new hardware
- [ ] Update pin configurations
- [ ] Test and optimize power management
- [ ] Create custom variant configuration

### Phase 3: Testing & Validation ✅
- [ ] Prototype assembly and bring-up
- [ ] Hardware functionality testing
- [ ] Range and mesh network testing
- [ ] Power consumption analysis
- [ ] Field testing and optimization

## Based On

This design builds upon proven firmware developed and tested on:
- Heltec WiFi LoRa 32 V3
- Heltec WiFi LoRa 32 V4

The custom PCB will incorporate lessons learned and optimizations from these reference platforms.

## Rough Bill of Materials (BOM)

| Component | Part Number / Description | Quantity | Notes |
|-----------|--------------------------|----------|-------|
| **Microcontroller** | ESP32-S3-WROOM-1 | 1 | Main processor with WiFi |
| **LoRa Module** | Wio-SX1262 Wireless Module | 1 | Sub-GHz LoRa transceiver |
| **Display** | ST7789 TFT LCD 240x320 | 1 | 2.4" color display |
| **Keypad** | 4x4 Matrix Keypad | 1 | Membrane or mechanical |
| **Battery** | Li-Po 3.7V 2000mAh | 1 | Rechargeable battery |
| **Charging IC** | TP4056 or MCP73831 | 1 | Li-Po battery charger |
| **Voltage Regulator** | AMS1117-3.3 | 1 | 3.3V power supply |
| **USB-C Connector** | USB Type-C Female | 1 | Power and programming |
| **Antenna** | 868/915MHz LoRa Antenna | 1 | External or PCB trace |
| **Crystal** | 32.768kHz | 1 | RTC oscillator (optional) |
| **Resistors** | Various values | ~20 | Pull-ups, dividers |
| **Capacitors** | 0.1μF, 10μF, 100μF | ~15 | Decoupling and filtering |
| **LEDs** | Status LEDs | 2-3 | Power, charging, activity |
| **Buttons** | Tactile switches | 2 | Reset, boot/user button |
| **PCB** | Custom 2-layer or 4-layer | 1 | FR-4, ENIG finish |

**Estimated Total Cost:** ~$35-50 per unit (excluding assembly)

*Note: Part numbers and quantities are preliminary and subject to change during detailed design.*

## Contact & Updates

This is an active development project. Design files and documentation will be added as the project progresses through each phase.

---

**Competition Preparation Status:** In Progress  
**Last Updated:** June 2026
