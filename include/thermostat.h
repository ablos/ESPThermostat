#ifndef THERMOSTAT_H
#define THERMOSTAT_H

#include <Arduino.h>
#include <data.h>

struct ThermostatStatus 
{
    float currentTemp = 19.0;
    float currentHumidity = 40;
};

class Thermostat 
{
    private:
        DataManager* dataManager;
        ThermostatStatus status;
        
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