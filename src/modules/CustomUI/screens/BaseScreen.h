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

#include <LovyanGFX.hpp>
#include <Arduino.h>
#include <vector>

// Navigation context structure
struct NavigationContext {
    bool isBack;        // TRUE if returning to this screen (back navigation)
    void* resultData;   // Optional data passed from previous screen

    NavigationContext(bool back = false, void* data = nullptr) 
        : isBack(back), resultData(data) {}
};

// Navigation hint structure
struct NavHint {
    char key;          // Key to press (e.g., '1', '2', '3')
    String label;      // Label to display (e.g., "Home", "Nodes")
    
    NavHint(char k, const String& l) : key(k), label(l) {}
};

/**
 * Base abstract screen class for CustomUI
 * Provides standard 3-section layout: header, content, footer
 * 
 * Layout:
 * ┌─────────────────────────────────────┐
 * │ Header: Device Name    Battery %    │ ← 30px height
 * ├─────────────────────────────────────┤
 * │                                     │
 * │            Content Area             │ ← 150px height
 * │                                     │
 * ├─────────────────────────────────────┤
 * │ Footer: [1]Home [2]Nodes [3]WiFi    │ ← 30px height
 * └─────────────────────────────────────┘
 */
class BaseScreen {
public:
    BaseScreen(const String& screenName);
    virtual ~BaseScreen();
    
    // Screen lifecycle
    virtual void onEnter(const NavigationContext& ctx = NavigationContext()) { (void)ctx; } // Default implementation
    virtual void onExit() {}      // Default empty implementation
    virtual void onDraw(lgfx::LGFX_Device& tft) = 0;  // Draw content area only
    
    // Input handling
    virtual bool handleKeyPress(char key) = 0;  // Return true if key was handled
    
    // Screen management
    void draw(lgfx::LGFX_Device& tft);  // Draw complete screen (header + content + footer)
    
    // Force just the content (onDraw) to run
    void forceRedraw() { contentNeedsRedraw = true; }
    
    // Force a full screen wipe and redraw
    void forceFullRedraw() { needsFullRedraw = true; headerNeedsUpdate = true; footerNeedsUpdate = true; contentNeedsRedraw = true; }
    
    virtual bool needsUpdate() const { return needsFullRedraw || headerNeedsUpdate || footerNeedsUpdate || contentNeedsRedraw; }
    
    // Navigation
    void setNavigationHints(const std::vector<NavHint>& hints);
    
    // Screen info
    const String& getName() const { return name; }
    
    // Layout constants
    static const int HEADER_HEIGHT = 30;
    static const int FOOTER_HEIGHT = 30;
    static const int CONTENT_Y = HEADER_HEIGHT;
    static const int CONTENT_HEIGHT = 240 - HEADER_HEIGHT - FOOTER_HEIGHT; // 180px
    static const int SCREEN_WIDTH = 320;
    static const int SCREEN_HEIGHT = 240;

protected:
    String name;
    bool needsFullRedraw;
    bool headerNeedsUpdate;
    bool footerNeedsUpdate;
    bool contentNeedsRedraw;
    std::vector<NavHint> navHints;
    
    // Layout helpers for derived classes
    int getContentY() const { return CONTENT_Y; }
    int getContentHeight() const { return CONTENT_HEIGHT; }
    int getContentWidth() const { return SCREEN_WIDTH; }

private:
    void drawHeader(lgfx::LGFX_Device& tft);
    void drawFooter(lgfx::LGFX_Device& tft);
    void updateHeader(lgfx::LGFX_Device& tft);
    
    // Header state tracking
    String lastDeviceName;
    String lastBatteryStatus;
};