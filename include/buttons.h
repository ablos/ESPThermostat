#ifndef BUTTONS_H
#define BUTTONS_H

#include <Arduino.h>
#include <config.h>
#include <OneButton.h>
#include <data.h>

class ButtonManager 
{
    private:
        DataManager *dataManager;
        static ButtonManager *instance;

        OneButton modeBtn;
        OneButton tUpBtn;
        OneButton tDownBtn;

        static void modeSingleClickEvent();
        void handleModeSingleClick();
        static void modeLongClickEvent();
        void handleModeLongClick();

        static void tempUpClickEvent();
        void handleTempUpClick();

        static void tempDownClickEvent();
        void handleTempDownClick();

    public:
        ButtonManager(DataManager *dm);

        void begin();
        void update();
};

#endif