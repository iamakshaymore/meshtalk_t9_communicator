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

#include "MainMenuScreen.h"
#include "modules/CustomUI/CustomUIModule.h"
#include "modules/CustomUI/screens/list_screens/NodesListScreen.h"
#include "modules/CustomUI/screens/list_screens/MessageListScreen.h"
#include "MessagesMenuScreen.h"
#include "modules/CustomUI/screens/SnakeGameScreen.h"

MainMenuScreen::MainMenuScreen() : BaseMenuScreen("Menu", 30) {
    // Initialize menu items
    addMenuItem("Nodes List", "1", ID_NODES);
    addMenuItem("Messages", "2", ID_MESSAGES);
    addMenuItem("Snake Game", "3", ID_SNAKE);
    
    // Set navigation hints
    std::vector<NavHint> hints;
    hints.push_back(NavHint('1', "Select"));
    hints.push_back(NavHint('2', "Up"));
    hints.push_back(NavHint('8', "Down"));
    hints.push_back(NavHint('A', "Back"));
    setNavigationHints(hints);
}

MainMenuScreen::~MainMenuScreen() {
}

void MainMenuScreen::onMenuItemSelected(int id) {
    if (!customUIModule) return;
    
    switch (id) {
        case ID_NODES:
            customUIModule->getScreenManager()->navigateTo(customUIModule->getNodesListScreen());
            break;
        case ID_MESSAGES:
            customUIModule->getScreenManager()->navigateTo(customUIModule->getMessagesMenuScreen());
            break;
        case ID_SNAKE:
            customUIModule->getScreenManager()->navigateTo(customUIModule->getSnakeGameScreen());
            break;
    }
}
