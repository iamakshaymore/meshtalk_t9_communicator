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

#include "DeviceMetricsHelper.h"
#include "configuration.h"

#ifdef ARCH_ESP32
#include <esp_heap_caps.h>
#endif

// Static member initialization
size_t DeviceMetricsHelper::lastFreeHeap = 0;
int DeviceMetricsHelper::lastMemoryPercent = -1;
int DeviceMetricsHelper::lastSramPercent = -1;
int DeviceMetricsHelper::lastPsramPercent = -1;
size_t DeviceMetricsHelper::minFreeHeapSeen = SIZE_MAX;
bool DeviceMetricsHelper::initialized = false;

void DeviceMetricsHelper::init() {
    if (initialized) return;
    
    lastFreeHeap = getFreeHeap();
    lastMemoryPercent = getMemoryUtilization();
    lastSramPercent = getSramUtilization();
    lastPsramPercent = getPsramUtilization();
    minFreeHeapSeen = lastFreeHeap;
    initialized = true;
    
    LOG_INFO("DeviceMetricsHelper initialized - Free heap: %zu bytes, SRAM: %d%%, PSRAM: %d%%", 
             lastFreeHeap, lastSramPercent, lastPsramPercent);
}

size_t DeviceMetricsHelper::getFreeHeap() {
#ifdef ARCH_ESP32
    size_t freeHeap = ESP.getFreeHeap();
    
#if defined(CONFIG_SPIRAM_SUPPORT) && defined(BOARD_HAS_PSRAM)
    // Add free PSRAM to total free memory if present
    if (ESP.getPsramSize() > 0) {
        freeHeap += ESP.getFreePsram();
    }
#endif
    
    return freeHeap;
#else
    // For other architectures, try to use available memory functions
    // This is a fallback implementation
    return 0;
#endif
}

size_t DeviceMetricsHelper::getSramFree() {
#ifdef ARCH_ESP32
    return ESP.getFreeHeap(); // This is SRAM only
#else
    return 0;
#endif
}

size_t DeviceMetricsHelper::getSramTotal() {
#ifdef ARCH_ESP32
    return ESP.getHeapSize(); // This is SRAM only
#else
    return 320000; // 320KB typical for ESP32
#endif
}

int DeviceMetricsHelper::getSramUtilization() {
    size_t freeSram = getSramFree();
    size_t totalSram = getSramTotal();
    
    if (totalSram == 0) return 0;
    
    size_t usedSram = totalSram - freeSram;
    int utilization = (int)((usedSram * 100) / totalSram);
    
    // Ensure utilization is within 0-100 range
    if (utilization < 0) utilization = 0;
    if (utilization > 100) utilization = 100;
    
    return utilization;
}

size_t DeviceMetricsHelper::getPsramFree() {
#if defined(ARCH_ESP32) && defined(CONFIG_SPIRAM_SUPPORT) && defined(BOARD_HAS_PSRAM)
    if (ESP.getPsramSize() > 0) {
        return ESP.getFreePsram();
    }
#endif
    return 0;
}

size_t DeviceMetricsHelper::getPsramTotal() {
#if defined(ARCH_ESP32) && defined(CONFIG_SPIRAM_SUPPORT) && defined(BOARD_HAS_PSRAM)
    return ESP.getPsramSize();
#endif
    return 0;
}

int DeviceMetricsHelper::getPsramUtilization() {
    size_t freePsram = getPsramFree();
    size_t totalPsram = getPsramTotal();
    
    if (totalPsram == 0) return 0;
    
    size_t usedPsram = totalPsram - freePsram;
    int utilization = (int)((usedPsram * 100) / totalPsram);
    
    // Ensure utilization is within 0-100 range
    if (utilization < 0) utilization = 0;
    if (utilization > 100) utilization = 100;
    
    return utilization;
}

bool DeviceMetricsHelper::hasPsram() {
#if defined(ARCH_ESP32) && defined(CONFIG_SPIRAM_SUPPORT) && defined(BOARD_HAS_PSRAM)
    return ESP.getPsramSize() > 0;
#endif
    return false;
}

String DeviceMetricsHelper::getSeparateMemoryString() {
    String result = "SRAM: " + String(getSramUtilization()) + "%";
    
    if (hasPsram()) {
        result += "\nPSRAM: " + String(getPsramUtilization()) + "%";
    }
    
    return result;
}

size_t DeviceMetricsHelper::getTotalHeap() {
#ifdef ARCH_ESP32
    size_t totalHeap = ESP.getHeapSize();
    
#if defined(CONFIG_SPIRAM_SUPPORT) && defined(BOARD_HAS_PSRAM)
    // Add PSRAM to total available memory if present
    if (ESP.getPsramSize() > 0) {
        totalHeap += ESP.getPsramSize();
    }
#endif
    
    return totalHeap;
#else
    // For other architectures, return a reasonable default
    return 320000; // 320KB typical for ESP32
#endif
}

int DeviceMetricsHelper::getMemoryUtilization() {
    size_t freeHeap = getFreeHeap();
    size_t totalHeap = getTotalHeap();
    
    if (totalHeap == 0) return 0;
    
    size_t usedHeap = totalHeap - freeHeap;
    int utilization = (int)((usedHeap * 100) / totalHeap);
    
    // Ensure utilization is within 0-100 range
    if (utilization < 0) utilization = 0;
    if (utilization > 100) utilization = 100;
    
    return utilization;
}

bool DeviceMetricsHelper::hasChanged() {
    if (!initialized) {
        init();
        return true;
    }
    
    size_t currentFreeHeap = getFreeHeap();
    int currentMemoryPercent = getMemoryUtilization();
    int currentSramPercent = getSramUtilization();
    int currentPsramPercent = getPsramUtilization();
    
    // Check if there's a significant change in memory
    bool changed = false;
    
    // Check for significant heap change
    if (abs((long)(currentFreeHeap - lastFreeHeap)) > MEMORY_CHANGE_THRESHOLD) {
        changed = true;
    }
    
    // Check for percentage changes
    if (abs(currentMemoryPercent - lastMemoryPercent) > 2) { // 2% threshold
        changed = true;
    }
    
    if (abs(currentSramPercent - lastSramPercent) > 2) { // 2% threshold for SRAM
        changed = true;
    }
    
    if (abs(currentPsramPercent - lastPsramPercent) > 2) { // 2% threshold for PSRAM
        changed = true;
    }
    
    if (changed) {
        lastFreeHeap = currentFreeHeap;
        lastMemoryPercent = currentMemoryPercent;
        lastSramPercent = currentSramPercent;
        lastPsramPercent = currentPsramPercent;
    }
    
    // Update minimum heap tracking
    if (currentFreeHeap < minFreeHeapSeen) {
        minFreeHeapSeen = currentFreeHeap;
    }
    
    return changed;
}

String DeviceMetricsHelper::getMemoryString() {
    int utilization = getMemoryUtilization();
    return String(utilization) + "%";
}

String DeviceMetricsHelper::getDetailedMemoryString() {
    size_t freeHeap = getFreeHeap();
    size_t totalHeap = getTotalHeap();
    size_t usedHeap = totalHeap - freeHeap;
    int utilization = getMemoryUtilization();
    
    String result = "Used: ";
    
    // Format used memory
    if (usedHeap >= 1024) {
        result += String(usedHeap / 1024) + "KB";
    } else {
        result += String(usedHeap) + "B";
    }
    
    result += "/";
    
    // Format total memory
    if (totalHeap >= 1024*1024) {
        result += String(totalHeap / (1024*1024)) + "MB";
    } else if (totalHeap >= 1024) {
        result += String(totalHeap / 1024) + "KB";
    } else {
        result += String(totalHeap) + "B";
    }
    
    result += " (" + String(utilization) + "%)";
    
#if defined(CONFIG_SPIRAM_SUPPORT) && defined(BOARD_HAS_PSRAM)
    if (ESP.getPsramSize() > 0) {
        result += " +PSRAM";
    }
#endif
    
    return result;
}

size_t DeviceMetricsHelper::getMinFreeHeap() {
#ifdef ARCH_ESP32
    return ESP.getMinFreeHeap();
#else
    return minFreeHeapSeen;
#endif
}