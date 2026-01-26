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

