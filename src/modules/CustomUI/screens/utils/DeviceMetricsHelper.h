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

/**
 * Device metrics utility helper for CustomUI screens
 * Provides memory utilization and system performance information
 */
class DeviceMetricsHelper {
public:
    static void init();
    
    /**
     * Get current free heap memory in bytes
     * @return free heap size in bytes
     */
    static size_t getFreeHeap();
    
    /**
     * Get total heap size in bytes
     * @return total heap size in bytes
     */
    static size_t getTotalHeap();
    
    /**
     * Get memory utilization percentage (0-100)
     * @return memory usage percentage
     */
    static int getMemoryUtilization();
    
    /**
     * Check if memory metrics have changed significantly since last call
     * @return true if memory info changed
     */
    static bool hasChanged();
    
    /**
     * Get formatted memory utilization string for display
     * @return formatted string like "65%" or "Free: 32KB"
     */
    static String getMemoryString();
    
    /**
     * Get detailed memory information string
     * @return detailed string like "Used: 128KB/256KB (50%)"
     */
    static String getDetailedMemoryString();
    
    /**
     * Get minimum free heap size since boot
     * @return minimum free heap in bytes
     */
    static size_t getMinFreeHeap();
    
    /**
     * Get SRAM free memory in bytes (excluding PSRAM)
     * @return free SRAM size in bytes
     */
    static size_t getSramFree();
    
    /**
     * Get SRAM total size in bytes (excluding PSRAM)
     * @return total SRAM size in bytes
     */
    static size_t getSramTotal();
    
    /**
     * Get SRAM utilization percentage (0-100)
     * @return SRAM usage percentage
     */
    static int getSramUtilization();
    
    /**
     * Get PSRAM free memory in bytes
     * @return free PSRAM size in bytes (0 if no PSRAM)
     */
    static size_t getPsramFree();
    
    /**
     * Get PSRAM total size in bytes
     * @return total PSRAM size in bytes (0 if no PSRAM)
     */
    static size_t getPsramTotal();
    
    /**
     * Get PSRAM utilization percentage (0-100)
     * @return PSRAM usage percentage (0 if no PSRAM)
     */
    static int getPsramUtilization();
    
    /**
     * Check if device has PSRAM available
     * @return true if PSRAM is available
     */
    static bool hasPsram();
    
    /**
     * Get separate SRAM and PSRAM memory strings
     * @return formatted string like "SRAM: 65%\nPSRAM: 42%" or just "SRAM: 65%"
     */
    static String getSeparateMemoryString();

private:
    static size_t lastFreeHeap;
    static int lastMemoryPercent;
    static int lastSramPercent;
    static int lastPsramPercent;
    static size_t minFreeHeapSeen;
    static bool initialized;
    
    // Threshold for considering memory change significant (in bytes)
    static const size_t MEMORY_CHANGE_THRESHOLD = 1024;
};