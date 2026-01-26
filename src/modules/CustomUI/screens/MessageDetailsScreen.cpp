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

#include "MessageDetailsScreen.h"
#include "BaseScreen.h"
#include "T9InputScreen.h"
#include "modules/CustomUI/CustomUIModule.h"
#include "utils/LoRaHelper.h"
#include "configuration.h"

// Logging macro
#ifndef LOG_INFO
#define LOG_INFO(format, ...) Serial.printf("[INFO] " format "\n", ##__VA_ARGS__)
#endif

MessageDetailsScreen::MessageDetailsScreen() 
    : BaseScreen("Message Details"), messageSet(false), scrollOffset(0), 
      maxVisibleLines(0), totalLines(0), contentDirty(true), headerDirty(true), footerDirty(true) {
    calculateVisibleLines();
    updateNavigationHints();
}

MessageDetailsScreen::~MessageDetailsScreen() {
    clearContent();
}

void MessageDetailsScreen::onEnter(const NavigationContext& ctx) {
    LOG_INFO("📱 MessageDetailsScreen: Entering screen (isBack=%d)", ctx.isBack);
    
    // Only reset scroll if fresh entry
    if (!ctx.isBack) {
        scrollOffset = 0;
    }
    
    // If returning, we need to re-wrap text (it was cleared in onExit to save RAM)
    if (ctx.isBack && hasValidMessage()) {
        wrapTextToLines();
    }
    
    // Mark everything for redraw
    contentDirty = true;
    headerDirty = true;
    footerDirty = true;
    
    updateNavigationHints();
}

void MessageDetailsScreen::onExit() {
    LOG_INFO("📱 MessageDetailsScreen: Exiting screen - cleaning memory");
    
    // Clear text lines vector to free memory
    textLines.clear();
    textLines.shrink_to_fit();
    std::vector<String>().swap(textLines);
    
    // Reset state
    // scrollOffset = 0; // Preserved for back navigation
    totalLines = 0;
    contentDirty = true;
    headerDirty = true;
    footerDirty = true;
    
    LOG_INFO("📱 MessageDetailsScreen: Memory cleaned and state reset");
}

void MessageDetailsScreen::onDraw(lgfx::LGFX_Device& tft) {
    if (!hasValidMessage()) {
        // Clear content area and show "No message" text
        tft.fillRect(0, getContentY(), getContentWidth(), getContentHeight(), COLOR_BLACK);
        tft.setTextColor(COLOR_YELLOW, COLOR_BLACK);
        tft.setTextSize(2);
        tft.setCursor(20, getContentY() + 60);
        tft.print("No message to display");
        tft.setTextSize(1);
        return;
    }

    // Only redraw sender if explicitly marked (e.g. new message)
    if (headerDirty) {
        drawSenderSection(tft);
        headerDirty = false;
    }

    // Redraw content when needed
    if (contentDirty) {
        drawTextSection(tft);
        contentDirty = false;
    }

    // Redraw footer (timestamp and page info) when needed
    if (footerDirty) {
        drawTimestampSection(tft);
        footerDirty = false;
    }
}

bool MessageDetailsScreen::handleKeyPress(char key) {
    if (!hasValidMessage()) {
        return false; // Let UI module handle navigation
    }

    switch (key) {
        case 'A':
            // Back button
            if (customUIModule) {
                 customUIModule->getScreenManager()->navigateBack();
            }
            return true;
            
        case '1':
            // Reply button
            if (customUIModule && !currentMessage.isOutgoing) {
                T9InputScreen* t9 = customUIModule->getT9InputScreen();
                if (t9) {
                    customUIModule->getScreenManager()->navigateToT9(t9, [this](const String& text) {
                        // Determine reply destination (DM vs Channel)
                        uint32_t dest = currentMessage.isDirectMessage ? currentMessage.senderNodeId : UINT32_MAX;
                        uint8_t ch = currentMessage.isDirectMessage ? 0 : currentMessage.channelIndex;

                        // Send message using LoRaHelper
                        uint32_t packetId = LoRaHelper::sendMessage(text, dest, ch);
                        if (packetId != 0) {
                             LOG_INFO("Message sent with Packet ID: %d", packetId);
                             // Future: Register this ID to listen for ACK
                        }

                        // Show popup confirmation
                        if (customUIModule) {
                            customUIModule->getScreenManager()->showPopup("Message Sent!", 1000);
                            customUIModule->getScreenManager()->navigateBack();
                        }
                    });
                }
            }
            return true;
            
        case '2':
            // Scroll up (up arrow)
            scrollUp();
            return true;
            
        case '8':
            // Scroll down (down arrow)
            scrollDown();
            return true;
            
        default:
            return false;
    }
}

void MessageDetailsScreen::setMessage(const MessageInfo& msgInfo) {
    LOG_INFO("📱 MessageDetailsScreen: Setting message from sender: %s", msgInfo.senderName);
    
    currentMessage = msgInfo;
    messageSet = msgInfo.isValid;
    
    if (messageSet) {
        // Wrap text to lines and reset scroll
        wrapTextToLines();
        scrollOffset = 0;
        
        // Mark all sections for redraw
        contentDirty = true;
        headerDirty = true;
        footerDirty = true;
        
        updateNavigationHints();
        forceRedraw();
        
        LOG_INFO("📱 MessageDetailsScreen: Message set successfully, %d total lines", totalLines);
    } else {
        LOG_INFO("📱 MessageDetailsScreen: Invalid message provided");
        clearContent();
    }
}

bool MessageDetailsScreen::hasValidMessage() const {
    return messageSet && currentMessage.isValid;
}

const MessageInfo& MessageDetailsScreen::getCurrentMessage() const {
    return currentMessage;
}

void MessageDetailsScreen::wrapTextToLines() {
    textLines.clear();
    
    if (!hasValidMessage()) {
        totalLines = 0;
        return;
    }

    // Use pixel-based wrapping for accurate text fitting
    String fullText = String(currentMessage.text);
    const int TEXT_MARGIN = 10;              // Left margin
    const int SCROLLBAR_WIDTH = 20;          // Reserve space for scrollbar
    const int AVAILABLE_WIDTH = getContentWidth() - TEXT_MARGIN - SCROLLBAR_WIDTH; // 290px effective width
    const int CHAR_WIDTH = 12;               // Approximate width per character at size 2 font
    const int CHARS_PER_LINE = AVAILABLE_WIDTH / CHAR_WIDTH; // ~24 chars for safe wrapping
    
    // Split by newlines first
    int start = 0;
    while (start < fullText.length()) {
        int newlinePos = fullText.indexOf('\n', start);
        if (newlinePos == -1) newlinePos = fullText.length();

        // Extract paragraph and remove carriage returns
        String paragraph = fullText.substring(start, newlinePos);
        paragraph.replace("\r", "");
        
        // Wrap this paragraph
        if (paragraph.length() == 0) {
             // textLines.push_back(""); // Optional: Add blank line for explicit double-newline
             // For now, let's treat double newline as a single break unless it's significant
             textLines.push_back(" "); // Push a space so it takes up a line
             // Actually, if it's empty, it's an empty line.
        } else {
            int pStart = 0; 
            while (pStart < paragraph.length()) {
                // If remaining fits, push it
                if (paragraph.length() - pStart <= CHARS_PER_LINE) {
                     textLines.push_back(paragraph.substring(pStart));
                     break;
                }
                
                int len = CHARS_PER_LINE;
                int splitIdx = pStart + len;
                
                // Backtrack for space to avoiding splitting words
                int spaceIdx = paragraph.lastIndexOf(' ', splitIdx);
                
                if (spaceIdx > pStart && spaceIdx > (splitIdx - (CHARS_PER_LINE/2))) {
                    // Good break point found
                    textLines.push_back(paragraph.substring(pStart, spaceIdx));
                    pStart = spaceIdx + 1; // Skip the space
                } else {
                    // No good space, hard break
                    textLines.push_back(paragraph.substring(pStart, splitIdx));
                    pStart = splitIdx;
                }
            }
        }
        
        start = newlinePos + 1;
    }
    
    totalLines = textLines.size();
    LOG_INFO("📱 MessageDetailsScreen: Text wrapped into %d lines (width: %dpx, chars: %d)", 
             totalLines, AVAILABLE_WIDTH, CHARS_PER_LINE);
}

void MessageDetailsScreen::calculateVisibleLines() {
    // Dynamic calculation based on available height
    maxVisibleLines = TEXT_AREA_HEIGHT / LINE_HEIGHT;
    LOG_INFO("📱 MessageDetailsScreen: Max visible lines per page: %d", maxVisibleLines);
}

void MessageDetailsScreen::drawSenderSection(lgfx::LGFX_Device& tft) {
    if (!hasValidMessage()) return;
    
    // Clear sender area
    int senderY = getContentY();
    tft.fillRect(0, senderY, getContentWidth(), SENDER_HEIGHT, COLOR_BLACK);
    
    // Draw sender name in green, bold style (size 2)
    tft.setTextColor(COLOR_GREEN, COLOR_BLACK);
    tft.setTextSize(2);
    tft.setCursor(10, senderY + 5);
    currentMessage.isOutgoing ? tft.print("To: ") : tft.print("From: ");
    tft.print(currentMessage.senderName);
    tft.setTextSize(1);
    
    LOG_INFO("📱 MessageDetailsScreen: Drew sender section");
}

void MessageDetailsScreen::drawTextSection(lgfx::LGFX_Device& tft) {
    if (!hasValidMessage()) return;
    
    // Calculate text area position
    int textY = getContentY() + SENDER_HEIGHT + 5;
    const int TEXT_MARGIN = 10;
    
    // Clear text area (avoid clearing sender area)
    tft.fillRect(0, textY, getContentWidth(), TEXT_AREA_HEIGHT, COLOR_BLACK);
    
    // Draw visible lines for current page
    tft.setTextColor(COLOR_WHITE, COLOR_BLACK);
    tft.setTextSize(2);
    
    int y = textY + 5; // Small top padding
    int currentPage = scrollOffset;
    int startLine = currentPage * maxVisibleLines;
    int endLine = std::min(startLine + maxVisibleLines, totalLines);
    
    for (int i = startLine; i < endLine; i++) {
        tft.setCursor(TEXT_MARGIN, y);
        tft.print(textLines[i]);
        y += LINE_HEIGHT;
    }
    
    tft.setTextSize(1);
    
    // Draw page indicator instead of scrollbar
    // if (totalLines > maxVisibleLines) {
    //     int totalPages = (totalLines + maxVisibleLines - 1) / maxVisibleLines;
    //     int currentPageNum = currentPage + 1;
        
    //     // Page indicator on the right
    //     String pageInfo = "Page " + String(currentPageNum) + "/" + String(totalPages);
    //     tft.setTextColor(COLOR_GRAY, COLOR_BLACK);
    //     tft.setCursor(getContentWidth() - 80, textY + TEXT_AREA_HEIGHT - 15);
    //     tft.print(pageInfo);
    // }
    
    LOG_INFO("📱 MessageDetailsScreen: Drew page %d, lines %d-%d of %d", 
             currentPage + 1, startLine + 1, endLine, totalLines);
}

void MessageDetailsScreen::drawTimestampSection(lgfx::LGFX_Device& tft) {
    if (!hasValidMessage()) return;
    
    // Calculate timestamp area position
    int timestampY = getContentY() + getContentHeight() - TIMESTAMP_HEIGHT;
    
    // Clear timestamp area
    tft.fillRect(0, timestampY, getContentWidth(), TIMESTAMP_HEIGHT, COLOR_BLACK);
    
    // Format and draw timestamp (matching MessagesScreen style)
    unsigned long t = currentMessage.timestamp;
    unsigned int h = (t / 3600) % 24;
    unsigned int m = (t / 60) % 60;
    unsigned int s = t % 60;
    char timebuf[32];
    currentMessage.isOutgoing ? 
    snprintf(timebuf, sizeof(timebuf), "Sent: %02u:%02u:%02u", h, m, s) 
    : snprintf(timebuf, sizeof(timebuf), "Received: %02u:%02u:%02u", h, m, s);
    
    tft.setTextColor(COLOR_YELLOW, COLOR_BLACK);
    tft.setTextSize(1);
    tft.setCursor(10, timestampY + 5);
    tft.print(timebuf);
    
    // Show page position if multiple pages
    if (totalLines > maxVisibleLines) {
        int totalPages = (totalLines + maxVisibleLines - 1) / maxVisibleLines;
        String pageInfo = String(scrollOffset + 1) + "/" + String(totalPages) + " pages";
        
        tft.setCursor(getContentWidth() - 80, timestampY + 5);
        tft.print(pageInfo);
    }
    
    LOG_INFO("📱 MessageDetailsScreen: Drew timestamp section");
}

void MessageDetailsScreen::scrollUp() {
    if (scrollOffset > 0) {
        scrollOffset--; // Move to previous page
        contentDirty = true; // Content changes
        footerDirty = true;  // Page number changes
        // Header (Sender) does not change
        updateNavigationHints();
        forceRedraw();
        LOG_INFO("📱 MessageDetailsScreen: Scrolled to page %d", scrollOffset + 1);
    }
}

void MessageDetailsScreen::scrollDown() {
    int totalPages = (totalLines + maxVisibleLines - 1) / maxVisibleLines;
    if (scrollOffset < totalPages - 1) {
        scrollOffset++; // Move to next page
        contentDirty = true; // Content changes
        footerDirty = true;  // Page number changes
        // Header (Sender) does not change
        updateNavigationHints();
        forceRedraw();
        LOG_INFO("📱 MessageDetailsScreen: Scrolled to page %d", scrollOffset + 1);
    }
}

void MessageDetailsScreen::updateNavigationHints() {
    std::vector<NavHint> newHints;
    
    // Show reply button if message is valid
    if (hasValidMessage() && !currentMessage.isOutgoing) {
        newHints.push_back(NavHint('1', "Reply"));
    }
    
    // Show page navigation hints only if message has multiple pages
    if (hasValidMessage() && totalLines > maxVisibleLines) {
        int totalPages = (totalLines + maxVisibleLines - 1) / maxVisibleLines;
        
        if (scrollOffset > 0) {
            newHints.push_back(NavHint('2', "PgUp"));
        }
        if (scrollOffset < totalPages - 1) {
            newHints.push_back(NavHint('8', "PgDn"));
        }
    }

    // Always show back button
    newHints.push_back(NavHint('A', "Back"));
    
    // Check if hints actually changed to avoid unnecessary redraws
    bool changed = false;
    if (navHints.size() != newHints.size()) {
        changed = true;
    } else {
        for (size_t i = 0; i < navHints.size(); i++) {
            if (navHints[i].key != newHints[i].key || !navHints[i].label.equals(newHints[i].label)) {
                changed = true;
                break;
            }
        }
    }
    
    // Only update if changed
    if (changed) {
        setNavigationHints(newHints);
        LOG_INFO("📱 MessageDetailsScreen: Updated navigation hints");
    }
}

void MessageDetailsScreen::clearContent() {
    textLines.clear();
    scrollOffset = 0;
    totalLines = 0;
    messageSet = false;
    contentDirty = true;
    headerDirty = true;
    footerDirty = true;
    
    // Clear message data
    currentMessage = MessageInfo();
    
    updateNavigationHints();
}