#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <SPI.h>
#include <GxEPD2_3C.h>
#include <config.h>
#include <data.h>
#include <thermostat.h>
#include <inter_extrabold.h>
#include <inter_semibold.h>
#include <icons.h>
#include <ezTime.h>
#include <WiFi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

class DisplayManager 
{
    private:
        DataManager* dataManager;
        Thermostat* thermostat;

        GxEPD2_3C<GxEPD2_290_C90c, GxEPD2_290_C90c::HEIGHT> *display;
        Timezone timezone;
        uint8_t partialRefreshCount;

        const int targetTempIconSize = 18;
        const int humidityIconSize = 26;
        const int iconSpacing = 4;
        const int degreeRadiusBig = 6;
        const int degreeRadiusSmall = 3;

        unsigned long lastRefresh = 0;
        float lastTargetTemp = -999.0;
        float lastCurrentTemp = -999.0;
        int lastHumidity = -1;
        bool lastHeatingActive = false;
        String lastMode = "mode";

        void getStringBounds(const char* str, uint16_t* w, uint16_t* h);
        void setFontExtraBold();
        void setFontSemiBold();

        void drawLines();
        void drawTargetTemperature(float temperature, String mode);
        void drawCurrentTemperature(float temperature, String mode);
        void drawHumidity(float humidity);
        void drawFireIcon(bool heatingActive);
        void drawLeafIcon(String mode);
        void drawDate();

        void refreshDisplay(float currentTemp, float targetTemp, float humidity, String mode, bool heatingActive);

        void updateTask();

    public:
        DisplayManager(DataManager* dm, Thermostat* ts);

        void begin();
        void update();
};

#endif