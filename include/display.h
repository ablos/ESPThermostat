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

class DisplayManager 
{
    private:
        DataManager* dataManager;
        Thermostat* thermostat;

        GxEPD2_3C<GxEPD2_290_C90c, GxEPD2_290_C90c::HEIGHT> *display;
        uint8_t partialRefreshCount;

        const int targetTempIconSize = 18;
        const int humidityIconSize = 26;
        const int spacing = 4;
        const int degreeRadius = 6;

        unsigned long lastRefresh = 0;
        float lastTargetTemp = -999.0;
        float lastCurrentTemp = -999.0;
        int lastHumidity = -1;
        bool lastHeatingActive = false;

        void getStringBounds(const char* str, uint16_t* w, uint16_t* h);
        void getCurrentTempRegion(float temperature, char *buffer, uint16_t *x, uint16_t *y, uint16_t *w, uint16_t *h);
        void getTargetTempRegion(float temperature, char *buffer, uint16_t *x, uint16_t *y, uint16_t *w, uint16_t *h);
        void getHumidityRegion(float humidity, char *buffer, uint16_t *x, uint16_t *y, uint16_t *w, uint16_t *h);

        void setFontExtraBold();
        void setFontSemiBold();

        void drawLines();
        void drawTargetTemperature(float temperature);
        void drawCurrentTemperature(float temperature);
        void drawHumidity(float humidity);
        void drawFireIcon();
        void drawDate();

        void refreshDisplay(float currentTemp, float targetTemp, float humidity);

    public:
        DisplayManager(DataManager* dm, Thermostat* ts);

        void begin();
        void update();
};

#endif