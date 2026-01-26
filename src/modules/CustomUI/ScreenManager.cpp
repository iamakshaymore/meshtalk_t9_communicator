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

#include "ScreenManager.h"
#include "screens/T9InputScreen.h"

#ifdef ESP32
#include <esp_heap_caps.h>
#endif

ScreenManager::ScreenManager() : tft(nullptr) {
    screenStack.reserve(5); // Pre-allocate some space
}

ScreenManager::~ScreenManager() {
    // Note: ScreenManager does not own the screens, so it doesn't delete them.
    // It only manages the navigation stack.
    screenStack.clear();
}

void ScreenManager::init(lgfx::LGFX_Device* display) {
    tft = display;
}

void ScreenManager::navigateTo(BaseScreen* screen, void* data) {
    if (!screen) return;
    
    // Don't navigate to the same screen if it's already top
    if (!screenStack.empty() && screenStack.back() == screen) {
        return;
    }

    // Handle exit of current screen
    if (!screenStack.empty()) {
        screenStack.back()->onExit();
    }
    
    // Display cleanup before switch
    if (tft) {
        tft->waitDisplay();
    }
    
#ifdef ESP32
    // Optional debug check
    // heap_caps_check_integrity_all(true);
#endif

    // Push new screen and enter
    screenStack.push_back(screen);
    screen->onEnter(NavigationContext(false, data));
    
    // Force full redraw
    if (tft) {
        screen->forceFullRedraw();
        screen->draw(*tft);
    }
}

void ScreenManager::navigateToT9(T9InputScreen* t9Screen, std::function<void(const String&)> onConfirm) {
    if (!t9Screen) return;
    
    // Configure the T9 screen
    t9Screen->clearInput();
    t9Screen->setConfirmCallback(onConfirm);
    
    // Perform navigation
    navigateTo(t9Screen);
}

void ScreenManager::navigateBack(void* resultData) {
    // Need at least 2 screens to go back (current + previous)
    if (screenStack.size() < 2) return;

    // Exit current screen
    BaseScreen* current = screenStack.back();
    current->onExit();
    
    // Display cleanup
    if (tft) {
        tft->waitDisplay();
    }

    // Remove current screen
    screenStack.pop_back();

    // Enter previous screen
    if (!screenStack.empty()) {
        BaseScreen* prev = screenStack.back();
        prev->onEnter(NavigationContext(true, resultData));
        
        // Force full redraw
        if (tft) {
            prev->forceFullRedraw();
            prev->draw(*tft);
        }
    }
}

void ScreenManager::showPopup(const String& message, int durationMs) {
    if (!tft) return;
    
    // Calculate centered position
    int w = tft->width();
    int h = tft->height();
    int boxW = 200;
    int boxH = 60;
    int boxX = (w - boxW) / 2;
    int boxY = (h - boxH) / 2;
    
    // Draw shadow
    tft->fillRoundRect(boxX + 4, boxY + 4, boxW, boxH, 8, 0x0000); // Black shadow
    
    // Draw box
    uint16_t COLOR_POPUP_BG = 0x4208; // Dark green
    uint16_t COLOR_WHITE = 0xFFFF;
    tft->fillRoundRect(boxX, boxY, boxW, boxH, 8, COLOR_POPUP_BG);
    tft->drawRoundRect(boxX, boxY, boxW, boxH, 8, COLOR_WHITE);
    
    // Draw text
    tft->setTextColor(COLOR_WHITE, COLOR_POPUP_BG);
    tft->setTextSize(2);
    tft->setTextDatum(textdatum_t::middle_center);
    tft->drawString(message, boxX + boxW/2, boxY + boxH/2);
    tft->setTextDatum(textdatum_t::top_left); // Reset
    
    // Wait
    if (durationMs > 0) {
        delay(durationMs);
    }
}

BaseScreen* ScreenManager::getCurrentScreen() {
    if (screenStack.empty()) return nullptr;
    return screenStack.back();
}
