#pragma once

#include "screens/BaseScreen.h"
#include <LovyanGFX.hpp>
#include <vector>
#include <functional>

class T9InputScreen; // Forward declaration

class ScreenManager {
public:
    ScreenManager();
    ~ScreenManager();

    // Initialize with display driver
    void init(lgfx::LGFX_Device* tft);

    // Navigate to a new screen, pushing it onto the stack
    void navigateTo(BaseScreen* screen, void* data = nullptr);
    
    // Navigate to T9 Input screen with a specific callback
    void navigateToT9(T9InputScreen* t9Screen, std::function<void(const String&)> onConfirm);

    // Navigate back to the previous screen
    void navigateBack(void* resultData = nullptr);

    /**
     * Show a modal popup message for a specified duration
     * @param message Text to display
     * @param durationMs Duration in milliseconds to show the popup (blocking)
     */
    void showPopup(const String& message, int durationMs = 1000);

    // Get the currently active screen
    BaseScreen* getCurrentScreen();

private:
    std::vector<BaseScreen*> screenStack;
    lgfx::LGFX_Device* tft;
};
