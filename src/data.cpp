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
        // Load from flash, using struct defaults as fallbacks
        settings.targetTemp = preferences.getFloat("targetTemp", settings.targetTemp);
        settings.mode = preferences.getString("mode", settings.mode);
        settings.tempOffset = preferences.getFloat("tempOffset", settings.tempOffset);
        settings.hysteresis = preferences.getFloat("hysteresis", settings.hysteresis);
        settings.minTemp = preferences.getFloat("minTemp", settings.minTemp);
        settings.maxTemp = preferences.getFloat("maxTemp", settings.maxTemp);
        settings.ecoTemp = preferences.getFloat("ecoTemp", settings.ecoTemp);
        settings.epdRefreshRate = preferences.getUInt("epdRefreshRate", settings.epdRefreshRate);
        settings.tempChangeThreshold = preferences.getFloat("tempChangeThreshold", settings.tempChangeThreshold);
        settings.humidityChangeThreshold = preferences.getFloat("humidityChangeThreshold", settings.humidityChangeThreshold);
        settings.timezone = preferences.getString("timezone", settings.timezone);
        settings.languageCode = preferences.getString("languageCode", settings.languageCode);

        Serial.println("Settings loaded from flash");
    }
    else
    {
        // First time: save defaults to flash
        saveAllSettings();
        preferences.putBool("initialized", true);
        Serial.println("Default settings applied");
    }

    initialized = true;
    printSettings();
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
    settings = ThermostatSettings();  // Reset to struct defaults
    saveAllSettings();
}

void DataManager::saveAllSettings()
{
    if (!initialized)
        return;

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
}

bool DataManager::updateSettings(const ThermostatSettings& newSettings)
{
    if (!initialized)
        return false;

    // Update each setting through individual setters for validation
    setMinTemp(newSettings.minTemp);
    setMaxTemp(newSettings.maxTemp);
    setTargetTemp(newSettings.targetTemp);
    setEcoTemp(newSettings.ecoTemp);
    setMode(newSettings.mode);
    setTempOffset(newSettings.tempOffset);
    setTempChangeThreshold(newSettings.tempChangeThreshold);
    setHumidityChangeThreshold(newSettings.humidityChangeThreshold);
    setEpdRefreshRate(newSettings.epdRefreshRate);
    setTimezone(newSettings.timezone);
    setLanguageCode(newSettings.languageCode);

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

bool DataManager::setTempOffset(float offset)
{
    if (!initialized)
        return false;

    settings.tempOffset = offset;
    preferences.putFloat("tempOffset", settings.tempOffset);

    Serial.printf("Temperature offset set to %.1f°C\n", offset);
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

float DataManager::getTempOffset()
{
    return settings.tempOffset;
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