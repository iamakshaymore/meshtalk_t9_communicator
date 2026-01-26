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
 * Messages Menu Screen
 * Sub-menu for selecting which message category to view
 */
class MessagesMenuScreen : public BaseMenuScreen {
public:
    MessagesMenuScreen();
    virtual ~MessagesMenuScreen();
    
protected:
    virtual void onMenuItemSelected(int id) override;
    virtual void onEnter(const NavigationContext& ctx) override;

private:
    void rebuildMenuItems();
    std::vector<uint8_t> activeChannels;

    static const int ID_DIRECT_MESSAGES = 100;
    // Channel IDs will start from 200 + channelIndex
    static const int ID_CHANNEL_BASE = 200;
};

