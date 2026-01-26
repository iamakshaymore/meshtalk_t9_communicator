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
 * Base interface for all CustomUI initializers
 * Provides a common pattern for modular component initialization
 * Initializers ONLY handle hardware initialization - no logic or updates
 */
class InitBase {
public:
    virtual ~InitBase() = default;
    
    /**
     * Initialize the component
     * @return true if initialization successful, false otherwise
     */
    virtual bool init() = 0;
    
    /**
     * Cleanup resources when shutting down
     */
    virtual void cleanup() = 0;
    
    /**
     * Get component name for logging
     * @return component name string
     */
    virtual const char* getName() const = 0;
    
    /**
     * Check if component is initialized and ready
     * @return true if ready, false otherwise
     */
    virtual bool isReady() const = 0;
};