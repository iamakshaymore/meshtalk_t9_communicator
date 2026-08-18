/*
 * MeshTalk T9 — Meshtastic variant
 * MCU  : ESP32-S3-WROOM-1-N16R8
 * LoRa : Seeed Wio-SX1262
 */

// ── LED ───────────────────────────────────────────────────────────────────────
// #define HAS_LED              1       // Tells Meshtastic to initialize LED subsystem

// #define LED_PIN              17      // Physical GPIO pin
// #define LED_STATE_ON         1       // 1 = Active HIGH (since HIGH turned your LED on)

// // Low-level aliases (Required for ESP32 BSP compatibility)
// #ifndef PIN_LED
//   #define PIN_LED            LED_PIN
// #endif
// #define LED_HEARTBEAT LED_PIN


// pinMode(17, OUTPUT);
// Tell power module to manage this LED state
// #define LED_POWER            LED_PIN
// #define LED_POWER_ON         LED_STATE_ON
// ── Boot button ───────────────────────────────────────────────────────────────
// Hardware pull-up R11 (10kΩ) on PCB — no BUTTON_NEED_PULLUP required
#define BUTTON_PIN 0

// ── Battery ADC ───────────────────────────────────────────────────────────────
// Divider R5(390kΩ) / R10(10kΩ) on GPIO8
// Drive ADC_CTRL HIGH before sampling, LOW after to kill quiescent current
#define BATTERY_PIN                   8
#define ADC_CHANNEL                   ADC_CHANNEL_7
#define BATTERY_SENSE_RESOLUTION_BITS 12
#define ADC_MULTIPLIER                4.9
#define ADC_ATTENUATION               ADC_ATTEN_DB_2_5
#define ADC_CTRL                      47
#define ADC_CTRL_ENABLED              HIGH
// ── Screen ────────────────────────────────────────────────────────────────────
// Meshtastic built-in screen disabled — handled by CustomUIModule
#define HAS_SCREEN 0

// ── LoRa / Wio-SX1262 ────────────────────────────────────────────────────────
#define USE_SX1262

#define LORA_MISO  11
#define LORA_SCK   13
#define LORA_MOSI  12
#define LORA_CS    21

#define LORA_RESET 14
#define LORA_DIO1   9

#ifdef USE_SX1262
#define SX126X_CS    LORA_CS
#define SX126X_DIO1  LORA_DIO1
#define SX126X_BUSY  10
#define SX126X_RESET LORA_RESET

#define SX126X_DIO2_AS_RF_SWITCH
#define SX126X_RXEN RADIOLIB_NC
#define SX126X_TXEN RADIOLIB_NC
#define SX126X_DIO3_TCXO_VOLTAGE 1.8

#define MESHTALK_T9 1

#endif

// ── T9 Matrix Keypad ─────────────────────────────────────────────────────────
// 4×4 matrix keypad wake source configuration
// Row pins driven LOW during sleep, column pins configured as GPIO wake sources
#define KEYPAD_ROW_COUNT 4
#define KEYPAD_COL_COUNT 4
#define KEYPAD_ROW_PINS {41, 40, 39, 38}
#define KEYPAD_COL_PINS {42, 2, 1, 48}
