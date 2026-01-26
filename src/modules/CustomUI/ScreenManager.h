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

#include "screens/BaseScreen.h"
#include <LovyanGFX.hpp>
#include <vector>
#include <functional>

class T9InputScreen; // Forward declaration

class ScreenManager {
public:
    ScreenManager();
    ~ScreenManager();

    // Initialize with display driver
    void init(lgfx::LGFX_Device* tft);

    // Navigate to a new screen, pushing it onto the stack
    void navigateTo(BaseScreen* screen, void* data = nullptr);
    
    // Navigate to T9 Input screen with a specific callback
    void navigateToT9(T9InputScreen* t9Screen, std::function<void(const String&)> onConfirm);

    // Navigate back to the previous screen
    void navigateBack(void* resultData = nullptr);

    /**
     * Show a modal popup message for a specified duration
     * @param message Text to display
     * @param durationMs Duration in milliseconds to show the popup (blocking)
     */
    void showPopup(const String& message, int durationMs = 1000);

    // Get the currently active screen
    BaseScreen* getCurrentScreen();

private:
    std::vector<BaseScreen*> screenStack;
    lgfx::LGFX_Device* tft;
};
