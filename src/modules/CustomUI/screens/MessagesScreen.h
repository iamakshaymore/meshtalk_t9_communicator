#pragma once
#include "BaseScreen.h"
#include <vector>
#include <LovyanGFX.hpp>
#include <Arduino.h>

struct MessageEntry {
    String text;
    String sender;
    unsigned long timestamp;
    MessageEntry(const String& t, const String& s, unsigned long ts)
        : text(t), sender(s), timestamp(ts) {}
};

class MessagesScreen : public BaseScreen {
public:
    MessagesScreen();
    virtual ~MessagesScreen();

    virtual void onEnter(const NavigationContext& ctx) override;
    virtual void onExit() override;
    virtual void onDraw(lgfx::LGFX_Device& tft) override;
    bool handleKeyPress(char key) override;

    // Call to update the relative time display (should be called every minute)
    void updateRelativeTime();

    // Add a new message to the buffer
    void addMessage(const String& text, const String& sender, unsigned long timestamp);
    bool hasMessages() const;
    void clearMessages();

private:
    static const int MAX_MESSAGES = 10;
    std::vector<MessageEntry> buffer;
    int currentIndex; // 0 = newest, buffer.size()-1 = oldest

    // Text wrapping and scrolling state
    std::vector<String> textLines;
    int scrollOffset;
    int maxVisibleLines;
    int totalLines;

    // Dirty rect tracking
    bool contentDirty;
    bool headerDirty;
    bool footerDirty;

    // Layout constants
    static const int SENDER_HEIGHT = 30;
    static const int TIMESTAMP_HEIGHT = 25;
    static const int LINE_HEIGHT = 20; // Size 2 text + padding
    // Match MessageDetailsScreen layout logic (approximate CONTENT_HEIGHT as 180 or use macro if available, but assuming 180 from original code)
    // MessageDetailsScreen uses: CONTENT_HEIGHT - SENDER_HEIGHT - TIMESTAMP_HEIGHT - 20
    static const int TEXT_AREA_HEIGHT = 180 - SENDER_HEIGHT - TIMESTAMP_HEIGHT - 20;

    void showPrev();
    void updateNavHint();
    
    // Helper methods for display logic
    void wrapTextToLines();
    void calculateVisibleLines();
    void scrollUp();
    void scrollDown();
};
