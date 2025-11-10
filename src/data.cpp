#include <data.h>

DataManager::DataManager() {}

DataManager::~DataManager() 
{
    if (initialized)
        preferences.end();
}

bool DataManager::begin() 
{
    Serial.println("Initializing Data Manager...");
    
    if (!preferences.begin("thermostat", false)) 
    {
        Serial.println("Failed to initialize preferences");
        return false;
    }
    
    // Load existing settings or set defaults
    if (preferences.isKey("initialized")) 
    {
        // Load from flash
        settings.targetTemp = preferences.getFloat("targetTemp", 20.5);
        settings.mode = preferences.getString("mode", "off");
        settings.tempOffset = preferences.getFloat("tempOffset", 0.0);
        settings.hysteresis = preferences.getFloat("hysteresis", 0.5);
        settings.minTemp = preferences.getFloat("minTemp", 10.0);
        settings.maxTemp = preferences.getFloat("maxTemp", 35.0);

        settings.ecoTemp = preferences.getFloat("ecoTemp", 16.0);

        settings.epdRefreshRate = preferences.getUInt("epdRefreshRate", 300);
        settings.tempChangeThreshold = preferences.getFloat("tempChangeThreshold", 0.5);
        settings.humidityChangeThreshold = preferences.getFloat("humidityChangeThreshold", 3);

        settings.timezone = preferences.getString("timezone", "Europe/Amsterdam");
        settings.languageCode = preferences.getString("languageCode", "nl");

        Serial.println("Settings loaded from flash");
    }
    else 
    {
        setDefaults();
        preferences.putBool("initialized", true);
        Serial.println("Default settings applied");
    }
    
    // Validate loaded settings
    if (!validateSettings()) 
    {
        Serial.println("Invalid settings detected, applying defaults...");
        setDefaults();
    }

    initialized = true;
    printSettings();
    return true;
}

bool DataManager::validateSettings() 
{
    // Check settings
    if (settings.minTemp >= settings.maxTemp) return false;
    if (settings.targetTemp < settings.minTemp || settings.targetTemp > settings.maxTemp) return false;
    if (settings.ecoTemp < settings.minTemp || settings.ecoTemp > settings.maxTemp) return false;
    if (settings.hysteresis < 0.1 || settings.hysteresis > 5.0) return false;
    if (settings.timezone.length() < 1) return false;
    if (settings.languageCode.length() != 2) return false;

    return true;
}

void DataManager::printSettings()
{
    Serial.println("=== Thermostat Settings ===");
    Serial.printf("Target Temp: %.1f°C\n", settings.targetTemp);
    Serial.printf("Mode: %s\n", settings.mode);
    Serial.printf("Eco Temp: %.1f°C\n", settings.ecoTemp);
    Serial.println("===========================");
}

// ==================
// SETTERS
// ==================

void DataManager::setDefaults() 
{
    settings.targetTemp = 20.5;
    settings.mode = "off";
    settings.tempOffset = 0.0;
    settings.hysteresis = 0.5;
    settings.minTemp = 10.0;
    settings.maxTemp = 35.0;
    
    settings.ecoTemp = 16.0;

    settings.epdRefreshRate = 300;
    settings.tempChangeThreshold = 0.5;
    settings.humidityChangeThreshold = 3;
    settings.timezone = "Europe/Amsterdam";
    settings.languageCode = "nl";

    // Save defaults
    updateSettings(settings);
}

bool DataManager::updateSettings(const ThermostatSettings& newSettings) 
{
    if (!initialized)
        return false;
        
    // Validate new settings
    ThermostatSettings temp = newSettings;
    if (temp.targetTemp < temp.minTemp) temp.targetTemp = temp.minTemp;
    if (temp.targetTemp > temp.maxTemp) temp.targetTemp = temp.maxTemp;
    if (temp.ecoTemp < temp.minTemp) temp.ecoTemp = temp.minTemp;
    if (temp.ecoTemp > temp.maxTemp) temp.ecoTemp = temp.maxTemp;

    settings = temp;
    
    // Save to flash
    preferences.putFloat("targetTemp", settings.targetTemp);
    preferences.putString("mode", settings.mode);
    preferences.putFloat("tempOffset", settings.tempOffset);
    preferences.putFloat("hysteresis", settings.hysteresis);
    preferences.putFloat("minTemp", settings.minTemp);
    preferences.putFloat("maxTemp", settings.maxTemp);

    preferences.putFloat("ecoTemp", settings.ecoTemp);

    preferences.putUInt("epdRefreshRate", settings.epdRefreshRate);
    preferences.putFloat("tempChangeThreshold", settings.tempChangeThreshold);
    preferences.putFloat("humidityChangeThreshold", settings.humidityChangeThreshold);

    preferences.putString("timezone", settings.timezone);
    preferences.putString("languageCode", settings.languageCode);

    Serial.println("Settings saved to flash");
    return true;
}

bool DataManager::setTargetTemp(float temp) 
{
    if (!initialized)
        return false;

    temp = constrain(temp, settings.minTemp, settings.maxTemp);
    settings.targetTemp = temp;
    preferences.putFloat("targetTemp", settings.targetTemp);
    
    Serial.printf("Target temperature set to %.1f°C\n", temp);
    return true;
}

bool DataManager::setEcoTemp(float temp)
{
    if (!initialized)
        return false;

    temp = constrain(temp, settings.minTemp, settings.maxTemp);
    settings.ecoTemp = temp;
    preferences.putFloat("ecoTemp", settings.ecoTemp);

    Serial.printf("Eco temperature set to %.1f°C\n", temp);
    return true;
}

bool DataManager::setMaxTemp(float temp) 
{
    if (!initialized)
        return false;

    temp = constrain(temp, settings.minTemp, 50);
    settings.maxTemp = temp;
    preferences.putFloat("maxTemp", settings.maxTemp);
    
    if (settings.targetTemp > settings.maxTemp) 
    {
        settings.targetTemp = settings.maxTemp;
        preferences.putFloat("targetTemp", settings.targetTemp);
    }
    
    Serial.printf("Max temperature set to %.1f°C\n", temp);
    return true;
}

bool DataManager::setMinTemp(float temp) 
{
    if (!initialized)
        return false;

    temp = constrain(temp, 0, settings.maxTemp);
    settings.minTemp = temp;
    preferences.putFloat("minTemp", settings.minTemp);
    
    if (settings.targetTemp < settings.minTemp) 
    {
        settings.targetTemp = settings.minTemp;
        preferences.putFloat("targetTemp", settings.targetTemp);
    }
    
    Serial.printf("Min temperature set to %.1f°C\n", temp);
    return true;
}

bool DataManager::setMode(String mode) 
{
    if (!initialized)
        return false;
        
    if (mode != "off" && mode != "eco" && mode != "on") 
    {
        Serial.println("INVALID MODE!");
        ESP.restart();
    }

    settings.mode = mode;
    preferences.putString("mode", settings.mode);
    
    Serial.printf("Thermostat %s\n", mode);
    return true;
}

bool DataManager::setEpdRefreshRate(uint32_t refreshRate) 
{
    if (!initialized)
        return false;

    settings.epdRefreshRate = refreshRate;
    preferences.putUInt("epdRefreshRate", refreshRate);

    Serial.printf("EPD Refresh rate set to %lu\n", refreshRate);
    return true;
}

bool DataManager::setTempChangeThreshold(float threshold) 
{
    if (!initialized)
        return false;

    settings.tempChangeThreshold = threshold;
    preferences.putFloat("tempChangeThreshold", threshold);

    Serial.printf("Temperature change threshold is set to %.1f\n", threshold);
    return true;
}

bool DataManager::setHumidityChangeThreshold(float threshold) 
{
    if (!initialized)
        return false;

    settings.humidityChangeThreshold = threshold;
    preferences.putFloat("humidityChangeThreshold", threshold);

    Serial.printf("Humidity change threshold is set to %.1f\n", threshold);
    return true;
}

bool DataManager::setTimezone(String timezone) 
{
    if (!initialized)
        return false;
        
    if (timezone.length() < 1)
        return false;

    settings.timezone = timezone;
    preferences.putString("timezone", timezone);

    Serial.printf("Timezone set to %s\n", timezone);
    return true;
}

bool DataManager::setLanguageCode(String languageCode) 
{
    if (!initialized)
        return false;
        
    if (languageCode.length() != 2)
        return false;

    settings.languageCode = languageCode;
    preferences.putString("languageCode", languageCode);

    Serial.printf("Language code set to %s\n", languageCode);
    return true;
}

void DataManager::reset() 
{
    if (initialized) 
    {
        preferences.clear();
        setDefaults();
        Serial.println("Settings reset to default");
    }
}

// ==================
// GETTERS
// ==================

ThermostatSettings DataManager::getSettings() 
{
    return settings;
}

float DataManager::getTargetTemp() 
{
    return settings.targetTemp;
}

String DataManager::getMode() 
{
    return settings.mode;
}

bool DataManager::isInitialized() 
{
    return initialized;
}

float DataManager::getEcoTemp()
{
    return settings.ecoTemp;
}

float DataManager::getMaxTemp() 
{
    return settings.maxTemp;
}

float DataManager::getMinTemp() 
{
    return settings.minTemp;
}

float DataManager::getHysteresis() 
{
    return settings.hysteresis;
}

uint32_t DataManager::getEpdRefreshRate() 
{
    return settings.epdRefreshRate;
}

float DataManager::getTempChangeThreshold() 
{
    return settings.tempChangeThreshold;
}

float DataManager::getHumidityChangeThreshold() 
{
    return settings.humidityChangeThreshold;
}

String DataManager::getTimezone() 
{
    return settings.timezone;
}

const std::map<String, const LanguagePack*> DataManager::LANGUAGE_PACKS = 
{
    {"nl", &NL},
    {"en", &EN}
};

const LanguagePack* DataManager::getLanguagePack()
{
    auto it = LANGUAGE_PACKS.find(settings.languageCode);
    if (it != LANGUAGE_PACKS.end()) {
        return it->second;
    }
    return &NL;  // default to Dutch
}