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

#include <WiFi.h>
#include <vector>
#include <String.h>

/**
 * WiFi Helper for network scanning and management
 * Handles WiFi operations for CustomUI module
 */

struct WiFiNetworkInfo {
    char ssid[33];          // Fixed buffer for SSID (max 32 chars + null)
    char security[8];       // Fixed buffer for security type
    int32_t rssi;
    uint8_t channel;
    bool isOpen;
    
    WiFiNetworkInfo() : rssi(0), channel(0), isOpen(false) {
        ssid[0] = '\0';
        security[0] = '\0';
    }
    
    WiFiNetworkInfo(const String& s, const String& sec, int32_t r, uint8_t ch, bool open) 
        : rssi(r), channel(ch), isOpen(open) {
        strncpy(ssid, s.c_str(), sizeof(ssid) - 1);
        ssid[sizeof(ssid) - 1] = '\0';
        strncpy(security, sec.c_str(), sizeof(security) - 1);
        security[sizeof(security) - 1] = '\0';
    }
};

class WiFiHelper {
public:
    WiFiHelper();
    ~WiFiHelper();
    
    /**
     * Scan for available WiFi networks
     * @param maxNetworks Maximum number of networks to return (default 15)
     * @return Vector of WiFi network information, sorted by signal strength
     */
    std::vector<WiFiNetworkInfo> scanNetworks(int maxNetworks = 15);
    
    /**
     * Start async WiFi scan (non-blocking)
     */
    void startAsyncScan();
    
    /**
     * Check if async scan is complete
     * @return true if scan is finished
     */
    bool isScanComplete();
    
    /**
     * Get results from completed async scan
     * @param maxNetworks Maximum number of networks to return
     * @return Vector of WiFi network information, sorted by signal strength
     */
    std::vector<WiFiNetworkInfo> getAsyncScanResults(int maxNetworks = 15);
    
    /**
     * Get signal strength description
     * @param rssi Signal strength in dBm
     * @return Signal strength as text (Excellent/Good/Fair/Weak)
     */
    String getSignalStrength(int32_t rssi);
    
    /**
     * Get signal strength bars (1-4)
     * @param rssi Signal strength in dBm
     * @return Number of bars (1-4)
     */
    int getSignalBars(int32_t rssi);
    
    /**
     * Get security type as readable string
     * @param authMode WiFi authentication mode
     * @return Security type string
     */
    String getSecurityType(wifi_auth_mode_t authMode);
    
    /**
     * Get security type as C-string (no heap allocation)
     * @param authMode WiFi authentication mode
     * @return Security type as const char*
     */
    const char* getSecurityTypeCStr(wifi_auth_mode_t authMode);
    
    /**
     * Check if WiFi is currently connected
     * @return true if connected
     */
    bool isConnected();
    
    /**
     * Get current connected SSID
     * @return SSID name or empty string if not connected
     */
    String getCurrentSSID();
    
    /**
     * Get current IP address
     * @return IP address as string or empty if not connected
     */
    String getCurrentIP();

private:
    /**
     * Sort networks by signal strength (strongest first)
     */
    void sortNetworksBySignal(std::vector<WiFiNetworkInfo>& networks);
    
    /**
     * Process scan results into WiFiNetworkInfo vector
     */
    std::vector<WiFiNetworkInfo> processNetworks(int networkCount, int maxNetworks);
    
    /**
     * Compare networks by signal strength (for sorting)
     */
    static bool compareBySignalStrength(const WiFiNetworkInfo& a, const WiFiNetworkInfo& b);
    
    bool asyncScanInProgress = false;
    
    /**
     * Remove duplicate SSIDs, keeping the strongest signal
     */
    void removeDuplicates(std::vector<WiFiNetworkInfo>& networks);
};