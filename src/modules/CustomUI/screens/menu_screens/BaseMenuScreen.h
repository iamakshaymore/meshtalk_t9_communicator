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

#include "../list_screens/BaseListScreen.h"
#include <vector>

// Define generic menu item structure
struct MenuItem {
    String title;
    String navKey; // e.g. "1", "2"
    int id; // For switch case handling
};

/**
 * Abstract Base Menu Screen
 * Extends BaseListScreen to provide common menu functionality
 * - Rendering of standard menu items (badge, title, arrow)
 * - Navigation handling
 * - Back navigation logic
 */
class BaseMenuScreen : public BaseListScreen {
public:
    BaseMenuScreen(const String& screenName, int itemHeight = 30);
    virtual ~BaseMenuScreen();
    
    // BaseListScreen overrides
    virtual void drawItem(lgfx::LGFX_Device& tft, int index, int y, bool isSelected) override;
    virtual int getItemCount() override;
    virtual void onItemSelected(int index) override;
    
    // Input handling overrides
    virtual bool handleKeyPress(char key) override;

protected:
    // Methods for derived classes to manage items
    void addMenuItem(const String& title, const String& navKey, int id);
    void clearMenuItems();
    
    // Abstract method for handling menu selection
    // Derived classes implement this to handle specific actions
    virtual void onMenuItemSelected(int id) = 0;

    std::vector<MenuItem> menuItems;

    // Define colors if not already available
    #ifndef COLOR_GREEN
    static const uint16_t COLOR_GREEN = 0x07E0;
    #endif
    #ifndef COLOR_WHITE
    static const uint16_t COLOR_WHITE = 0xFFFF;
    #endif
};
