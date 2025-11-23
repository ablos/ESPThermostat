#ifndef BUTTONS_H
#define BUTTONS_H

#include <Arduino.h>
#include <config.h>
#include <OneButton.h>
#include <data.h>

class ButtonManager 
{
    private:
        DataManager& dataManager = DataManager::getInstance();

        bool initialized = false;

        OneButton modeBtn;
        OneButton tUpBtn;
        OneButton tDownBtn;
        OneButton progBtn;

        static void modeSingleClickEvent();
        void handleModeSingleClick();
        static void modeLongClickEvent();
        void handleModeLongClick();

        static void tempUpClickEvent();
        void handleTempUpClick();

        static void tempDownClickEvent();
        void handleTempDownClick();

        static void progSingleClickEvent();
        void handleProgSingleClick();
        
        ButtonManager();

    public:
        // Singleton accessor
        static ButtonManager& getInstance() 
        {
            static ButtonManager instance;
            return instance;
        }
        
        // Delete copy constructor and assignment operator
        ButtonManager(const ButtonManager &) = delete;
        ButtonManager &operator=(const ButtonManager &) = delete;

        bool begin();
        void update();
        bool isInitialized();
};

#endif