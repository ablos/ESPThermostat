#ifndef DATA_H
#define DATA_H

#include <Preferences.h>
#include <Arduino.h>

struct ThermostatSettings 
{
    // General settings
    float targetTemp = 20.5;
    String mode = "off";
    float tempOffset = 0.0;
    float hysteresis = 0.5;
    float minTemp = 10.0;
    float maxTemp = 35.0;

    // Eco mode settings
    float ecoTemp = 16.0;
};

class DataManager 
{
    private:
        Preferences preferences;
        ThermostatSettings settings;
        bool initialized = false;
        
        // Internal helpers
        void setDefaults();
        bool validateSettings();
    
    public:
        DataManager();
        ~DataManager();
        
        // Initialization
        bool begin();
        void reset();
        
        // Settings access
        ThermostatSettings getSettings();
        bool updateSettings(const ThermostatSettings& newSettings);
        
        // Individual setting updates
        bool setTargetTemp(float temp);
        bool setMode(String mode);
        bool setEcoTemp(float temp);
        bool setMaxTemp(float temp);
        bool setMinTemp(float temp);

        // Quick access methods
        float getTargetTemp();
        String getMode();
        float getEcoTemp();
        float getMaxTemp();
        float getMinTemp();

        // Status
        bool isInitialized();
        void printSettings();
};

#endif