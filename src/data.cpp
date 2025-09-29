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

        settings.awayTemp = preferences.getFloat("awayTemp", 16.0);

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
    if (settings.awayTemp < settings.minTemp || settings.awayTemp > settings.maxTemp) return false;
    if (settings.hysteresis < 0.1 || settings.hysteresis > 5.0) return false;

    return true;
}

void DataManager::printSettings()
{
    Serial.println("=== Thermostat Settings ===");
    Serial.printf("Target Temp: %.1f°C\n", settings.targetTemp);
    Serial.printf("Mode: %s\n", settings.mode);
    Serial.printf("Away Temp: %.1f°C\n", settings.awayTemp);
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
    
    settings.awayTemp = 16.0;
    
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
    if (temp.awayTemp < temp.minTemp) temp.awayTemp = temp.minTemp;
    if (temp.awayTemp > temp.maxTemp) temp.awayTemp = temp.maxTemp;

    settings = temp;
    
    // Save to flash
    preferences.putFloat("targetTemp", settings.targetTemp);
    preferences.putString("mode", settings.mode);
    preferences.putFloat("tempOffset", settings.tempOffset);
    preferences.putFloat("hysteresis", settings.hysteresis);
    preferences.putFloat("minTemp", settings.minTemp);
    preferences.putFloat("maxTemp", settings.maxTemp);

    preferences.putFloat("awayTemp", settings.awayTemp);

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

bool DataManager::setAwayTemp(float temp) 
{
    if (!initialized)
        return false;

    temp = constrain(temp, settings.minTemp, settings.maxTemp);
    settings.awayTemp = temp;
    preferences.putFloat("awayTemp", settings.awayTemp);
    
    Serial.printf("Away temperature set to %.1f°C\n", temp);
    return true;
}

bool DataManager::setMode(String mode) 
{
    if (!initialized)
        return false;
        
    if (mode != "off" && mode != "away" && mode != "on") 
    {
        Serial.println("INVALID MODE!");
        ESP.restart();
    }

    settings.mode = mode;
    preferences.putString("mode", settings.mode);
    
    Serial.printf("Thermostat %s\n", mode);
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

float DataManager::getAwayTemp() 
{
    return settings.awayTemp;
}