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

#include "BaseMenuScreen.h"
#include "modules/CustomUI/CustomUIModule.h"

// Define standard colors if not already available
#ifndef COLOR_SELECTION
#define COLOR_SELECTION 0x4208 // Dim green, consistent with BaseListScreen
#endif
#ifndef COLOR_BLACK
#define COLOR_BLACK 0x0000
#endif

// Logging macro
#ifndef LOG_INFO
#define LOG_INFO(format, ...) Serial.printf("[INFO] " format "\n", ##__VA_ARGS__)
#endif

BaseMenuScreen::BaseMenuScreen(const String& screenName, int itemHeight) 
    : BaseListScreen(screenName, itemHeight) {
    LOG_INFO("BaseMenuScreen '%s' created", screenName.c_str());
}

BaseMenuScreen::~BaseMenuScreen() {
    LOG_INFO("BaseMenuScreen '%s' destroyed", name.c_str());
}

void BaseMenuScreen::addMenuItem(const String& title, const String& navKey, int id) {
    menuItems.push_back({title, navKey, id});
}

void BaseMenuScreen::clearMenuItems() {
    menuItems.clear();
}

int BaseMenuScreen::getItemCount() {
    return menuItems.size();
}

void BaseMenuScreen::drawItem(lgfx::LGFX_Device& tft, int index, int y, bool isSelected) {
    if (index < 0 || index >= (int)menuItems.size()) return;
    
    const MenuItem& item = menuItems[index];
    
    // Determine colors
    uint16_t bgColor = isSelected ? COLOR_SELECTION : COLOR_BLACK;
    uint16_t textColor = isSelected ? COLOR_WHITE : COLOR_GREEN;
    uint16_t borderColor = 0x4208; // Dark Grid/Grey
    
    // Draw background
    tft.fillRect(0, y, getContentWidth(), itemHeight, bgColor);
    
    // Draw separator
    tft.drawFastHLine(0, y + itemHeight - 1, getContentWidth(), borderColor);
    
    // Draw Key Badge
    int badgeSize = 20;
    int badgeX = 5;
    int badgeY = y + (itemHeight - badgeSize) / 2;
    
    // Badge background
    if (isSelected) {
        tft.fillRoundRect(badgeX, badgeY, badgeSize, badgeSize, 4, COLOR_BLACK);
    } else {
        tft.fillRoundRect(badgeX, badgeY, badgeSize, badgeSize, 4, 0x3333);
    }
    
    // Badge Text (Always Green)
    tft.setTextColor(COLOR_GREEN);
    
    tft.setTextSize(1);
    // Center single character
    int charWidth = 6; // Approx width for standard font size 1
    int charX = badgeX + (badgeSize - charWidth) / 2 + 1;
    tft.setCursor(charX, badgeY + 6);
    tft.print(item.navKey);
    
    // Draw Title
    tft.setTextColor(textColor);
    tft.setTextSize(1);
    
    // Using LGFX font metrics if possible, otherwise simple offset
    int textY = y + (itemHeight / 2) - 4;
    tft.setCursor(badgeX + badgeSize + 10, textY);
    tft.print(item.title);
    
    // Draw arrow indicator on right if selected
    if (isSelected) {
        tft.drawString(">", getContentWidth() - 15, textY);
    }
}

void BaseMenuScreen::onItemSelected(int index) {
    if (index < 0 || index >= (int)menuItems.size()) return;
    
    // Retrieve ID and call abstract method
    onMenuItemSelected(menuItems[index].id);
}

bool BaseMenuScreen::handleKeyPress(char key) {
    if (!customUIModule) return false;

    // Default back navigation behavior
    switch (key) {
        case 'A':
        case 'a':
            customUIModule->getScreenManager()->navigateBack();
            return true;
        default:
            return BaseListScreen::handleKeyPress(key);
    }
}
