#ifndef THERMOSTAT_H
#define THERMOSTAT_H

#include <Arduino.h>
#include <data.h>
#include <Adafruit_AHTX0.h>
#include <config.h>

struct ThermostatStatus
{
    float currentTemp = 19.0;
    float currentHumidity = 50.0;
    bool heaterActive = false;
};

class Thermostat 
{
    private:
        DataManager& dataManager = DataManager::getInstance();
        ThermostatStatus status;
        Adafruit_AHTX0 aht;

        unsigned long lastSensorTime = 0;
        unsigned long lastControlTime = 0;

        void updateSensor();
        void controlHeater();
        
        Thermostat();

        bool initialized = false;

    public:
        // Singleton accessor
        static Thermostat& getInstance() 
        {
            static Thermostat instance;
            return instance;
        }
        
        // Delete copy constructor and assignment operator
        Thermostat(const Thermostat &) = delete;
        Thermostat &operator=(const Thermostat &) = delete;

        bool begin();
        void update();
        
        // Status access
        bool isInitialized();
        ThermostatStatus getStatus();
        float getCurrentTemp();
        float getCurrentHumidity();
        bool isHeaterActive();
};

#endif