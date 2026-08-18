#pragma once

#include "configuration.h"

#if defined(VARIANT_heltec_v3_custom) || defined(VARIANT_heltec_v4_custom) || defined(MESHTALK_T9)

#include "SinglePortModule.h"
#include "concurrency/OSThread.h"
#include "Observer.h"
#include <LovyanGFX.hpp>
#include <Keypad.h>
#include <Arduino.h>
#include <vector>
#include <memory>
#include "ScreenManager.h"

// Forward declarations
class InitBase;
class InitDisplay;
class InitKeypad;
class BaseScreen;
class HomeScreen;
class MainMenuScreen;
class NodesListScreen;
class MessageListScreen;
class MessageDetailsScreen;
class SnakeGameScreen;
class T9InputScreen;
class MessagesMenuScreen;

#include "screens/MessagesScreen.h"
/**
 * Modular Custom UI Module for external ST7789 display with LovyanGFX
 * Architecture:
 * - Modular initializers in init/ directory (initialization only)
 * - Screen-based UI with BaseScreen abstract class
 * - CustomUIModule handles navigation and input routing
 * - LovyanGFX with automatic PSRAM, DMA, and high-speed SPI (80MHz)
 * - 4x4 keypad for navigation and input
 * - Extensible for future screens and components
 * 
 * Performance targets:
 * - 40-60 FPS with optimized drawing
 * - ~150-220KB free memory
 * - Smooth screen transitions
 */
class MessagesScreen; // Forward declaration

class CustomUIModule : public SinglePortModule, private concurrency::OSThread {
public:
    CustomUIModule();
    virtual ~CustomUIModule();
    
    // Module interface
    virtual int32_t runOnce() override;
    virtual ProcessMessage handleReceived(const meshtastic_MeshPacket &mp) override;
    virtual bool wantPacket(const meshtastic_MeshPacket *p) override;
    virtual bool wantUIFrame() override;
    
    // Initialization
    void initAll();

    // Screen Management
    ScreenManager* getScreenManager() { return &screenManager; }

    // Accessors for Screens
    HomeScreen* getHomeScreen() { return homeScreen; }
    MainMenuScreen* getMenuScreen() { return menuScreen; }
    NodesListScreen* getNodesListScreen() { return nodesListScreen; }
    MessageListScreen* getMessageListScreen() { return messageListScreen; }
    MessageDetailsScreen* getMessageDetailsScreen() { return messageDetailsScreen; }
    MessagesScreen* getMessagesScreen() { return messagesScreen; }
    MessagesMenuScreen* getMessagesMenuScreen() { return messagesMenuScreen; }
    SnakeGameScreen* getSnakeGameScreen() { return snakeGameScreen; }
    T9InputScreen* getT9InputScreen() { return t9InputScreen; }

private:
    // Modular initializers
    std::vector<std::unique_ptr<InitBase>> initializers;
    
    // Component references for easy access
    InitDisplay* displayInit;
    InitKeypad* keypadInit;
    
    bool allInitialized;

    // Screen Manager
    ScreenManager screenManager;
    
    // Display and input handling
    lgfx::LGFX_Device* tft;
    Keypad* keypad;
    
    // Screen management
    BaseScreen* currentScreen;
    HomeScreen* homeScreen;
    MainMenuScreen* menuScreen;
    NodesListScreen* nodesListScreen;
    MessageListScreen* messageListScreen;
    MessageDetailsScreen* messageDetailsScreen;
    MessagesScreen* messagesScreen;
    MessagesMenuScreen* messagesMenuScreen;
    SnakeGameScreen* snakeGameScreen;
    T9InputScreen* t9InputScreen;
    
    // Splash screen animation state
    bool isSplashActive;
    unsigned long splashStartTime;
    int loadingProgress;           // Current progress 0-100
    unsigned long lastProgressUpdate;  // Last time progress was updated
    class InitialSplashScreen* splashScreen; // Splash screen instance
    
    // Display sleep management
    bool displayAsleep;
    bool displayWakeStabilizing;        // True while display stabilizes after wake
    unsigned long displayWakeStabilizeTime; // Time when stabilization started
    unsigned long lastActivityTime;
    
    // LED notification blink state
    enum LedBlinkState {
        LED_IDLE,
        LED_ON_FIRST,
        LED_OFF_MIDDLE,
        LED_ON_SECOND
    };
    LedBlinkState ledState;
    unsigned long ledStateStartTime;
    
    // Helper methods
    void triggerLedBlink();
    void updateLedBlink();
    void registerInitializers();
    void connectComponents();
    void initScreens();
    void showSplashScreen();
    void updateSplashAnimation();  // Update progressive loading animation
    
    // Display power management
    void checkDisplaySleep();
    void sleepDisplay();
    void wakeDisplay();
    void updateLastActivity();
    
    // Deep sleep handling
    int onDeepSleep(void *unused);
    CallbackObserver<CustomUIModule, void *> deepSleepObserver = 
        CallbackObserver<CustomUIModule, void *>(this, &CustomUIModule::onDeepSleep);
    
    // Light sleep handling (keypad wake source setup/cleanup)
    int onLightSleep(void *unused);
    int onLightSleepEnd(esp_sleep_wakeup_cause_t cause);
    CallbackObserver<CustomUIModule, void *> lightSleepObserver = 
        CallbackObserver<CustomUIModule, void *>(this, &CustomUIModule::onLightSleep);
    CallbackObserver<CustomUIModule, esp_sleep_wakeup_cause_t> lightSleepEndObserver = 
        CallbackObserver<CustomUIModule, esp_sleep_wakeup_cause_t>(this, &CustomUIModule::onLightSleepEnd);
    
    // Input handling
    void checkKeypadInput();
    void handleKeyPress(char key);
};

// Global setup function
void setup_CustomUIModule();

// Global instance
extern CustomUIModule *customUIModule;

#endif