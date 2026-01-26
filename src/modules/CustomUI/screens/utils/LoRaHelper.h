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

#include <Arduino.h>
#include <vector>

/**
 * Information about a mesh node for display purposes
 */
struct NodeInfo {
    uint32_t nodeNum;           // Node number
    char longName[32];          // Long name (fixed buffer)
    char shortName[8];          // Short name (fixed buffer)
    uint32_t lastHeard;         // When we last heard from this node (Unix timestamp)
    float snr;                  // Signal-to-noise ratio in dB
    int signalBars;             // Signal strength as bars (0-4)
    bool isOnline;              // Is node considered online
    bool isFavorite;            // Is node marked as favorite
    bool viaInternet;           // Heard via internet/MQTT
    uint8_t hopsAway;           // Number of hops away
    
    NodeInfo() : nodeNum(0), lastHeard(0), snr(0), signalBars(0), 
                 isOnline(false), isFavorite(false), viaInternet(false), hopsAway(0) {
        longName[0] = '\0';
        shortName[0] = '\0';
    }
};

/**
 * Information about a message for display purposes
 */
struct MessageInfo {
    char text[200];             // Message text (increased from 64 to 200 bytes)
    char senderName[32];        // Formatted sender name
    uint32_t timestamp;         // Message timestamp
    uint32_t senderNodeId;      // Sender node ID
    uint32_t toNodeId;          // Destination node ID (for DM detection)
    uint32_t messageId;         // Packet ID / Message ID
    uint8_t channelIndex;       // Channel index for channel messages
    char channelName[16];       // Channel name or identifier
    bool isOutgoing;            // True if sent by us
    bool isDirectMessage;       // True if this is a direct message
    bool ackReceived;           // True if ACK received for this message
    bool isValid;               // True if message data is valid
    
    MessageInfo() : timestamp(0), senderNodeId(0), toNodeId(0), messageId(0), channelIndex(0),
                    isOutgoing(false), isDirectMessage(false), ackReceived(false), isValid(false) {
        text[0] = '\0';
        senderName[0] = '\0';
        channelName[0] = '\0';
    }
};

/**
 * Information about a channel for display purposes
 */
struct ChannelHelperInfo {
    uint8_t index;              // Channel index
    char name[16];              // Channel name
    bool isPrimary;             // Is this the primary channel
    bool isSecondary;           // Is this a secondary channel
    
    ChannelHelperInfo() : index(0), isPrimary(false), isSecondary(false) {
        name[0] = '\0';
    }
};

/**
 * LoRa utility helper for CustomUI screens
 * Provides device name and LoRa status information
 */
class LoRaHelper {
public:
    static void init();
    
    /**
     * Get device long name
     * @return device long name string
     */
    static String getDeviceLongName();
    
    /**
     * Get device short name
     * @return device short name string
     */
    static String getDeviceShortName();
    
    /**
     * Check if device name has changed since last call
     * @return true if device name changed
     */
    static bool hasChanged();
    
    /**
     * Get LoRa signal strength (RSSI)
     * @return RSSI value, 0 if unavailable
     */
    static int getRSSI();
    
    /**
     * Get number of connected nodes
     * @return node count
     */
    static int getNodeCount();
    
    /**
     * Check if LoRa radio is online
     * @return true if LoRa is active
     */
    static bool isLoRaOnline();

    /**
     * Get list of configured channels with names and types
     * @return vector of ChannelHelperInfo
     */
    static std::vector<ChannelHelperInfo> getChannelList();

    /**
     * Get list of mesh nodes with their information
     * @param maxNodes Maximum number of nodes to return (default 15)
     * @param includeOffline Include offline nodes in the list
     * @return vector of NodeInfo structures
     */
    static std::vector<NodeInfo> getNodesList(int maxNodes = 15, bool includeOffline = true);

    /**
     * Convert SNR to signal bars (0-4)
     * @param snr Signal-to-noise ratio in dB
     * @return number of signal bars (0-4)
     */
    static int snrToSignalBars(float snr);

    /**
     * Convert time since last heard to online status
     * @param lastHeard Unix timestamp of last heard
     * @return true if node is considered online
     */
    static bool isNodeOnline(uint32_t lastHeard);

    /**
     * Get the most recent received message from device state
     * @return MessageInfo structure with message data, isValid=false if no message
     */
    static MessageInfo getLastReceivedMessage();

    /**
     * Get list of recent messages (currently only last message)
     * @param maxMessages Maximum number of messages to return
     * @return vector of MessageInfo structures
     */
    static std::vector<MessageInfo> getRecentMessages(int maxMessages = 10);

    /**
     * Format sender name for a node ID
     * @param nodeId Node ID to format
     * @param isOutgoing True if this is an outgoing message
     * @return formatted sender name
     */
    static String formatSenderName(uint32_t nodeId, bool isOutgoing);

    /**
     * Format timestamp as "X time ago"
     * @param timestamp Unix timestamp
     * @return formatted time string
     */
    static String formatTimeAgo(uint32_t timestamp);

    /**
     * Send a text message via LoRa mesh
     * @param messageText Text to send
     * @param toNodeId Destination node ID (UINT32_MAX for broadcast)
     * @param channelIndex Channel to send on (0 = primary)
     * @return Packet ID if message was sent successfully, 0 otherwise
     */
    static uint32_t sendMessage(const String& messageText, uint32_t toNodeId = UINT32_MAX, uint8_t channelIndex = 0);

private:
    static String lastLongName;
    static String lastShortName;
    static bool initialized;
};