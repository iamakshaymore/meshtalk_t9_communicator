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

#include "BaseScreen.h"
#include <LovyanGFX.hpp>

/**
 * Home screen - main screen with device status and basic info
 * Shows:
 * - Device status
 * - Node count
 * - Network status
 * - Last activity
 */
class HomeScreen : public BaseScreen {
public:
    HomeScreen();
    virtual ~HomeScreen();
    
    // Screen lifecycle
    virtual void onEnter(const NavigationContext& ctx) override;
    virtual void onExit() override;
    virtual void onDraw(lgfx::LGFX_Device& tft) override;
    
    // Input handling
    virtual bool handleKeyPress(char key) override;
    
    // Update methods
    void updateStatus();

private:
    void drawDeviceStatus(lgfx::LGFX_Device& tft);
    void drawNetworkInfo(lgfx::LGFX_Device& tft);
    void drawSystemMetrics(lgfx::LGFX_Device& tft);
    void drawLastActivity(lgfx::LGFX_Device& tft);
    
    // Status tracking
    unsigned long lastUpdate;
    String lastStatus;
    int lastNodeCount;
    bool statusChanged;

    // Layout constants
    static const int LEFT_COLUMN_X = 10;
    static const int RIGHT_COLUMN_X = 170;
    static const int COLUMN_WIDTH = 140;
    static const int LINE_HEIGHT = 18;
};