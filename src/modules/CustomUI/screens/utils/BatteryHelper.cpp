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

#include "BatteryHelper.h"
#include "PowerStatus.h"
#include "configuration.h"

// Static member initialization
int BatteryHelper::lastBatteryPercent = -1;
bool BatteryHelper::lastChargingState = false;
bool BatteryHelper::initialized = false;

void BatteryHelper::init() {
    // Initialize power management if available
    initialized = true;
}

int BatteryHelper::getBatteryPercent() {
    if (!initialized) {
        init();
    }
    
    // Use the global powerStatus instance
    if (powerStatus) {
        return powerStatus->getBatteryChargePercent();
    }
    
    // Fallback - return 0 if no power status available
    return 0;
}

bool BatteryHelper::hasChanged() {
    int currentPercent = getBatteryPercent();
    bool currentCharging = isCharging();
    
    bool changed = (currentPercent != lastBatteryPercent) || 
                   (currentCharging != lastChargingState);
    
    if (changed) {
        lastBatteryPercent = currentPercent;
        lastChargingState = currentCharging;
    }
    
    return changed;
}

String BatteryHelper::getBatteryString() {
    int percent = getBatteryPercent();
    
    if (percent < 0) {
        return "N/A";
    }
    
    String result = String(percent) + "%";
    
    if (isCharging()) {
        result += "+"; // Indicate charging
    }
    
    return result;
}

bool BatteryHelper::isCharging() {
    // Use the global powerStatus instance
    if (powerStatus) {
        return powerStatus->getIsCharging();
    }
    
    return false;
}