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
        settings.enabled = preferences.getBool("enabled", false);
        settings.tempOffset = preferences.getFloat("tempOffset", 0.0);
        settings.hysteresis = preferences.getFloat("hysteresis", 0.5);
        settings.minTemp = preferences.getFloat("minTemp", 10.0);
        settings.maxTemp = preferences.getFloat("maxTemp", 35.0);

        settings.awayMode = preferences.getBool("awayMode", false);
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
    Serial.printf("Enabled: %s\n", settings.enabled ? "Yes" : "No");
    Serial.printf("Away Mode: %s\n", settings.awayMode ? "Yes" : "No");
    Serial.printf("Away Temp: %.1f°C\n", settings.awayTemp);
    Serial.println("===========================");
}

// ==================
// SETTERS
// ==================

void DataManager::setDefaults() 
{
    settings.targetTemp = 20.5;
    settings.enabled = false;
    settings.tempOffset = 0.0;
    settings.hysteresis = 0.5;
    settings.minTemp = 10.0;
    settings.maxTemp = 35.0;

    settings.awayMode = false;
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
    preferences.putBool("enabled", settings.enabled);
    preferences.putFloat("tempOffset", settings.tempOffset);
    preferences.putFloat("hysteresis", settings.hysteresis);
    preferences.putFloat("minTemp", settings.minTemp);
    preferences.putFloat("maxTemp", settings.maxTemp);

    preferences.putFloat("awayTemp", settings.awayTemp);
    preferences.putBool("awayMode", settings.awayMode);

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

bool DataManager::setEnabled(bool enabled) 
{
    if (!initialized)
        return false;

    settings.enabled = enabled;
    preferences.putBool("enabled", settings.enabled);
    
    Serial.printf("Thermostat %s\n", enabled ? "enabled" : "disabled");
    return true;
}

bool DataManager::setAwayMode(bool away) 
{
    if (!initialized)
        return false;

    settings.awayMode = away;
    preferences.putBool("awayMode", settings.awayMode);
    
    Serial.printf("Away mode %s\n", away ? "ON" : "OFF");
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

bool DataManager::isEnabled() 
{
    return settings.enabled;
}

bool DataManager::isAwayMode() 
{
    return settings.awayMode;
}

bool DataManager::isInitialized() 
{
    return initialized;
}