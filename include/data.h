#ifndef DATA_H
#define DATA_H

#include <Preferences.h>
#include <Arduino.h>
#include <languages.h>
#include <map>

struct ThermostatSettings 
{
    // General settings
    float targetTemp = 20.5;
    String mode = "off";
    float tempOffset = -1.0;
    float hysteresis = 0.3;
    float minTemp = 10.0;
    float maxTemp = 35.0;

    // Eco mode settings
    float ecoTemp = 16.0;

    // Display settings
    uint32_t epdRefreshRate = 300;
    float tempChangeThreshold = 0.5;
    float humidityChangeThreshold = 3;
    
    // Time and language
    String timezone = "Europe/Amsterdam";
    String languageCode = "nl";
};

class DataManager
{
    private:
        Preferences preferences;
        ThermostatSettings settings;
        bool initialized = false;

        // Language packs map
        static const std::map<String, const LanguagePack*> LANGUAGE_PACKS;

        // Internal helpers
        void setDefaults();
        void saveAllSettings();
        
        // Private constructor and destructor
        DataManager();
        ~DataManager();
    
    public:
        // Singleton accessor
        static DataManager& getInstance() 
        {
            static DataManager instance;
            return instance;
        }
        
        // Delete copy constructor and assignment operator
        DataManager(const DataManager &) = delete;
        DataManager &operator=(const DataManager &) = delete;

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
        bool setTempOffset(float offset);
        bool setEpdRefreshRate(uint32_t refreshRate);
        bool setTempChangeThreshold(float threshold);
        bool setHumidityChangeThreshold(float threshold);
        bool setTimezone(String timezone);
        bool setLanguageCode(String languageCode);

        // Quick access methods
        float getTargetTemp();
        String getMode();
        float getEcoTemp();
        float getMaxTemp();
        float getMinTemp();
        float getTempOffset();
        float getHysteresis();
        uint32_t getEpdRefreshRate();
        float getTempChangeThreshold();
        float getHumidityChangeThreshold();
        String getTimezone();
        const LanguagePack* getLanguagePack();

        // Status
        bool isInitialized();
        void printSettings();
};

#endif