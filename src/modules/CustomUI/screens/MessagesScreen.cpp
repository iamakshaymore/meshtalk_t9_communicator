#include "MessagesScreen.h"
#include "modules/CustomUI/CustomUIModule.h"
#include "modules/CustomUI/screens/HomeScreen.h"
#include "BaseScreen.h"
#include "configuration.h"

// Logging macro
#ifndef LOG_INFO
#define LOG_INFO(format, ...) Serial.printf("[INFO] " format "\n", ##__VA_ARGS__)
#endif

// Colors (matching MessageDetailsScreen)
static const uint16_t COLOR_BLACK = 0x0000;
static const uint16_t COLOR_WHITE = 0xFFFF;
static const uint16_t COLOR_GREEN = 0x07E0;     // Sender name
static const uint16_t COLOR_YELLOW = 0xFFE0;    // Timestamp
static const uint16_t COLOR_GRAY = 0x8410;      // Disabled/secondary text

MessagesScreen::MessagesScreen() 
    : BaseScreen("Messages"), currentIndex(0), scrollOffset(0), 
      maxVisibleLines(0), totalLines(0),
      contentDirty(true), headerDirty(true), footerDirty(true) {
    calculateVisibleLines();
    updateNavHint();
}

MessagesScreen::~MessagesScreen() {}

void MessagesScreen::calculateVisibleLines() {
    // Dynamic calculation based on available height
    maxVisibleLines = TEXT_AREA_HEIGHT / LINE_HEIGHT;
    LOG_INFO("💬 MessagesScreen: Max visible lines per page: %d", maxVisibleLines);
}

void MessagesScreen::onEnter(const NavigationContext& ctx) {
    if (!ctx.isBack) {
        currentIndex = 0;
        scrollOffset = 0;
        if (!buffer.empty()) {
            wrapTextToLines();
        }
    }
    
    // Mark all for redraw
    contentDirty = true;
    headerDirty = true;
    footerDirty = true;
    
    updateNavHint();
    forceRedraw();
}

void MessagesScreen::onExit() {
    LOG_INFO("💬 MessagesScreen: Exiting screen - cleaning memory");
    
    // Force complete vector deallocation
    buffer.clear();
    buffer.shrink_to_fit();
    std::vector<MessageEntry>().swap(buffer);
    
    // Clear text lines
    textLines.clear();
    textLines.shrink_to_fit();
    
    // Reset state
    currentIndex = 0;
    scrollOffset = 0;
    totalLines = 0;
    
    // Update navigation hints
    updateNavHint();
    
    // Log memory cleanup
    LOG_INFO("💬 MessagesScreen: Vector memory deallocated, state reset");
}

void MessagesScreen::wrapTextToLines() {
    textLines.clear();
    
    if (buffer.empty()) {
        totalLines = 0;
        return;
    }

    const MessageEntry& msg = buffer[currentIndex];
    String fullText = msg.text;
    
    const int TEXT_MARGIN = 10;
    const int SCROLLBAR_WIDTH = 20;
    const int AVAILABLE_WIDTH = getContentWidth() - TEXT_MARGIN - SCROLLBAR_WIDTH; 
    const int CHAR_WIDTH = 12; // Approx width for size 2
    const int CHARS_PER_LINE = AVAILABLE_WIDTH / CHAR_WIDTH;
    
    // Split by newlines first
    int start = 0;
    while (start < fullText.length()) {
        int newlinePos = fullText.indexOf('\n', start);
        if (newlinePos == -1) newlinePos = fullText.length();

        String paragraph = fullText.substring(start, newlinePos);
        paragraph.replace("\r", "");
        
        if (paragraph.length() == 0) {
             textLines.push_back(" "); 
        } else {
            int pStart = 0; 
            while (pStart < paragraph.length()) {
                if (paragraph.length() - pStart <= CHARS_PER_LINE) {
                     textLines.push_back(paragraph.substring(pStart));
                     break;
                }
                
                int len = CHARS_PER_LINE;
                int splitIdx = pStart + len;
                int spaceIdx = paragraph.lastIndexOf(' ', splitIdx);
                
                if (spaceIdx > pStart && spaceIdx > (splitIdx - (CHARS_PER_LINE/2))) {
                    textLines.push_back(paragraph.substring(pStart, spaceIdx));
                    pStart = spaceIdx + 1; 
                } else {
                    textLines.push_back(paragraph.substring(pStart, splitIdx));
                    pStart = splitIdx;
                }
            }
        }
        start = newlinePos + 1;
    }
    
    totalLines = textLines.size();
}

void MessagesScreen::onDraw(lgfx::LGFX_Device& tft) {
    if (buffer.empty()) {
        if (contentDirty || headerDirty || footerDirty) {
            tft.fillRect(0, getContentY(), getContentWidth(), getContentHeight(), COLOR_BLACK);
            tft.setTextColor(COLOR_YELLOW, COLOR_BLACK);
            tft.setTextSize(2);
            tft.setCursor(20, getContentY() + 60);
            tft.print("No messages");
            tft.setTextSize(1);
            contentDirty = false; headerDirty = false; footerDirty = false;
        }
        return;
    }

    const MessageEntry& msg = buffer[currentIndex];
    
    // --- 1. Draw Sender Section ---
    if (headerDirty) {
        int senderY = getContentY();
        tft.fillRect(0, senderY, getContentWidth(), SENDER_HEIGHT, COLOR_BLACK);
        tft.setTextColor(COLOR_GREEN, COLOR_BLACK);
        tft.setTextSize(2);
        tft.setCursor(10, senderY + 5);
        tft.print("From: "); 
        tft.print(msg.sender);
        headerDirty = false;
    }
    
    // --- 2. Draw Text Section ---
    if (contentDirty) {
        int textY = getContentY() + SENDER_HEIGHT + 5;
        const int TEXT_MARGIN = 10;
        
        tft.fillRect(0, textY, getContentWidth(), TEXT_AREA_HEIGHT, COLOR_BLACK);
        
        tft.setTextColor(COLOR_WHITE, COLOR_BLACK);
        tft.setTextSize(2);
        
        int y = textY + 5; 
        int currentPage = scrollOffset;
        int startLine = currentPage * maxVisibleLines;
        int endLine = std::min(startLine + maxVisibleLines, totalLines);
        
        for (int i = startLine; i < endLine; i++) {
            tft.setCursor(TEXT_MARGIN, y);
            tft.print(textLines[i]);
            y += LINE_HEIGHT;
        }
        tft.setTextSize(1);
        contentDirty = false;
    }

    // --- 3. Draw Footer (Timestamp & Page Info) ---
    if (footerDirty) {
        int timestampY = getContentY() + getContentHeight() - TIMESTAMP_HEIGHT;
        tft.fillRect(0, timestampY, getContentWidth(), TIMESTAMP_HEIGHT, COLOR_BLACK);
        
        unsigned long t = msg.timestamp / 1000;
        unsigned int h = (t / 3600) % 24;
        unsigned int m = (t / 60) % 60;
        unsigned int s = t % 60;
        char timebuf[32];
        snprintf(timebuf, sizeof(timebuf), "Received: %02u:%02u:%02u", h, m, s);
        
        tft.setTextColor(COLOR_YELLOW, COLOR_BLACK);
        tft.setTextSize(1);
        tft.setCursor(10, timestampY + 5);
        tft.print(timebuf);

        // Right side info: Msg count + Page count
        String info = "";
        
        // Message Index - only if > 1 message
        if (buffer.size() > 1) {
            info += String(currentIndex + 1) + "/" + String(buffer.size());
        }
        
        // Page Index
        if (totalLines > maxVisibleLines) {
            int totalPages = (totalLines + maxVisibleLines - 1) / maxVisibleLines;
            if (info.length() > 0) info += "  ";
            info += String(scrollOffset + 1) + "/" + String(totalPages) + " pages";
        }
        
        if (info.length() > 0) {
            int infoWidth = tft.textWidth(info);
            tft.setCursor(getContentWidth() - infoWidth - 10, timestampY + 5);
            tft.print(info);
        }
        footerDirty = false;
    }
}

bool MessagesScreen::handleKeyPress(char key) {
    if (key == 'A' || key == 'a') {
        if (buffer.empty() || buffer.size() == 1 || currentIndex == buffer.size() - 1) {
            // Go home
            if (customUIModule) {
                customUIModule->getScreenManager()->navigateTo(customUIModule->getHomeScreen());
            }
            return true;
        } else {
            showPrev();
            return true;
        }
    } 
    else if (key == '2') {
        scrollUp();
        return true;
    }
    else if (key == '8') {
        scrollDown();
        return true;
    }
    return false;
}

void MessagesScreen::addMessage(const String& text, const String& sender, unsigned long timestamp) {
    if (buffer.size() >= MAX_MESSAGES) {
        buffer.pop_back(); // Remove oldest
    }
    buffer.insert(buffer.begin(), MessageEntry(text, sender, timestamp));
    
    // Reset view to new message
    currentIndex = 0;
    scrollOffset = 0;
    wrapTextToLines();
    
    // Mark all for redraw
    contentDirty = true;
    headerDirty = true;
    footerDirty = true;
    
    updateNavHint();
    forceRedraw();
}

bool MessagesScreen::hasMessages() const {
    return !buffer.empty();
}

void MessagesScreen::clearMessages() {
    buffer.clear();
    currentIndex = 0;
    scrollOffset = 0;
    totalLines = 0;
    textLines.clear();
    
    // Mark all for redraw (to clear screen)
    contentDirty = true;
    headerDirty = true;
    footerDirty = true;
    
    updateNavHint();
    forceRedraw();
}

void MessagesScreen::showPrev() {
    if (currentIndex < buffer.size() - 1) {
        currentIndex++;
        scrollOffset = 0;
        wrapTextToLines();
        
        // Mark all for redraw
        contentDirty = true;
        headerDirty = true;
        footerDirty = true;
        
        updateNavHint();
        forceRedraw();
    }
}

void MessagesScreen::scrollUp() {
    if (scrollOffset > 0) {
        scrollOffset--;
        contentDirty = true; // Redraw content
        footerDirty = true; // Redraw footer (page numbers)
        // Header unchanged
        forceRedraw();
        updateNavHint();
    }
}

void MessagesScreen::scrollDown() {
    int totalPages = (totalLines + maxVisibleLines - 1) / maxVisibleLines;
    if (scrollOffset < totalPages - 1) {
        scrollOffset++;
        contentDirty = true; // Redraw content
        footerDirty = true; // Redraw footer (page numbers)
        // Header unchanged
        forceRedraw();
        updateNavHint();
    }
}

void MessagesScreen::updateNavHint() {
    std::vector<NavHint> newHints;
    
    // Scroll hints
    if (totalLines > maxVisibleLines) {
        int totalPages = (totalLines + maxVisibleLines - 1) / maxVisibleLines;
        if (scrollOffset > 0) newHints.push_back(NavHint('2', "PgUp"));
        if (scrollOffset < totalPages - 1) newHints.push_back(NavHint('8', "PgDn"));
    }
    
    // Navigation hint
    if (buffer.empty() || buffer.size() == 1 || currentIndex == buffer.size() - 1) {
        newHints.push_back(NavHint('A', "Home"));
    } else {
        newHints.push_back(NavHint('A', "Prev")); 
    }
    
    // Only update if changed
    if (navHints.size() != newHints.size()) {
       setNavigationHints(newHints);
       return;
    }
    for (size_t i = 0; i < navHints.size(); i++) {
        if (navHints[i].key != newHints[i].key || !navHints[i].label.equals(newHints[i].label)) {
            setNavigationHints(newHints);
            return;
        }
    }
}
