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

#include "BaseMenuScreen.h"

/**
 * Main Menu Screen
 * Implements the specific main menu items and navigation logic
 */
class MainMenuScreen : public BaseMenuScreen {
public:
    MainMenuScreen();
    virtual ~MainMenuScreen();
    
protected:
    virtual void onMenuItemSelected(int id) override;

private:
    // Menu item IDs
    static const int ID_NODES = 1;
    static const int ID_MESSAGES = 2;
    static const int ID_SNAKE = 3;
};
