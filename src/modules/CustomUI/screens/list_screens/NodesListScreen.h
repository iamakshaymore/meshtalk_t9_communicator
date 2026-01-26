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

/**
 * Nodes List Screen - Shows mesh nodes with selection
 * Features:
 * - Scrollable list of up to 15 nodes
 * - Signal strength indicators (SNR)
 * - Last heard time display
 * - Online/offline status
 * - Navigation: [A] Back, [1] Select
 */
class NodesListScreen : public BaseListScreen {
public:
    NodesListScreen();
    virtual ~NodesListScreen();

    // BaseListScreen interface  
    virtual void onEnter(const NavigationContext& ctx) override;
    virtual void onExit() override;
    virtual bool handleKeyPress(char key) override;

protected:
    // BaseListScreen abstract methods
    virtual void drawItem(lgfx::LGFX_Device& tft, int index, int y, bool isSelected) override;
    virtual int getItemCount() override;
    virtual void onItemSelected(int index) override;
    virtual bool onBeforeDrawItems(lgfx::LGFX_Device& tft) override;

private:
    /**
     * Refresh the nodes list from mesh database
     */
    void refreshNodesList();
    
    /**
     * Draw signal strength bars from SNR
     */
    void drawSignalBars(lgfx::LGFX_Device& tft, int x, int y, int bars);
    
    /**
     * Format time since last heard for display
     */
    String formatTimeSince(uint32_t lastHeard);

    // Node and Channel data
    std::vector<ChannelHelperInfo> channels;
    std::vector<NodeInfo> nodes;
    
    // UI state
    bool isLoading;         // Currently loading flag
    unsigned long lastRefreshTime;
    
    // Layout constants
    static const int SIGNAL_BAR_WIDTH = 20;
    
    // Colors
    static const uint16_t COLOR_BLACK = 0x0000;
    static const uint16_t COLOR_GREEN = 0x07E0;
    static const uint16_t COLOR_YELLOW = 0xFFE0;
    static const uint16_t COLOR_DIM_GREEN = 0x4208;
    static const uint16_t COLOR_DARK_RED = 0x7800;
    static const uint16_t COLOR_BLUE = 0x001F;
    static const uint16_t COLOR_GRAY = 0x8410;
};