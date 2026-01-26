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
 * Battery utility helper for CustomUI screens
 * Provides battery percentage and status information
 */
class BatteryHelper {
public:
    static void init();
    
    /**
     * Get current battery percentage (0-100)
     * @return battery percentage, -1 if unavailable
     */
    static int getBatteryPercent();
    
    /**
     * Check if battery information has changed since last call
     * @return true if battery info changed
     */
    static bool hasChanged();
    
    /**
     * Get formatted battery string for display
     * @return formatted string like "85%" or "N/A"
     */
    static String getBatteryString();
    
    /**
     * Check if device is charging
     * @return true if charging
     */
    static bool isCharging();

private:
    static int lastBatteryPercent;
    static bool lastChargingState;
    static bool initialized;
};