/*
 * CustomUIModule - A T9 Matrix Keypad & Display Driver for Meshtastic
 * Copyright (C) 2026 Akshay Pramod More
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#pragma once

#include "InitBase.h"
#include <LovyanGFX.hpp>

// Forward declaration
class LGFX;

/**
 * Display initializer using LovyanGFX for optimal ESP32-S3 + ST7789 performance
 * Only handles initialization - CustomUIModule handles all drawing logic
 * Features:
 * - 80MHz SPI with automatic DMA
 * - PSRAM support
 * - High-performance rendering (40-60 FPS)
 * - Memory efficient (~150-220KB free)
 */
class InitDisplay : public InitBase {
public:
    InitDisplay();
    virtual ~InitDisplay();
    
    // InitBase interface
    virtual bool init() override;
    void update() { /* No update needed - initialization only */ }
    virtual void cleanup() override;
    virtual const char* getName() const override { return "Display"; }
    virtual bool isReady() const override { return (tft != nullptr); }
    
    // Display access for CustomUIModule
    lgfx::LGFX_Device* getDisplay() { return tft; }

private:
    lgfx::LGFX_Device* tft;
    bool initialized;
};