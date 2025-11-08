#ifndef THERMOSTAT_H
#define THERMOSTAT_H

#include <Arduino.h>
#include <data.h>
#include <Adafruit_AHTX0.h>
#include <config.h>

struct ThermostatStatus
{
    float currentTemp = 19.0;
    float currentHumidity = 40;
    bool heaterActive = false;
};

class Thermostat 
{
    private:
        DataManager* dataManager;
        ThermostatStatus status;
        Adafruit_AHTX0 aht;

        unsigned long lastSensorTime = 0;
        unsigned long lastControlTime = 0;

        void updateSensor();
        void controlHeater();

    public:
        Thermostat(DataManager* dm);

        bool begin();
        void update();
        
        // Status access
        ThermostatStatus getStatus();
        float getCurrentTemp();
        float getCurrentHumidity();
};

#endif