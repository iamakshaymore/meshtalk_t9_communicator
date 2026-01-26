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

#include "MessagesMenuScreen.h"
#include "modules/CustomUI/CustomUIModule.h"
#include "modules/CustomUI/screens/list_screens/MessageListScreen.h"
#include "../utils/LoRaHelper.h" 
#include "configuration.h"

MessagesMenuScreen::MessagesMenuScreen() : BaseMenuScreen("Messages", 30) {
    // Initial build
    rebuildMenuItems();
}

MessagesMenuScreen::~MessagesMenuScreen() {
}

void MessagesMenuScreen::onEnter(const NavigationContext& ctx) {
    // Rebuild every time we enter to ensure latest active channels are shown
    rebuildMenuItems();
    BaseMenuScreen::onEnter(ctx);
}

void MessagesMenuScreen::rebuildMenuItems() {
    clearMenuItems(); 
    
    int navKey = 1;
    
    // 1. Direct Messages
    addMenuItem("Direct Messages", String(navKey++), ID_DIRECT_MESSAGES);
    
    // 2. configured Channels from LoRaHelper
    std::vector<ChannelHelperInfo> channels = LoRaHelper::getChannelList();
    
    for (const auto& ch : channels) {
         String name = String(ch.name);
         if (name.length() == 0) {
             name = "Channel " + String(ch.index);
         }
         
         String keyStr = (navKey <= 9) ? String(navKey++) : ""; 
         addMenuItem(name, keyStr, ID_CHANNEL_BASE + ch.index);
    }
    
    // Update navigation hints
    std::vector<NavHint> hints;
    hints.push_back(NavHint('1', "Select"));
    hints.push_back(NavHint('A', "Back"));
    setNavigationHints(hints);
}

void MessagesMenuScreen::onMenuItemSelected(int id) {
    if (!customUIModule) return;
    
    // Create and configure filter based on selection
    MessageFilter filter;
    filter.reset();
    filter.active = true;
    
    if (id == ID_DIRECT_MESSAGES) {
        filter.onlyDMs = true;
        LOG_INFO("💬 Menu: Selected DMs");
    } else if (id >= ID_CHANNEL_BASE) {
         int chIndex = id - ID_CHANNEL_BASE;
         filter.channelIndex = chIndex;
         LOG_INFO("💬 Menu: Selected Channel %d", chIndex);
    } else {
        LOG_WARN("💬 Menu: Unknown selection");
        return;
    }

    // Reuse the existing singleton MessageListScreen
    MessageListScreen* screen = customUIModule->getMessageListScreen();
    if (screen) {
        screen->setFilter(filter);
        customUIModule->getScreenManager()->navigateTo(screen);
    } else {
        LOG_ERROR("❌ MessageListScreen not initialized!");
    }
}
