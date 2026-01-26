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

#include "MessageListScreen.h"
#include "modules/CustomUI/CustomUIModule.h"
#include "../MessageDetailsScreen.h"
#include "gps/RTC.h" // for getTime() function
#include <Arduino.h>
#include <algorithm>

// Logging macro
#ifndef LOG_INFO
#define LOG_INFO(format, ...) Serial.printf("[INFO] " format "\n", ##__VA_ARGS__)
#endif

MessageListScreen::MessageListScreen(MessageFilter filter) : BaseListScreen("Messages", 25), currentFilter(filter) {
    // Set navigation hints
    std::vector<NavHint> hints;
    
    // Customize title and hints based on filter
    if (currentFilter.active) {
        if (currentFilter.onlyOutgoing) {
            // No select/reply for sent messages
            hints.push_back(NavHint('A', "Back"));
        } else if (currentFilter.onlyIncomingDMs) {
            hints.push_back(NavHint('1', "Select"));
            hints.push_back(NavHint('A', "Back"));
        } else if (currentFilter.channelIndex != -1) {
            char titleBuf[20];
            snprintf(titleBuf, sizeof(titleBuf), "Channel %d", currentFilter.channelIndex);
            hints.push_back(NavHint('1', "Select"));
            hints.push_back(NavHint('A', "Back"));
        } else {
             hints.push_back(NavHint('1', "Select"));
             hints.push_back(NavHint('A', "Back"));
        }
    } else {
        // Default View
        hints.push_back(NavHint('1', "Select"));
        hints.push_back(NavHint('A', "Back"));
    }
    
    setNavigationHints(hints);
    
    isLoading = false;
    lastRefreshTime = 0;
    
    LOG_INFO("💬 MessageListScreen: Created with filter active=%d", filter.active);
}

MessageListScreen::~MessageListScreen() {
    LOG_INFO("💬 MessageListScreen: Destroyed");
}

void MessageListScreen::onEnter(const NavigationContext& ctx) {
    LOG_INFO("💬 MessageListScreen: Entering screen");
    
    // Call parent onEnter to preserve selection
    BaseListScreen::onEnter(ctx);
    
    // Initialize message state
    messages.clear();
    isLoading = false;
    
    // Load messages on next update cycle
    lastRefreshTime = 0; // This will trigger refresh in onBeforeDrawItems
    
    LOG_INFO("💬 MessageListScreen: Screen ready, messages will load on next update");
}

void MessageListScreen::onExit() {
    LOG_INFO("💬 MessageListScreen: Exiting screen - cleaning memory");
    
    // Call parent onExit
    BaseListScreen::onExit();
    
    // Force complete vector deallocation
    messages.clear();
    messages.shrink_to_fit();
    std::vector<MessageInfo>().swap(messages);
    
    // Reset message state
    isLoading = false;
    lastRefreshTime = 0;
    
    // Log memory cleanup
    LOG_INFO("💬 MessageListScreen: Vector memory deallocated, state reset");
}

bool MessageListScreen::onBeforeDrawItems(lgfx::LGFX_Device& tft) {
    // Refresh message list periodically or on first load
    unsigned long now = millis();
    if (lastRefreshTime == 0 || (now - lastRefreshTime > 10000)) { // Refresh every 10 seconds
        refreshMessageList();
        lastRefreshTime = now;
    }
    
    if (isLoading) {
        // Clear content area and show loading message
        tft.fillRect(0, getContentY(), getContentWidth(), getContentHeight(), COLOR_BLACK);
        tft.setTextColor(COLOR_YELLOW, COLOR_BLACK);
        tft.setTextSize(1);
        tft.setCursor(10, getContentY() + 20);
        tft.print("Loading messages...");
        return true; // We handled the drawing
    }
    
    if (messages.empty()) {
        // Clear content area and show no messages message
        tft.fillRect(0, getContentY(), getContentWidth(), getContentHeight(), COLOR_BLACK);
        tft.setTextColor(COLOR_DARK_RED, COLOR_BLACK);
        tft.setTextSize(1);
        tft.setCursor(10, getContentY() + 20);
        tft.print("No messages found");
        
        tft.setTextColor(COLOR_DIM_GREEN, COLOR_BLACK);
        tft.setCursor(10, getContentY() + 40);
        tft.print("Messages will appear here");
        return true; // We handled the drawing
    }
    
    return false; // Let BaseListScreen handle normal list drawing
}

bool MessageListScreen::handleKeyPress(char key) {
    LOG_INFO("💬 MessageListScreen: Key pressed: %c (isLoading: %s, messages: %d)", 
        key, isLoading ? "true" : "false", messages.size());
    
    if (isLoading) {
        return true; // Ignore keys while loading
    }
    
    switch (key) {
        case '1':
            LOG_INFO("💬 MessageListScreen: Details button pressed");
            if (customUIModule && hasValidSelection()) {
                MessageInfo msg = getSelectedMessage();
                MessageDetailsScreen* details = customUIModule->getMessageDetailsScreen();
                if (details) {
                    details->setMessage(msg);
                    customUIModule->getScreenManager()->navigateTo(details);
                }
            }
            return true;
            
        case 'A':
        case 'a':
            LOG_INFO("💬 MessageListScreen: Back button pressed");
            if (customUIModule) {
                customUIModule->getScreenManager()->navigateBack();
            }
            return true;
            
        case '#':
            LOG_INFO("💬 MessageListScreen: Refreshing message list");
            refreshMessageList();
            return true;
            
        default:
            // Let BaseListScreen handle navigation (arrow keys, select)
            return BaseListScreen::handleKeyPress(key);
    }
}

void MessageListScreen::refreshMessageList() {
    LOG_INFO("💬 MessageListScreen: Refreshing message list");
    isLoading = true;
    
    // Get messages from LoRa helper (fetch up to 100 messages)
    int fetchCount = 100;
    std::vector<MessageInfo> fetchedMessages = LoRaHelper::getRecentMessages(fetchCount);
    std::vector<MessageInfo> newMessages;
    
    // Apply filters if active
    if (currentFilter.active) {
        for (const auto& msg : fetchedMessages) {
            bool matches = true;
            
            // Filter 1: Only sent messages
            if (currentFilter.onlyOutgoing && !msg.isOutgoing) {
                matches = false;
            }
            
            // Filter 2: Only Incoming DMs
            if (currentFilter.onlyIncomingDMs) {
                // strict interpretation: must be incoming AND a direct message
                if (msg.isOutgoing || !msg.isDirectMessage) {
                    matches = false;
                }
            }
            
            // Filter 2b: All DMs (Incoming and Outgoing)
            if (currentFilter.onlyDMs) {
                if (!msg.isDirectMessage) {
                    matches = false;
                }
            }
            
            // Filter 3: Channel Index
            if (currentFilter.channelIndex != -1) {
                // Must match channel index
                if (msg.channelIndex != currentFilter.channelIndex) {
                    matches = false;
                }
                
                // Exclude DMs from Channel views (they belong in "Direct Messages" or "Outbox")
                // Channel views are for Broadcasts/Group Chats
                if (msg.isDirectMessage) {
                    matches = false;
                }
            }
            
            if (matches) {
                newMessages.push_back(msg);
            }
            
            // Limit to reasonable list size
            if (newMessages.size() >= 100) break;
        }
    } else {
        newMessages = fetchedMessages;
         // Limit to reasonable list size
        if (newMessages.size() > 100) newMessages.resize(100);
    }
    
    // Only update if data actually changed
    bool dataChanged = (newMessages.size() != messages.size());
    if (!dataChanged) {
        // Check if any message data changed
        for (size_t i = 0; i < newMessages.size() && i < messages.size(); i++) {
            if (newMessages[i].timestamp != messages[i].timestamp || 
                strcmp(newMessages[i].text, messages[i].text) != 0 || 
                newMessages[i].ackReceived != messages[i].ackReceived) {
                dataChanged = true;
                break;
            }
        }
    }
    
    if (dataChanged) {
        messages = newMessages;
        
        // Reset selection if current selection is out of bounds
        if (getSelectedIndex() >= static_cast<int>(messages.size())) {
            setSelection(std::max(0, static_cast<int>(messages.size()) - 1));
        }
        
        // Only invalidate list if data actually changed
        invalidateList();
        LOG_INFO("💬 MessageListScreen: Data changed, list invalidated");
    }
    
    isLoading = false;
    LOG_INFO("💬 MessageListScreen: Refresh completed, found %d messages (changed: %s)", 
             messages.size(), dataChanged ? "yes" : "no");
}

void MessageListScreen::onItemSelected(int index) {
    // Selection is now handled by CustomUIModule navigation logic
    LOG_INFO("💬 MessageListScreen: Item %d selected", index);
}

int MessageListScreen::getItemCount() {
    return static_cast<int>(messages.size());
}

void MessageListScreen::drawItem(lgfx::LGFX_Device& tft, int index, int y, bool isSelected) {
    if (index < 0 || index >= static_cast<int>(messages.size())) {
        return; // Invalid index
    }
    
    const MessageInfo& msg = messages[index];
    
    // BaseListScreen needs COLOR_SELECTION constant
    static const uint16_t COLOR_SELECTION = 0x4208; // Dim green for selection
    
    // Background color
    uint16_t bgColor = isSelected ? COLOR_SELECTION : COLOR_BLACK;
    
    // Draw Outgoing/Incoming indicator
    if (msg.isOutgoing) {
         // > for outgoing. Green if ACKed, Red if not.
         uint16_t indicatorColor = msg.ackReceived ? COLOR_GREEN : COLOR_RED;
         tft.setTextColor(indicatorColor, bgColor);
         tft.setCursor(5, y + 5);
         tft.print(">");
    } else {
         // < for incoming (or standard dot/box) - using dot color logic
         uint16_t typeColor = msg.isDirectMessage ? COLOR_GREEN : COLOR_RED;
         if (isSelected) {
            typeColor = msg.isDirectMessage ? 0xFFFF : 0xFFFF; // White when selected
         }
         tft.fillRect(5, y + 7, 4, 4, typeColor);
    }
    
    // Sender name / Destination (main area)
    uint16_t textColor = isSelected ? 0xFFFF : (msg.isOutgoing ? COLOR_YELLOW : (msg.isDirectMessage ? COLOR_GREEN : COLOR_RED));
    tft.setTextColor(textColor, bgColor);
    tft.setTextSize(1);
    
    // Create display name
    String displayName;
    
    if (msg.isOutgoing) {
        if (msg.isDirectMessage) {
              // Outgoing DMs now store Recipient Name in senderName field
              displayName = "To: " + String(msg.senderName); 
        } else {
             displayName = "To: " + String(msg.channelName);
        }
    } else {
        if (msg.isDirectMessage) {
            displayName = String(msg.senderName) + "[DM]";
        } else {
            displayName = String(msg.senderName) + "[" + String(msg.channelName) + "]";
        }
    }
    
    // Layout Calculation
    int contentWidth = getContentWidth();
    int rightEdge = contentWidth - 5;
    int displayX = 15;

    // Line 1: Name (Left) ... Time (Right)
    
    // Time String
    String timeStr = formatTimeSince(msg.timestamp);
    int timeWidth = tft.textWidth(timeStr);
    
    // Draw Time (Right Aligned)
    uint16_t timeColor = isSelected ? 0xC618 : COLOR_DIM_GREEN; // Light grey when selected
    tft.setTextColor(timeColor, bgColor);
    tft.setCursor(rightEdge - timeWidth, y + 4);
    tft.print(timeStr);
    
    // Draw Name (Left Aligned)
    // Calculate max width for name to avoid overlapping time
    int maxNameWidth = (rightEdge - timeWidth) - displayX - 10;
    
    // Truncate name if too long
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
    
    // Line 2: Message preview
    String messageText = String(msg.text);
    
    // Fix: Replace line breaks to prevent overflow
    messageText.replace("\n", " ");
    messageText.replace("\r", " ");
    
    int maxMsgWidth = rightEdge - displayX;
    
    if (tft.textWidth(messageText) > maxMsgWidth) {
        String displayMsg = messageText;
        while (displayMsg.length() > 0 && tft.textWidth(displayMsg + "...") > maxMsgWidth) {
             displayMsg.remove(displayMsg.length() - 1);
        }
        messageText = displayMsg + "...";
    }
    
    uint16_t msgColor = isSelected ? 0xFFFF : 0xCCCC; // White when selected, light gray otherwise
    tft.setTextColor(msgColor, bgColor);
    tft.setCursor(displayX, y + 14);
    tft.print(messageText);
}

MessageInfo MessageListScreen::getSelectedMessage() const {
    int currentSelection = getSelectedIndex();
    LOG_INFO("💬 MessageListScreen: getSelectedMessage - selection: %d, total messages: %d", 
             currentSelection, static_cast<int>(messages.size()));
    
    if (currentSelection >= 0 && currentSelection < static_cast<int>(messages.size()) && !messages.empty()) {
        LOG_INFO("💬 MessageListScreen: Returning valid message from: %s", messages[currentSelection].senderName);
        return messages[currentSelection];
    }
    LOG_INFO("💬 MessageListScreen: Returning invalid MessageInfo");
    return MessageInfo(); // Return invalid MessageInfo
}

bool MessageListScreen::hasValidSelection() const {
    int currentSelection = getSelectedIndex();
    return currentSelection >= 0 && 
           currentSelection < static_cast<int>(messages.size()) && 
           !messages.empty();
}

String MessageListScreen::formatTimeSince(uint32_t timestamp) {
    if (timestamp == 0) {
        return "Unknown";
    }
    
    uint32_t now = getTime();
    if (now == 0) now = millis() / 1000; // Fallback if RTC not available
    
    uint32_t elapsed = now - timestamp;
    
    if (elapsed < 60) {
        return String(elapsed) + "s ago";
    } else if (elapsed < 3600) { // Less than 1 hour
        int minutes = elapsed / 60;
        return String(minutes) + "m ago";
    } else if (elapsed < 86400) { // Less than 1 day
        int hours = elapsed / 3600;
        return String(hours) + "h ago";
    } else { // Days
        int days = elapsed / 86400;
        return String(days) + "d ago";
    }
}

