#pragma once

#include "BaseMenuScreen.h"

/**
 * Main Menu Screen
 * Implements the specific main menu items and navigation logic
 */
class MainMenuScreen : public BaseMenuScreen {
public:
    MainMenuScreen();
    virtual ~MainMenuScreen();
    
protected:
    virtual void onMenuItemSelected(int id) override;

private:
    // Menu item IDs
    static const int ID_NODES = 1;
    static const int ID_MESSAGES = 2;
    static const int ID_SNAKE = 3;
};
