#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#include <stdint.h>

// Espressif's standard VID/PID for custom ESP32-S3 builds
#define USB_VID 0x303a
#define USB_PID 0x1001

// No I2C devices on MeshTalk T9 — dummy values to prevent Wire.begin()
// from conflicting with TFT_BL on GPIO4
static const uint8_t SDA =  3;
static const uint8_t SCL = 46;

// Default SPI maps to LoRa radio bus
static const uint8_t MISO = 11;  // SPI_MISO_LORA
static const uint8_t SCK  = 13;  // SPI_SCK_LORA
static const uint8_t MOSI = 12;  // SPI_MOSI_LORA
static const uint8_t SS   = 21;  // LORA_CS

#endif /* Pins_Arduino_h */
