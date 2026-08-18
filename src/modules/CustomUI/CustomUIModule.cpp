/**
 * Modular Custom UI Module for External ST7789 Display with LovyanGFX
 * Uses modular architecture with separate initializers and screen-based UI
 * Only compiles when building the heltec-v3-custom variant
 */

#include "configuration.h"

#if defined(VARIANT_heltec_v3_custom) || defined(VARIANT_heltec_v4_custom) || defined(MESHTALK_T9)

#include "CustomUIModule.h"
#include "DebugConfiguration.h"
#include "PowerFSM.h"
#include "Default.h"
#include "init/InitBase.h"
#include "init/InitDisplay.h"
#include "init/InitKeypad.h"
#include "screens/BaseScreen.h"
#include "screens/HomeScreen.h"
#include "screens/menu_screens/MainMenuScreen.h"
#include "screens/menu_screens/MessagesMenuScreen.h"
#include "screens/list_screens/NodesListScreen.h"
#include "screens/list_screens/MessageListScreen.h"
#include "screens/MessageDetailsScreen.h"
#include "screens/MessagesScreen.h"
#include "screens/SnakeGameScreen.h"
#include "screens/T9InputScreen.h"
#include "InitialSplashScreen.h"
#include "screens/utils/DataStore.h"
#include "screens/utils/LoRaHelper.h"
#include "sleep.h"
#include <LovyanGFX.hpp>
#include <Arduino.h>
#include <pb_decode.h>

#ifdef ESP32
#include <esp_heap_caps.h>
#include <esp_heap_caps_init.h>
#include <esp_sleep.h>
#include <driver/gpio.h>
#endif
#include <RTC.h>

CustomUIModule *customUIModule;

CustomUIModule::CustomUIModule() 
    : SinglePortModule("CustomUIModule", meshtastic_PortNum_TEXT_MESSAGE_APP),
      OSThread("CustomUIModule"),
      displayInit(nullptr),
      keypadInit(nullptr),
      allInitialized(false),
      tft(nullptr),
      keypad(nullptr),
      currentScreen(nullptr),
      homeScreen(nullptr),
      menuScreen(nullptr),
      nodesListScreen(nullptr),
      messageListScreen(nullptr),
      messageDetailsScreen(nullptr),
      messagesScreen(nullptr),
      messagesMenuScreen(nullptr),
      snakeGameScreen(nullptr),
      t9InputScreen(nullptr),
      isSplashActive(false),
      splashStartTime(0),
      loadingProgress(0),
      lastProgressUpdate(0),
      splashScreen(nullptr),
      displayAsleep(false),
      displayWakeStabilizing(false),
      displayWakeStabilizeTime(0),
      lastActivityTime(0),
      ledState(LED_IDLE),
      ledStateStartTime(0) {
    
    pinMode(17, OUTPUT);
    digitalWrite(17, LOW); // Ensure LED starts OFF 
    LOG_INFO("🔧 CUSTOM UI: Module constructed with screen-based architecture");
    registerInitializers();
    
    // Register for sleep notifications
    deepSleepObserver.observe(&notifyDeepSleep);
    lightSleepObserver.observe(&notifyLightSleep);
    lightSleepEndObserver.observe(&notifyLightSleepEnd);
    LOG_INFO("🔧 CUSTOM UI: Registered sleep observers (deep + light)");
}

CustomUIModule::~CustomUIModule() {
    // Unregister deep sleep observer
    deepSleepObserver.unobserve(&notifyDeepSleep);
    
    // Cleanup splash screen
    if (splashScreen) {
        delete splashScreen;
        splashScreen = nullptr;
    }
    
    // Cleanup screens
    if (homeScreen) {
        delete homeScreen;
        homeScreen = nullptr;
    }

    if (menuScreen) {
        delete menuScreen;
        menuScreen = nullptr;
    }

    if (messagesMenuScreen) {
        delete messagesMenuScreen;
        messagesMenuScreen = nullptr;
    }
    
    if (nodesListScreen) {
        delete nodesListScreen;
        nodesListScreen = nullptr;
    }
    
    if (messageListScreen) {
        delete messageListScreen;
        messageListScreen = nullptr;
    }
    
    if (messageDetailsScreen) {
        delete messageDetailsScreen;
        messageDetailsScreen = nullptr;
    }
    
    if (snakeGameScreen) {
        delete snakeGameScreen;
        snakeGameScreen = nullptr;
    }
    
    if (t9InputScreen) {
        delete t9InputScreen;
        t9InputScreen = nullptr;
    }
    
    // Cleanup all initializers
    for (auto& init : initializers) {
        init->cleanup();
    }
    initializers.clear();
}

void CustomUIModule::registerInitializers() {
    LOG_INFO("🔧 CUSTOM UI: Registering initializers...");
    
    // Register display initializer
    std::unique_ptr<InitDisplay> display(new InitDisplay());
    displayInit = display.get(); // Keep reference for easy access
    initializers.push_back(std::move(display));
    
    // Register keypad initializer
    std::unique_ptr<InitKeypad> keypadInitPtr(new InitKeypad());
    keypadInit = keypadInitPtr.get(); // Keep reference for easy access
    initializers.push_back(std::move(keypadInitPtr));
    
    // Future initializers can be added here:
    // initializers.push_back(std::unique_ptr<InitWiFi>(new InitWiFi()));
    // initializers.push_back(std::unique_ptr<InitBluetooth>(new InitBluetooth()));
    
    LOG_INFO("🔧 CUSTOM UI: Registered %d initializers", initializers.size());
}

void CustomUIModule::initAll() {
    LOG_INFO("🔧 CUSTOM UI: Starting initialization sequence...");
    
    bool allSuccess = true;
    
    // Initialize all components in order
    for (auto& init : initializers) {
        LOG_INFO("🔧 CUSTOM UI: Initializing %s...", init->getName());
        
        if (!init->init()) {
            LOG_ERROR("🔧 CUSTOM UI: Failed to initialize %s", init->getName());
            allSuccess = false;
        } else {
            LOG_INFO("🔧 CUSTOM UI: ✅ %s initialized successfully", init->getName());
        }
    }
    
    if (allSuccess) {
        // Connect components after all are initialized
        connectComponents();
        
        // Initialize screens
        initScreens();
        
        // Set initial activity time
        updateLastActivity();
        
        allInitialized = true;
        LOG_INFO("🔧 CUSTOM UI: ✅ All initializers and screens completed successfully");
    } else {
        LOG_ERROR("🔧 CUSTOM UI: ❌ Some initializers failed");
    }
}

void CustomUIModule::connectComponents() {
    LOG_INFO("🔧 CUSTOM UI: Connecting components...");
    
    // Get direct access to initialized components for logic handling
    if (displayInit && displayInit->isReady()) {
        tft = displayInit->getDisplay();
        LOG_INFO("🔧 CUSTOM UI: Display connected");
        
        // Report current memory status with PSRAM info
        LOG_INFO("🔧 CUSTOM UI: Post-display Memory Status:");
        LOG_INFO("🔧 CUSTOM UI: - Free Heap: %zu bytes (%.1fKB)", ESP.getFreeHeap(), ESP.getFreeHeap()/1024.0);
        
#if defined(CONFIG_SPIRAM_SUPPORT) && defined(BOARD_HAS_PSRAM)
        size_t psramSize = ESP.getPsramSize();
        if (psramSize > 0) {
            size_t freePsram = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
            LOG_INFO("🔧 CUSTOM UI: - PSRAM Total: %zu bytes (%.1fMB)", psramSize, psramSize/(1024.0*1024.0));
            LOG_INFO("🔧 CUSTOM UI: - PSRAM Free: %zu bytes (%.1fMB)", freePsram, freePsram/(1024.0*1024.0));
            LOG_INFO("🔧 CUSTOM UI: ✅ PSRAM available for graphics");
        } else {
            LOG_INFO("🔧 CUSTOM UI: ⚠️  No PSRAM detected");
        }
#else
        LOG_INFO("🔧 CUSTOM UI: ⚠️  PSRAM support not compiled in");
#endif
        
        // Show splash screen with progressive animation
        showSplashScreen();
    }
    
    if (keypadInit && keypadInit->isReady()) {
        keypad = keypadInit->getKeypad();
        LOG_INFO("🔧 CUSTOM UI: Keypad connected");
    }
    
    // Initialize ScreenManager with display capability
    if (tft) {
        screenManager.init(tft);
    }
}

void CustomUIModule::showSplashScreen() {
    if (!tft) return;
    
    LOG_INFO("🔧 CUSTOM UI: Starting progressive loading animation");
    
    // Create splash screen instance
    splashScreen = new InitialSplashScreen();
    
    // Initialize the splash screen (title and progress bar setup)
    splashScreen->playAnimation(tft);
    
    // Initialize animation state
    loadingProgress = 0;
    lastProgressUpdate = millis();
    isSplashActive = true;
    
    LOG_INFO("🔧 CUSTOM UI: Progressive loading animation initialized");
}

void CustomUIModule::initScreens() {
    LOG_INFO("🔧 CUSTOM UI: Initializing screens...");
    
    // Create home screen
    homeScreen = new HomeScreen();

    // Create menu screen
    menuScreen = new MainMenuScreen();

    // Create messages menu screen
    messagesMenuScreen = new MessagesMenuScreen();

    // Create nodes list screen
    nodesListScreen = new NodesListScreen();

    // Create message list screen
    messageListScreen = new MessageListScreen();

    // Create message details screen
    messageDetailsScreen = new MessageDetailsScreen();

    // Create messages screen
    messagesScreen = new MessagesScreen();
    
    // Create snake game screen
    snakeGameScreen = new SnakeGameScreen();

    // Create T9 input screen
    t9InputScreen = new T9InputScreen();
    // Default callback - specific screens should override this via ScreenManager::navigateToT9
    t9InputScreen->setConfirmCallback([](const String& text) {
        LOG_WARN("🔧 CUSTOM UI: T9 input confirmed but no handler set");
    });

    // Screens are ready but don't switch yet - animation will handle transition

    LOG_INFO("🔧 CUSTOM UI: ✅ Screens created, animation will handle transition");
}

int32_t CustomUIModule::runOnce() {
    if (!allInitialized) {
        return 1000; // Wait 1 second if not initialized
    }
    
    // Handle progressive splash screen animation
    if (isSplashActive && tft && splashScreen) {
        updateSplashAnimation();
        
        // Check if animation is complete
        if (splashScreen->isAnimationComplete()) {
            LOG_INFO("🔧 CUSTOM UI: Animation complete, transitioning to Home screen");
            isSplashActive = false;
            
            // Clean up splash screen
            delete splashScreen;
            splashScreen = nullptr;
            
            // Switch to home screen
            if (homeScreen) {
                // Use the screen manager directly
                screenManager.navigateTo(homeScreen);
            }
        }
        
        return 20; // Update every 20ms for smooth animation and responsive input
    }
    
    // Get current screen from manager
    currentScreen = screenManager.getCurrentScreen();
    
    // Handle display wake stabilization (non-blocking alternative to delay(50) in wakeDisplay)
    if (displayWakeStabilizing) {
        if (millis() - displayWakeStabilizeTime >= 50) {
            displayWakeStabilizing = false;
            LOG_DEBUG("🔧 CUSTOM UI: Display wake stabilized");
        } else {
            return 10; // Check again soon
        }
    }
    
    if (!currentScreen || !tft) {
        return 1000; // Wait 1 second if no screen ready
    }
    
    // Handle keypad input first (needed to wake display)
    checkKeypadInput();

    // Refresh current screen in case navigation happened
    currentScreen = screenManager.getCurrentScreen();
    // Re-check validity after potential navigation
    if (!currentScreen) return 1000;
    
    // Update LED blink animation (non-blocking)
    updateLedBlink();
    
    // Check for display sleep timeout
    checkDisplaySleep();
    
    // Skip UI updates if display is asleep
    // if (displayAsleep) {
    //     return 1000; // Check for wake conditions every second
    // }
    
    // Update current screen if needed
    if (currentScreen->needsUpdate()) {
        currentScreen->draw(*tft);
    }
    
    return 20; // 50 FPS update rate for responsive input and smooth UI
}

bool CustomUIModule::wantUIFrame() {
    return false; // We don't want to integrate with the main UI
}


bool CustomUIModule::wantPacket(const meshtastic_MeshPacket *p) {
    return p->decoded.portnum == meshtastic_PortNum_TEXT_MESSAGE_APP || 
           p->decoded.portnum == meshtastic_PortNum_ROUTING_APP;
}

// Handle incoming LoRa messages and show MessagesScreen
ProcessMessage CustomUIModule::handleReceived(const meshtastic_MeshPacket &mp) {
    // Handle ACK messages (ROUTING_APP with Error::NONE)
    if (mp.decoded.portnum == meshtastic_PortNum_ROUTING_APP) {
        meshtastic_Routing routingMsg;
        LOG_INFO("🔧 CUSTOM UI: Decoding ROUTING_APP message from node %08X", mp.from);
        // Decode the routing payload
        pb_istream_t stream = pb_istream_from_buffer(mp.decoded.payload.bytes, mp.decoded.payload.size);
        if (pb_decode(&stream, meshtastic_Routing_fields, &routingMsg)) {
             // Check if it's an ACK (Success error code)
             if (routingMsg.which_variant == meshtastic_Routing_error_reason_tag && 
                 routingMsg.error_reason == meshtastic_Routing_Error_NONE) {
                 
                 // The ACKed message ID is in the Data packet's request_id field
                 uint32_t ackedMessageId = mp.decoded.request_id;
                 
                 if (ackedMessageId != 0) {
                     LOG_INFO("ACK received for message ID %u", ackedMessageId);
                     DataStore::getInstance().ackReceived(ackedMessageId);
                 }
             }
        }
        return ProcessMessage::CONTINUE;
    }

    // Only handle text messages (TEXT_MESSAGE_APP)
    if (mp.decoded.portnum == meshtastic_PortNum_TEXT_MESSAGE_APP) {
        // Ignore messages from self
        if (nodeDB && mp.from == nodeDB->getNodeNum()) {
            return ProcessMessage::CONTINUE; 
        }

        // Wake display if asleep
        if (displayAsleep) {
            wakeDisplay();
        }
        
        // Update activity time
        updateLastActivity();
        
        // Extract text from payload (payload.bytes is not null-terminated)
        const meshtastic_Data_payload_t &payload = mp.decoded.payload;
        String text;
        if (payload.size > 0) {
            text = String(reinterpret_cast<const char *>(payload.bytes), payload.size);
        }
        
        // Try to get sender long name from NodeDB
        String sender;
        if (nodeDB) {
            meshtastic_NodeInfoLite *info = nodeDB->getMeshNode(mp.from);
            if (info && info->long_name[0] != '\0') {
                sender = String(info->long_name);
            } else {
                char senderBuf[12];
                snprintf(senderBuf, sizeof(senderBuf), "%08X", mp.from);
                sender = String(senderBuf);
            }
        } else {
            char senderBuf[12];
            snprintf(senderBuf, sizeof(senderBuf), "%08X", mp.from);
            sender = String(senderBuf);
        }
        
        // Create MessageInfo and store in DataStore
        if (text.length() > 0) {
            MessageInfo messageInfo;
            
            // Copy message text (truncate if too long)
            size_t textLen = std::min((size_t)text.length(), sizeof(messageInfo.text) - 1);
            memcpy(messageInfo.text, text.c_str(), textLen);
            messageInfo.text[textLen] = '\0';
            
            // Copy sender name
            strncpy(messageInfo.senderName, sender.c_str(), sizeof(messageInfo.senderName) - 1);
            messageInfo.senderName[sizeof(messageInfo.senderName) - 1] = '\0';
            
            // Set message properties
            uint32_t now = getTime();
            if (now == 0) now = millis() / 1000;
            messageInfo.timestamp = now;
            //messageInfo.timestamp = mp.rx_time > 0 ? mp.rx_time : now;
            messageInfo.senderNodeId = mp.from;
            messageInfo.toNodeId = mp.to;
            messageInfo.messageId = mp.id;
            messageInfo.channelIndex = mp.channel;
            messageInfo.isOutgoing = (nodeDB && mp.from == nodeDB->getNodeNum());
            
            // Determine if this is a direct message
            messageInfo.isDirectMessage = (nodeDB && mp.to == nodeDB->getNodeNum() && mp.to != NODENUM_BROADCAST);
            
            // Format channel name
            if (messageInfo.isDirectMessage) {
                strcpy(messageInfo.channelName, "DM");
            } else if (messageInfo.channelIndex == 0) {
                strcpy(messageInfo.channelName, "Primary");
            } else {
                snprintf(messageInfo.channelName, sizeof(messageInfo.channelName), "CH%d", messageInfo.channelIndex);
            }
            
            messageInfo.isValid = true;
            
            // Store message in DataStore
            DataStore::getInstance().addMessage(messageInfo);
            
            // Trigger non-blocking LED blink notification
            triggerLedBlink();
            
            // Show message on MessagesScreen
            if (messagesScreen) {
                unsigned long timestamp = millis();
                messagesScreen->addMessage(text, sender, timestamp);
                screenManager.navigateTo(messagesScreen);
            }
        }
    }
    return ProcessMessage::CONTINUE;
}

void setup_CustomUIModule() {
    if (!customUIModule) {
        customUIModule = new CustomUIModule();
        customUIModule->initAll();
    }
}

void CustomUIModule::updateSplashAnimation() {
    if (!splashScreen || !tft) {
        return;
    }
    
    unsigned long currentTime = millis();
    
    // Update progress every 30ms for smooth animation (about 33 FPS)
    if (currentTime - lastProgressUpdate >= 30) {
        loadingProgress += 2; // Increment by 2% each update
        
        // Ensure we don't exceed 100%
        if (loadingProgress > 100) {
            loadingProgress = 100;
        }
        
        // Update the splash screen with current progress
        splashScreen->updateLoadingProgress(tft, loadingProgress);
        
        lastProgressUpdate = currentTime;
        
        // Log progress for debugging (every 20%)
        if (loadingProgress % 20 == 0) {
            LOG_INFO("🔧 CUSTOM UI: Loading progress: %d%%", loadingProgress);
        }
    }
}

// ========== Input Handling Methods ==========
void CustomUIModule::checkKeypadInput() {
    if (!keypad) return;
    
    char key = keypad->getKey();
    
    if (key) {
        // Reset PowerFSM idle timer on every keypress (not just when display asleep)
        // This ensures screen_on_secs/ls_secs timers respect real T9 usage
        powerFSM.trigger(EVENT_INPUT);
        
        LOG_INFO("🔧 CUSTOM UI: Keypad key pressed: %c (display asleep: %s)", key, displayAsleep ? "YES" : "NO");
        
        // Wake display if asleep
        if (displayAsleep) {
            wakeDisplay();
            return; // First keypress just wakes display
        }
        
        // Update activity time and handle key
        updateLastActivity();
        handleKeyPress(key);
    }
}

void CustomUIModule::handleKeyPress(char key) {
    if (!currentScreen) return;

    // Let current screen handle the key first
    if (currentScreen->handleKeyPress(key)) {
        return; // Screen handled the key
    }
    
    // Fallback: If screen didn't handle it, we rely on individual screens 
    // to implement their own navigation logic now.
    // If any global hotkeys are absolutely needed that apply to ALL screens
    // and weren't handled, they could go here.
}

// ========== Display Power Management ==========
void CustomUIModule::checkDisplaySleep() {
    if (displayAsleep || !tft) {
        return;
    }
    
    unsigned long currentTime = millis();
    unsigned long timeSinceActivity = currentTime - lastActivityTime;
    
    // Use configurable screen_on_secs timeout instead of hardcoded 30s
    // This makes CustomUI consistent with the rest of the firmware's power management
    uint32_t sleepTimeoutMs = Default::getConfiguredOrDefaultMs(config.display.screen_on_secs, default_screen_on_secs);
    
    // Check if timeout exceeded
    if (timeSinceActivity >= sleepTimeoutMs) {
        LOG_INFO("🔧 CUSTOM UI: Display sleep timeout reached (%lu ms since last activity, configured: %lu ms)", 
                 timeSinceActivity, sleepTimeoutMs);
        sleepDisplay();
    }
}

void CustomUIModule::sleepDisplay() {
    if (displayAsleep || !tft) {
        return;
    }
    
    LOG_INFO("🔧 CUSTOM UI: Putting display to sleep after inactivity");
    
    // Turn off display using LovyanGFX sleep function
    tft->sleep();
    displayAsleep = true;
}

void CustomUIModule::wakeDisplay() {
    if (!displayAsleep || !tft) {
        return;
    }
    
    LOG_INFO("🔧 CUSTOM UI: Waking display from activity");
    
    // Wake up display using LovyanGFX wakeup function
    tft->wakeup();
    
    // Set non-blocking stabilization state instead of delay(50)
    // Checked in runOnce() to avoid blocking mesh packet path
    displayWakeStabilizing = true;
    displayWakeStabilizeTime = millis();
    
    displayAsleep = false;
    
    // Update activity time
    updateLastActivity();
    
    //Not required as wake already handles drawing
    // Force complete screen refresh with proper state restoration
    // if (currentScreen) {
    //     // Clear screen first
    //     tft->fillScreen(0x0000);
        
        
    //     // Force full redraw and render immediately
    //     currentScreen->forceFullRedraw();
    //     currentScreen->draw(*tft);
    // }
    
    LOG_INFO("🔧 CUSTOM UI: Display awakened, screen state restored and refreshed");
}

void CustomUIModule::updateLastActivity() {
    lastActivityTime = millis();
}

// ========== Deep Sleep Cleanup ==========
int CustomUIModule::onDeepSleep(void *unused) {
    LOG_INFO("🔧 CUSTOM UI: Preparing for deep sleep - cleaning up display");
    
    if(displayAsleep){
        wakeDisplay();
    }

    // Force any pending display operations to complete
    if (tft) {
        tft->waitDisplay();
        
        // Show a shutdown message briefly
        tft->fillScreen(0x0000);
        tft->setTextColor(0xFFFF); // White text
        tft->setTextSize(2);
        tft->setCursor(80, 110);
        tft->print("Sleeping...");
        delay(500); // Show message briefly
        
        // Put display into sleep mode
        tft->sleep();
        LOG_INFO("🔧 CUSTOM UI: Display put to sleep");
    }
    
    // Mark display as asleep
    displayAsleep = true;
    
    // Cleanup all initializers properly
    for (auto& init : initializers) {
        if (init) {
            init->cleanup();
            LOG_INFO("🔧 CUSTOM UI: Cleaned up %s", init->getName());
        }
    }
    
    LOG_INFO("🔧 CUSTOM UI: Deep sleep cleanup completed");
    return 0; // Allow deep sleep to proceed
}

// ========== Light Sleep Handlers ==========
int CustomUIModule::onLightSleep(void *unused) {
#if defined(MESHTALK_T9) && defined(ESP32)
    // Matrix keypad wake source setup
    // Drive all row pins LOW so any keypress pulls a column pin HIGH
    // Then configure column pins as GPIO wake sources
    static const byte keypadRowPins[] = KEYPAD_ROW_PINS;
    static const byte keypadColPins[] = KEYPAD_COL_PINS;
    
    for (int i = 0; i < KEYPAD_ROW_COUNT; i++) {
        pinMode(keypadRowPins[i], OUTPUT);
        digitalWrite(keypadRowPins[i], LOW);
    }
    
    // Enable GPIO wakeup on column pins (active HIGH when row is LOW and key is pressed)
    for (int i = 0; i < KEYPAD_COL_COUNT; i++) {
        gpio_wakeup_enable((gpio_num_t)keypadColPins[i], GPIO_INTR_HIGH_LEVEL);
    }
    
    LOG_DEBUG("🔧 CUSTOM UI: Enabled matrix keypad wake source (rows LOW, columns wake on HIGH)");
#endif
    return 0;
}

int CustomUIModule::onLightSleepEnd(esp_sleep_wakeup_cause_t cause) {
#if defined(MESHTALK_T9) && defined(ESP32)
    // Disable matrix keypad wake sources and restore normal operation
    static const byte keypadRowPins[] = KEYPAD_ROW_PINS;
    static const byte keypadColPins[] = KEYPAD_COL_PINS;
    
    for (int i = 0; i < KEYPAD_COL_COUNT; i++) {
        gpio_wakeup_disable((gpio_num_t)keypadColPins[i]);
    }
    
    // Restore row pins to normal Keypad library control (set as inputs with pullup)
    // The Keypad library will handle them from here
    for (int i = 0; i < KEYPAD_ROW_COUNT; i++) {
        pinMode(keypadRowPins[i], INPUT_PULLUP);
    }
    
    LOG_DEBUG("🔧 CUSTOM UI: Disabled matrix keypad wake sources, restored normal scanning");
#endif
    return 0;
}

// ========== Non-blocking LED Blink ==========
void CustomUIModule::triggerLedBlink() {
    ledState = LED_ON_FIRST;
    ledStateStartTime = millis();
    digitalWrite(17, HIGH);
    LOG_INFO("🔧 CUSTOM UI: LED blink triggered");
}

void CustomUIModule::updateLedBlink() {
    if (ledState == LED_IDLE) return;
    
    unsigned long elapsed = millis() - ledStateStartTime;
    
    switch (ledState) {
        case LED_ON_FIRST:
            if (elapsed >= 50) { // 0.3s
                digitalWrite(17, LOW);
                ledState = LED_OFF_MIDDLE;
                ledStateStartTime = millis();
            }
            break;
            
        case LED_OFF_MIDDLE:
            if (elapsed >= 25) { // 0.1s
                digitalWrite(17, HIGH);
                ledState = LED_ON_SECOND;
                ledStateStartTime = millis();
            }
            break;
            
        case LED_ON_SECOND:
            if (elapsed >= 50) { // 0.3s
                digitalWrite(17, LOW);
                ledState = LED_IDLE;
                LOG_INFO("🔧 CUSTOM UI: LED blink complete");
            }
            break;
            
        default:
            ledState = LED_IDLE;
            digitalWrite(17, LOW);
            break;
    }
}

#endif