#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#include <stdint.h>

// CHANGED: 0x2886/0x0059 is Seeed Studio's VID/PID for the XIAO
// 0x303a/0x1001 is Espressif's standard VID/PID for custom ESP32-S3 builds
#define USB_VID 0x303a
#define USB_PID 0x1001

// CHANGED: XIAO uses 47/48 for its I2C screen connector
// No I2C devices on MeshTalk T9 — mapped to unconnected GPIOs
// to prevent any library calling Wire.begin() from clashing with TFT_BL (GPIO4)
static const uint8_t SDA = 35;
static const uint8_t SCL = 36;

// CHANGED: XIAO LoRa SPI is 8/7/9/41 (B2B connector pins)
// MeshTalk T9 LoRa SPI is on different GPIOs
static const uint8_t MISO = 11;   // SPI_MISO_LORA
static const uint8_t SCK  = 13;   // SPI_SCK_LORA
static const uint8_t MOSI = 12;   // SPI_MOSI_LORA
static const uint8_t SS   = 21;   // LORA_CS

#endif /* Pins_Arduino_h */