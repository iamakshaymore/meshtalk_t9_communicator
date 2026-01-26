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

#include "BaseListScreen.h"
#include "../utils/LoRaHelper.h"
#include <vector>

struct MessageFilter {
    bool active;
    bool onlyOutgoing;
    bool onlyIncomingDMs;
    bool onlyDMs;
    int8_t channelIndex; // -1 for any

    MessageFilter() : active(false), onlyOutgoing(false), onlyIncomingDMs(false), onlyDMs(false), channelIndex(-1) {}
    
    void reset() {
        active = false;
        onlyOutgoing = false;
        onlyIncomingDMs = false;
        onlyDMs = false;
        channelIndex = -1;
    }
};

/**
 * Message List Screen - Shows recent mesh messages
 * Features:
 * - Scrollable list of recent messages  
 * - Color coding: Green for DMs, Red for channel messages
 * - Time since received display
 * - Horizontal scrolling for long messages when selected
 * - Navigation: [A] Back, [Left/Right] Scroll message
 */
class MessageListScreen : public BaseListScreen {
public:
    MessageListScreen(MessageFilter filter = MessageFilter());
    virtual ~MessageListScreen();

    // BaseListScreen interface  
    virtual void onEnter(const NavigationContext& ctx) override;
    virtual void onExit() override;
    virtual bool handleKeyPress(char key) override;

    /**
     * Get the currently selected message for detail view
     * @return MessageInfo object, or invalid MessageInfo if no selection
     */
    MessageInfo getSelectedMessage() const;
    
    /**
     * Check if there is a valid message selection
     * @return true if a message is selected and valid
     */
    bool hasValidSelection() const;

    /**
     * Set the message filter
     * @param filter The filter to apply
     */
    void setFilter(const MessageFilter& filter) {
        currentFilter = filter;
        refreshMessageList();
    }

protected:
    // BaseListScreen abstract methods
    virtual void drawItem(lgfx::LGFX_Device& tft, int index, int y, bool isSelected) override;
    virtual int getItemCount() override;
    virtual void onItemSelected(int index) override;
    virtual bool onBeforeDrawItems(lgfx::LGFX_Device& tft) override;

private:
    /**
     * Refresh the message list from mesh database
     */
    void refreshMessageList();
    
    /**
     * Format time since message for display
     */
    String formatTimeSince(uint32_t timestamp);

    // Message data
    std::vector<MessageInfo> messages;
    MessageFilter currentFilter;
    
    // UI state
    bool isLoading;         // Currently loading flag
    unsigned long lastRefreshTime;
    
    // Colors
    static const uint16_t COLOR_BLACK = 0x0000;
    static const uint16_t COLOR_GREEN = 0x07E0;
    static const uint16_t COLOR_RED = 0xF800;
    static const uint16_t COLOR_YELLOW = 0xFFE0;
    static const uint16_t COLOR_DIM_GREEN = 0x4208;
    static const uint16_t COLOR_DARK_RED = 0x7800;
    static const uint16_t COLOR_GRAY = 0x8410;
};