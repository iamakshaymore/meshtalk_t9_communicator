/*
 * MeshTalk T9 — Meshtastic UI variant
 * MCU  : ESP32-S3-WROOM-1-N16R8
 * LoRa : Seeed Wio-SX1262
 * Display: ST7789 SPI 320×240 (landscape) via Meshtastic built-in screen stack
 */

// ── Screen orientation ────────────────────────────────────────────────────────
// Panel needs the raw default MADCTL (MV|MY). DISPLAY_FLIP_SCREEN seeds
// config.display.flip_screen=true on first boot (NodeDB.cpp:1102), which
// makes Screen.cpp skip its corrective flipScreenVertically() call (MV|MX) —
// that call, and mirrorScreen() (MV|MX|MY), were both confirmed wrong on hardware.
#define DISPLAY_FLIP_SCREEN


#define USE_ST7789
#define HAS_SPI_TFT         1

#define ST7789_NSS          7     // CS
#define ST7789_RS           6     // DC (Data/Command)
#define ST7789_SDA          16    // MOSI (DIN)
#define ST7789_SCK          15    // CLK
#define ST7789_RESET        5     // RST
#define ST7789_MISO         -1    // not connected
#define ST7789_BUSY         -1

// Backlight: PWM via analogWrite — no separate power rail on T9
#define VTFT_LEDA           4
#define TFT_BACKLIGHT_ON    HIGH

#define ST7789_SPI_HOST     SPI2_HOST
#define SPI_FREQUENCY       40000000
#define SPI_READ_FREQUENCY  16000000

// Landscape 320×240
#define TFT_WIDTH           320
#define TFT_HEIGHT          240
#define TFT_OFFSET_X        0
#define TFT_OFFSET_Y        0

#define BRIGHTNESS_DEFAULT  150

// ── Button ────────────────────────────────────────────────────────────────────
// Hardware pull-up R11 (10kΩ) on PCB
#define BUTTON_PIN          0

// ── 4×4 T9 Matrix Keypad ────────────────────────────────────────────────────────
// Feeds Meshtastic's generic InputBroker matrix-keyboard driver (src/input/kbMatrixBase.cpp),
// type 2: 0-9 multi-tap cycle letters like a classic feature phone, A-D are
// Up/Down/Select/Back, */# are Left/Right (this grid has no dedicated arrow keys).
// Same pin wiring and physical key grid as the CustomUI-based meshtalk_t9 variant
// (src/modules/CustomUI/init/InitKeypad.cpp).
#define INPUTBROKER_MATRIX_TYPE 2
#define KEYS_ROWS            {41, 40, 39, 38}
#define KEYS_COLS            {42, 2, 1, 48}

// ── Battery ADC ───────────────────────────────────────────────────────────────
// Divider R5(390kΩ) / R10(10kΩ) on GPIO8; drive ADC_CTRL HIGH before sampling
#define BATTERY_PIN                   8
#define ADC_CHANNEL                   ADC_CHANNEL_7
#define BATTERY_SENSE_RESOLUTION_BITS 12
#define ADC_MULTIPLIER                4.9
#define ADC_ATTENUATION               ADC_ATTEN_DB_2_5
#define ADC_CTRL                      47
#define ADC_CTRL_ENABLED              HIGH

// ── LoRa / Wio-SX1262 ────────────────────────────────────────────────────────
#define USE_SX1262

#define LORA_MISO   11
#define LORA_SCK    13
#define LORA_MOSI   12
#define LORA_CS     21

#define LORA_RESET  14
#define LORA_DIO1   9

#define SX126X_CS           LORA_CS
#define SX126X_DIO1         LORA_DIO1
#define SX126X_BUSY         10
#define SX126X_RESET        LORA_RESET

#define SX126X_DIO2_AS_RF_SWITCH
#define SX126X_RXEN         RADIOLIB_NC
#define SX126X_TXEN         RADIOLIB_NC
#define SX126X_DIO3_TCXO_VOLTAGE 1.8
