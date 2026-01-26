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

#include "NodesListScreen.h"
#include "modules/CustomUI/CustomUIModule.h"
#include "gps/RTC.h" // for getTime() function
#include <Arduino.h>
#include <algorithm>

// Logging macro
#ifndef LOG_INFO
#define LOG_INFO(format, ...) Serial.printf("[INFO] " format "\n", ##__VA_ARGS__)
#endif

NodesListScreen::NodesListScreen() : BaseListScreen("Mesh Nodes", 25) {
    // Set navigation hints
    std::vector<NavHint> hints;
    hints.push_back(NavHint('1', "Select"));
    hints.push_back(NavHint('A', "Back"));
    setNavigationHints(hints);
    
    isLoading = false;
    lastRefreshTime = 0;
    
    LOG_INFO("📡 NodesListScreen: Created");
}

NodesListScreen::~NodesListScreen() {
    LOG_INFO("📡 NodesListScreen: Destroyed");
}

void NodesListScreen::onEnter(const NavigationContext& ctx) {
    LOG_INFO("📡 NodesListScreen: Entering screen");
    
    // Call parent onEnter to handle selection preservation
    BaseListScreen::onEnter(ctx);
    
    // Initialize state
    nodes.clear();
    channels.clear();
    isLoading = false;
    
    // Load data on next update cycle
    lastRefreshTime = 0; // This will trigger refresh in onBeforeDrawItems
    
    LOG_INFO("📡 NodesListScreen: Screen ready, data will load on next update");
}

void NodesListScreen::onExit() {
    LOG_INFO("📡 NodesListScreen: Exiting screen - cleaning memory");
    
    // Call parent onExit
    BaseListScreen::onExit();
    
    // Force complete vector deallocation
    nodes.clear();
    nodes.shrink_to_fit();
    std::vector<NodeInfo>().swap(nodes);
    
    channels.clear();
    channels.shrink_to_fit();
    std::vector<ChannelHelperInfo>().swap(channels);
    
    // Reset state
    isLoading = false;
    lastRefreshTime = 0;
    
    // Log memory cleanup
    LOG_INFO("📡 NodesListScreen: Vector memory deallocated, state reset");
}

bool NodesListScreen::onBeforeDrawItems(lgfx::LGFX_Device& tft) {
    // Refresh list periodically or on first load
    unsigned long now = millis();
    if (lastRefreshTime == 0 || (now - lastRefreshTime > 10000)) { // Refresh every 10 seconds (reduced frequency)
        refreshNodesList();
        lastRefreshTime = now;
    }
    
    if (isLoading) {
        // Clear content area and show loading message
        tft.fillRect(0, getContentY(), getContentWidth(), getContentHeight(), COLOR_BLACK);
        tft.setTextColor(COLOR_YELLOW, COLOR_BLACK);
        tft.setTextSize(1);
        tft.setCursor(10, getContentY() + 20);
        tft.print("Loading mesh info...");
        return true; // We handled the drawing
    }
    
    if (nodes.empty() && channels.empty()) {
        // Clear content area and show no data message
        tft.fillRect(0, getContentY(), getContentWidth(), getContentHeight(), COLOR_BLACK);
        tft.setTextColor(COLOR_DARK_RED, COLOR_BLACK);
        tft.setTextSize(1);
        tft.setCursor(10, getContentY() + 20);
        tft.print("No nodes/channels found");
        
        tft.setTextColor(COLOR_DIM_GREEN, COLOR_BLACK);
        tft.setCursor(10, getContentY() + 40);
        tft.print("Press [#] to refresh");
        return true; // We handled the drawing
    }
    
    return false; // Let BaseListScreen handle normal list drawing
}

bool NodesListScreen::handleKeyPress(char key) {
    LOG_INFO("📡 NodesListScreen: Key pressed: %c (isLoading: %s, nodes: %d)", 
        key, isLoading ? "true" : "false", nodes.size());
    
    if (isLoading) {
        return true; // Ignore keys while loading
    }
    
    switch (key) {
        case 'A':
        case 'a':
            LOG_INFO("📡 NodesListScreen: Back button pressed");
            if (customUIModule) {
                customUIModule->getScreenManager()->navigateBack();
            }
            return true;

        case '1':
            // Send request to selected node or channel
            {
                int idx = getSelectedIndex();
                if (idx < 0) return true;
                
                if (customUIModule) {
                    T9InputScreen* t9 = customUIModule->getT9InputScreen();
                    if (t9) {
                        
                        // CASE 1: CHANNEL SELECTED
                        if (idx < static_cast<int>(channels.size())) {
                            const ChannelHelperInfo& selectedCh = channels[idx];
                            uint8_t chIndex = selectedCh.index;
                            String chName = String(selectedCh.name);
                            
                            LOG_INFO("📡 NodesListScreen: Selected Channel: %s (%d)", chName.c_str(), chIndex);
                            
                            customUIModule->getScreenManager()->navigateToT9(t9, [chIndex](const String& text) {
                                // Send message to channel (Broadcast)
                                uint32_t packetId = LoRaHelper::sendMessage(text, UINT32_MAX, chIndex);
                                
                                if (packetId != 0) {
                                     LOG_INFO("Message sent to Channel %d with Packet ID: %d", chIndex, packetId);
                                }

                                if (customUIModule) {
                                    customUIModule->getScreenManager()->showPopup("Sent to Channel!", 1000);
                                    customUIModule->getScreenManager()->navigateBack();
                                }
                            });
                        }
                        // CASE 2: NODE SELECTED
                        else {
                            int nodeIdx = idx - channels.size();
                            if (nodeIdx >= 0 && nodeIdx < static_cast<int>(nodes.size())) {
                                const NodeInfo& selectedNode = nodes[nodeIdx];
                                uint32_t destNodeId = selectedNode.nodeNum;
                                
                                LOG_INFO("📡 NodesListScreen: Selected Node: 0x%08X", destNodeId);
                                
                                customUIModule->getScreenManager()->navigateToT9(t9, [destNodeId](const String& text) {
                                    // Send direct message
                                    uint32_t packetId = LoRaHelper::sendMessage(text, destNodeId, 0); 
                                    
                                    if (packetId != 0) {
                                         LOG_INFO("Message sent to %08X with Packet ID: %d", destNodeId, packetId);
                                    }

                                    if (customUIModule) {
                                        customUIModule->getScreenManager()->showPopup("Message Sent!", 1000);
                                        customUIModule->getScreenManager()->navigateBack();
                                    }
                                });
                            }
                        }
                    }
                }
            }
            return true;
            
        case '#':
            LOG_INFO("📡 NodesListScreen: Refreshing nodes list");
            refreshNodesList();
            return true;
            
        default:
            // Let BaseListScreen handle navigation (arrow keys, select)
            return BaseListScreen::handleKeyPress(key);
    }
}

void NodesListScreen::refreshNodesList() {
    LOG_INFO("📡 NodesListScreen: Refreshing list");
    isLoading = true;
    
    // Get nodes and channels
    std::vector<NodeInfo> newNodes = LoRaHelper::getNodesList(25, true);
    std::vector<ChannelHelperInfo> newChannels = LoRaHelper::getChannelList();
    
    // Check if data changed
    bool dataChanged = (newNodes.size() != nodes.size()) || (newChannels.size() != channels.size());
    
    if (!dataChanged) {
        // Check node data content
        for (size_t i = 0; i < newNodes.size() && i < nodes.size(); i++) {
            if (newNodes[i].nodeNum != nodes[i].nodeNum || 
                newNodes[i].lastHeard != nodes[i].lastHeard ||
                newNodes[i].snr != nodes[i].snr) {
                dataChanged = true;
                break;
            }
        }
        // Simplified check for channels (assuming they change less often)
    }
    
    if (dataChanged) {
        nodes = newNodes;
        channels = newChannels;
        
        int totalItems = nodes.size() + channels.size();
        
        // Reset selection if current selection is out of bounds
        if (getSelectedIndex() >= totalItems) {
            setSelection(std::max(0, totalItems - 1));
        }
        
        // Only invalidate list if data actually changed
        invalidateList();
        LOG_INFO("📡 NodesListScreen: Data changed, list invalidated");
    }
    
    isLoading = false;
    LOG_INFO("📡 NodesListScreen: Refresh completed, %d channels, %d nodes", channels.size(), nodes.size());
}

void NodesListScreen::onItemSelected(int index) {
    // Handled in handleKeyPress
}

int NodesListScreen::getItemCount() {
    return static_cast<int>(channels.size() + nodes.size());
}

void NodesListScreen::drawSignalBars(lgfx::LGFX_Device& tft, int x, int y, int bars) {
    // Draw 4 possible bars, fill based on signal strength
    for (int i = 0; i < 4; i++) {
        int barHeight = 2 + (i * 2); // 2, 4, 6, 8 pixels high
        int barY = y + 12 - barHeight;
        int barX = x + (i * 3);
        
        uint16_t color = (i < bars) ? COLOR_GREEN : 0x2104; // Bright green or dark green
        tft.fillRect(barX, barY, 2, barHeight, color);
    }
}

void NodesListScreen::drawItem(lgfx::LGFX_Device& tft, int index, int y, bool isSelected) {
    if (index < 0 || index >= getItemCount()) {
        return; // Invalid index
    }
    
    // BaseListScreen needs COLOR_SELECTION constant
    static const uint16_t COLOR_SELECTION = 0x4208; // Dim green for selection
    uint16_t bgColor = isSelected ? COLOR_SELECTION : COLOR_BLACK;

    // Check if it's a channel or a node
    if (index < static_cast<int>(channels.size())) {
        // --- CHANNEL DRAWING ---
        const ChannelHelperInfo& ch = channels[index];
        
        // Channel Icon/Type
        tft.setTextColor(COLOR_YELLOW, bgColor);
        tft.setTextSize(1);
        tft.setCursor(8, y + 6); // Align with where signal bars would be
        if (ch.isPrimary) {
            tft.print("P");
        } else if (ch.isSecondary) {
            tft.print("S");
        } else {
            tft.print("#");
        }
        
        // Channel Name
        tft.setTextColor(isSelected ? 0xFFFF : COLOR_GREEN, bgColor);
        tft.setCursor(30, y + 6);
        tft.print(ch.name);
        
        // Channel Index (Right aligned)
        tft.setTextColor(COLOR_DIM_GREEN, bgColor);
        String idxStr = "Idx:" + String(ch.index);
        int idxWidth = tft.textWidth(idxStr);
        tft.setCursor(getContentWidth() - idxWidth - 5, y + 6);
        tft.print(idxStr);
        
        return;
    }
    
    // --- NODE DRAWING ---
    // Adjust index to map to nodes vector
    int nodeIndex = index - channels.size();
    if (nodeIndex >= static_cast<int>(nodes.size())) return;
    
    const NodeInfo& node = nodes[nodeIndex];

    // Signal strength bars (Vertical center aligned around y+12)
    // Adjust y to center bars in 25px height
    drawSignalBars(tft, 8, y + 6, node.signalBars);
    
    // Name Color Logic
    uint16_t textColor = isSelected ? 0xFFFF : COLOR_GREEN; // White when selected, green when not
    if (!node.isOnline) {
        textColor = isSelected ? 0xC618 : COLOR_DIM_GREEN; // Light grey when selected, dim green when not
    }
    
    tft.setTextColor(textColor, bgColor);
    tft.setTextSize(1);
    
    // Layout Layout
    int contentWidth = getContentWidth();
    int rightEdge = contentWidth - 5;
    int displayX = 30; // After signal bars
    
    // Right Side Info: distance/snr/time
    // Prioritize Time, then SNR
    
    // Time String
    String timeStr = formatTimeSince(node.lastHeard);
    int timeWidth = tft.textWidth(timeStr);
    
    // Draw Time (Right Aligned, Top Line)
    uint16_t timeColor = node.isOnline ? COLOR_GREEN : COLOR_DIM_GREEN;
    if (isSelected) {
        timeColor = node.isOnline ? 0xFFFF : 0xC618;
    }
    tft.setTextColor(timeColor, bgColor);
    tft.setCursor(rightEdge - timeWidth, y + 4);
    tft.print(timeStr);
    
    // Draw SNR (Right Aligned, Bottom Line)
    // Small SNR display
    String snrStr = "SNR:" + String(node.snr, 0); 
    int snrWidth = tft.textWidth(snrStr);
    
    uint16_t snrColor = isSelected ? 0xC618 : COLOR_DIM_GREEN; 
    tft.setTextColor(snrColor, bgColor);
    tft.setCursor(rightEdge - snrWidth, y + 14);
    tft.print(snrStr);

    // Main Name (Left, Top Line)
    // Calculate max width for name
    int maxNameWidth = (rightEdge - std::max(timeWidth, snrWidth)) - displayX - 10;
    
    String displayName = String(node.longName);
    
    if (tft.textWidth(displayName) > maxNameWidth) {
         String displayMsg = displayName;
         while (displayMsg.length() > 0 && tft.textWidth(displayMsg + "..") > maxNameWidth) {
             displayMsg.remove(displayMsg.length() - 1);
         }
         displayName = displayMsg + "..";
    }
    
    tft.setTextColor(textColor, bgColor);
    tft.setCursor(displayX, y + 4);
    tft.print(displayName);
    
    // Bottom Line: Badges (Favorite, Internet, Hops)
    int badgeX = displayX;
    int badgeY = y + 14;
    
    // 1. Favorite
    if (node.isFavorite) {
        tft.setTextColor(COLOR_YELLOW, bgColor);
        tft.setCursor(badgeX, badgeY);
        tft.print("*Fav");
        badgeX += 30;
    }
    
    // 2. Internet
    if (node.viaInternet) {
        uint16_t indicatorColor = isSelected ? 0x87FF : COLOR_BLUE; 
        tft.setTextColor(indicatorColor, bgColor);
        tft.setCursor(badgeX, badgeY);
        tft.print("MQTT");
        badgeX += 30;
    }
    
    // 3. Hops
    if (node.hopsAway > 0) {
        uint16_t hopsColor = isSelected ? 0xFFFF : COLOR_DIM_GREEN; 
        tft.setTextColor(hopsColor, bgColor);
        tft.setCursor(badgeX, badgeY);
        tft.print("Hop:" + String(node.hopsAway));
    } else if (node.hopsAway == 0) {
        // Direct
    }
}

String NodesListScreen::formatTimeSince(uint32_t lastHeard) {
    if (lastHeard == 0) {
        return "Never";
    }
    
    uint32_t now = getTime();
    uint32_t elapsed = now - lastHeard;
    
    if (elapsed < 60) {
        return "Now";
    } else if (elapsed < 3600) { // Less than 1 hour
        int minutes = elapsed / 60;
        return String(minutes) + "m";
    } else if (elapsed < 86400) { // Less than 1 day
        int hours = elapsed / 3600;
        return String(hours) + "h";
    } else { // Days
        int days = elapsed / 86400;
        return String(days) + "d";
    }
}

