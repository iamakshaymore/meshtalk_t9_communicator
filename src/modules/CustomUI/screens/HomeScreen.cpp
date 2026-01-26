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

#include "HomeScreen.h"
#include "modules/CustomUI/CustomUIModule.h"
#include "modules/CustomUI/screens/menu_screens/MainMenuScreen.h"
#include "modules/CustomUI/screens/list_screens/NodesListScreen.h"
#include "modules/CustomUI/screens/list_screens/MessageListScreen.h"
#include "utils/BatteryHelper.h"
#include "utils/LoRaHelper.h"
#include "utils/DeviceMetricsHelper.h"
#include "configuration.h"

HomeScreen::HomeScreen() : BaseScreen("Home"), lastUpdate(0), lastNodeCount(-1), statusChanged(true) {
    // Set navigation hints for home screen
    std::vector<NavHint> hints;
    hints.push_back(NavHint('1', "Menu"));
    hints.push_back(NavHint('A', "Nodes"));
    setNavigationHints(hints);
    
    // Initialize device metrics
    DeviceMetricsHelper::init();
    
    LOG_INFO("HomeScreen created");
}

HomeScreen::~HomeScreen() {
    LOG_INFO("HomeScreen destroyed");
}

void HomeScreen::onEnter(const NavigationContext& ctx) {
    LOG_INFO("Entering Home screen");
    statusChanged = true;
    lastUpdate = 0;
}

void HomeScreen::onExit() {
    LOG_INFO("Exiting Home screen");
}

void HomeScreen::onDraw(lgfx::LGFX_Device& tft) {
    // Update status if needed
    unsigned long now = millis();
    if (now - lastUpdate > 5000) { // Update every 5 seconds
        updateStatus();
        lastUpdate = now;
    }
    
    // Clear content area to pure black
    tft.fillRect(0, getContentY(), getContentWidth(), getContentHeight(), 0x0000);
    
    // Draw border around content area
    tft.drawRect(5, getContentY() + 5, getContentWidth() - 10, getContentHeight() - 10, 0xFFE0);
    
    // Draw content in two-column layout
    drawDeviceStatus(tft);
    drawNetworkInfo(tft);
    drawSystemMetrics(tft);
    drawLastActivity(tft);
}

bool HomeScreen::handleKeyPress(char key) {
    switch (key) {
        case '1':
            // Menu Screen
            if (customUIModule) {
                customUIModule->getScreenManager()->navigateTo(customUIModule->getMenuScreen());
            }
            return true;
            
        case 'A':
        case 'a':
            // Nodes List (Quick Access)
            if (customUIModule) {
                customUIModule->getScreenManager()->navigateTo(customUIModule->getNodesListScreen());
            }
            return true;
            
        default:
            return false; // Key not handled
    }
}

void HomeScreen::updateStatus() {
    // Check if any status has changed
    int currentNodeCount = LoRaHelper::getNodeCount();
    
    if (currentNodeCount != lastNodeCount || 
        BatteryHelper::hasChanged() || 
        LoRaHelper::hasChanged() ||
        DeviceMetricsHelper::hasChanged()) {
        
        statusChanged = true;
        lastNodeCount = currentNodeCount;
    }
}

void HomeScreen::drawDeviceStatus(lgfx::LGFX_Device& tft) {
    int y = getContentY() + 15;
    
    // Device status section - Left column
    tft.setTextColor(0xFFE0, 0x0000); // Bright yellow on black
    tft.setTextSize(1);
    tft.setCursor(LEFT_COLUMN_X, y);
    tft.print("DEVICE:");
    y += LINE_HEIGHT;
    
    // Device name (truncated if needed)
    tft.setTextColor(0x07E0, 0x0000); // Bright green on black
    tft.setCursor(LEFT_COLUMN_X + 5, y);
    String deviceName = LoRaHelper::getDeviceLongName();
    if (deviceName.length() > 18) {
        deviceName = deviceName.substring(0, 15) + "...";
    }
    tft.print(deviceName);
    y += LINE_HEIGHT;
    
    // Battery info with color coding
    tft.setCursor(LEFT_COLUMN_X + 5, y);
    String batteryInfo = "BAT: " + BatteryHelper::getBatteryString();
    int batteryPercent = BatteryHelper::getBatteryPercent();
    
    if (batteryPercent > 50) {
        tft.setTextColor(0x07E0, 0x0000); // Green
    } else if (batteryPercent > 20) {
        tft.setTextColor(0xFFE0, 0x0000); // Yellow
    } else {
        tft.setTextColor(0xF800, 0x0000); // Red
    }
    tft.print(batteryInfo);
}

void HomeScreen::drawNetworkInfo(lgfx::LGFX_Device& tft) {
    int y = getContentY() + 75;
    
    // Network section - Left column
    tft.setTextColor(0xFFE0, 0x0000); // Bright yellow on black
    tft.setTextSize(1);
    tft.setCursor(LEFT_COLUMN_X, y);
    tft.print("NETWORK:");
    y += LINE_HEIGHT;
    
    // Node count
    tft.setTextColor(0x07E0, 0x0000); // Bright green on black
    tft.setCursor(LEFT_COLUMN_X + 5, y);
    String nodeInfo = "Nodes: " + String(LoRaHelper::getNodeCount());
    tft.print(nodeInfo);
    y += LINE_HEIGHT;
    
    // LoRa status
    tft.setCursor(LEFT_COLUMN_X + 5, y);
    if (LoRaHelper::getNodeCount() > 0) {
        tft.setTextColor(0x07E0, 0x0000); // Green
        tft.print("LoRa: Connected");
    } else {
        tft.setTextColor(0xFFE0, 0x0000); // Yellow
        tft.print("LoRa: Searching");
    }
}

void HomeScreen::drawSystemMetrics(lgfx::LGFX_Device& tft) {
    int y = getContentY() + 15;
    
    // System metrics section - Right column
    tft.setTextColor(0xFFE0, 0x0000); // Bright yellow on black
    tft.setTextSize(1);
    tft.setCursor(RIGHT_COLUMN_X, y);
    tft.print("SYSTEM:");
    y += LINE_HEIGHT;
    
    // SRAM utilization
    tft.setCursor(RIGHT_COLUMN_X + 5, y);
    int sramPercent = DeviceMetricsHelper::getSramUtilization();
    String sramInfo = "SRAM >> " + String(sramPercent) + "%";
    
    // Color code SRAM based on utilization
    if (sramPercent < 70) {
        tft.setTextColor(0x07E0, 0x0000); // Green - Good
    } else if (sramPercent < 85) {
        tft.setTextColor(0xFFE0, 0x0000); // Yellow - Warning
    } else {
        tft.setTextColor(0xF800, 0x0000); // Red - Critical
    }
    tft.print(sramInfo);
    y += LINE_HEIGHT;
    
    // PSRAM utilization (if available)
    if (DeviceMetricsHelper::hasPsram()) {
        tft.setCursor(RIGHT_COLUMN_X + 5, y);
        int psramPercent = DeviceMetricsHelper::getPsramUtilization();
        String psramInfo = "PSRAM >> " + String(psramPercent) + "%";
        
        // Color code PSRAM based on utilization
        if (psramPercent < 70) {
            tft.setTextColor(0x07E0, 0x0000); // Green - Good
        } else if (psramPercent < 85) {
            tft.setTextColor(0xFFE0, 0x0000); // Yellow - Warning
        } else {
            tft.setTextColor(0xF800, 0x0000); // Red - Critical
        }
        tft.print(psramInfo);
        y += LINE_HEIGHT;
    }
    
    // Free SRAM details
    tft.setTextColor(0x4208, 0x0000); // Dim green
    tft.setCursor(RIGHT_COLUMN_X + 5, y);
    size_t freeSram = DeviceMetricsHelper::getSramFree();
    String freeInfo = "SRAM Free: ";
    if (freeSram >= 1024) {
        freeInfo += String(freeSram / 1024) + "KB";
    } else {
        freeInfo += String(freeSram) + "B";
    }
    tft.print(freeInfo);
    y += LINE_HEIGHT;
    
    // Free PSRAM details (if available)
    if (DeviceMetricsHelper::hasPsram()) {
        tft.setTextColor(0x4208, 0x0000); // Dim green
        tft.setCursor(RIGHT_COLUMN_X + 5, y);
        size_t freePsram = DeviceMetricsHelper::getPsramFree();
        
        // Debug logging
        LOG_DEBUG("PSRAM Free in UI: %zu bytes (%.1fMB)", freePsram, (float)freePsram / (1024.0 * 1024.0));
        
        String psramFreeInfo = "PSRAM Free: ";
        if (freePsram >= 1024*1024) {
            float mbValue = (float)freePsram / (1024.0 * 1024.0);
            psramFreeInfo += String(mbValue, 1) + "MB";  // Show 1 decimal place
        } else if (freePsram >= 1024) {
            psramFreeInfo += String(freePsram / 1024) + "KB";
        } else {
            psramFreeInfo += String(freePsram) + "B";
        }
        tft.print(psramFreeInfo);
        y += LINE_HEIGHT;
    }
    
    // Draw border around system metrics
    int borderHeight = DeviceMetricsHelper::hasPsram() ? 98 : 65; // Smaller height when no PSRAM
    tft.drawRect(RIGHT_COLUMN_X - 3, getContentY() + 12, COLUMN_WIDTH - 10, borderHeight, 0xFFE0);
}

void HomeScreen::drawLastActivity(lgfx::LGFX_Device& tft) {
    int y = getContentY() + 135;
    
    // Activity section - Spans both columns at bottom
    tft.setTextColor(0xFFE0, 0x0000); // Bright yellow on black
    tft.setTextSize(1);
    tft.setCursor(LEFT_COLUMN_X, y);
    tft.print("UPTIME:");
    
    // Uptime - Center aligned
    tft.setTextColor(0x4208, 0x0000); // Dim green
    unsigned long uptimeSeconds = millis() / 1000;
    unsigned long hours = uptimeSeconds / 3600;
    unsigned long minutes = (uptimeSeconds % 3600) / 60;
    unsigned long seconds = uptimeSeconds % 60;
    
    String uptimeStr = String(hours) + "h " + String(minutes) + "m " + String(seconds) + "s";
    
    // Center the uptime string
    int textWidth = uptimeStr.length() * 6; // Approximate character width
    int centerX = (getContentWidth() - textWidth) / 2;
    tft.setCursor(centerX, y + LINE_HEIGHT);
    tft.print(uptimeStr);
}