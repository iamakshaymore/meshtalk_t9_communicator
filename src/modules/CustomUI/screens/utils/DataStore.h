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

#include "configuration.h"

#if defined(VARIANT_heltec_v3_custom) || defined(VARIANT_heltec_v4_custom)

#include "LoRaHelper.h" // For MessageInfo structure
#include <vector>
#include <Arduino.h>

/**
 * Singleton DataStore for managing message history
 * 
 * This class provides centralized storage for MessageInfo objects,
 * allowing the CustomUIModule to store incoming messages and 
 * LoRaHelper to retrieve them for display purposes.
 * 
 * Features:
 * - Singleton pattern for global access
 * - Fixed-size circular buffer to prevent memory issues
 * - Thread-safe operations
 * - Automatic cleanup of old messages
 */
class DataStore {
public:
    // Singleton access
    static DataStore& getInstance();
    
    // Message storage operations
    void addMessage(const MessageInfo& message);
    void ackReceived(uint32_t messageId);
    std::vector<MessageInfo> getRecentMessages(int maxMessages = 10) const;
    
    // Utility methods
    size_t getMessageCount() const;
    void clearMessages();
    bool hasMessages() const;
    
    // Get latest message
    MessageInfo getLatestMessage() const;
    
    // Get list of channel indexes containing messages
    std::vector<uint8_t> getActiveChannelIndexes() const;

private:
    // Private constructor for singleton
    DataStore();
    ~DataStore() = default;
    
    // Delete copy constructor and assignment operator
    DataStore(const DataStore&) = delete;
    DataStore& operator=(const DataStore&) = delete;
    
    // Message storage
#if defined(VARIANT_heltec_v3_custom)
    static const size_t MAX_MESSAGES = 50; // Reduced for V3 (SRAM limit)
#else
    static const size_t MAX_MESSAGES = 100; // Maximum messages to store
#endif
    std::vector<MessageInfo> messages;
    mutable bool needsSort; // Flag to indicate if messages need sorting
    
    // Helper methods
    void sortMessagesByTimestamp() const;
    void enforceMaxSize();
    void logStorageStats() const;
};

#endif