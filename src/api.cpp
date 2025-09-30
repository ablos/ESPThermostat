#include <api.h>

APIHandler::APIHandler(DataManager* dm, Thermostat* thermo) : dataManager(dm), thermostat(thermo) {}

String APIHandler::handleError(const char* errorMessage) 
{
    DynamicJsonDocument errorDoc(128);
    errorDoc["status"] = "error";
    errorDoc["error"] = errorMessage;
    String output;
    serializeJson(errorDoc, output);
    return output;
}

String APIHandler::handleStatus() 
{
    DynamicJsonDocument doc(512);
    
    // System status
    doc["status"] = "ok";
    doc["ip"] = WiFi.localIP().toString();
    doc["rssi"] = WiFi.RSSI();
    doc["heap"] = ESP.getFreeHeap();
    doc["uptime"] = millis() / 1000;
    
    // Datamanger data
    if (dataManager && dataManager->isInitialized())
    {
        doc["targetTemp"] = dataManager->getTargetTemp();
        doc["ecoTemp"] = dataManager->getEcoTemp();
        doc["mode"] = dataManager->getMode();
        doc["maxTemp"] = dataManager->getMaxTemp();
        doc["minTemp"] = dataManager->getMinTemp();
    }
    
    // Thermostat data
    if (thermostat) 
    {
        doc["currentTemp"] = thermostat->getCurrentTemp();
        doc["humidity"] = thermostat->getCurrentHumidity();
    }

    String output;
    serializeJson(doc, output);
    return output;
}



String APIHandler::handleGetCurrentTemperature() 
{
    return handleGet("currentTemp", thermostat->getCurrentTemp());
}

String APIHandler::handleGetCurrentHumidity() 
{
    return handleGet("currentHumidity", thermostat->getCurrentHumidity());
}



String APIHandler::handleGetTargetTemperature()
{
    return handleGet("targetTemp", dataManager->getTargetTemp());
}

String APIHandler::handleSetTargetTemperature(const String& requestBody)
{
    return handleSet<float>(requestBody, "targetTemp", &DataManager::setTargetTemp);
}



String APIHandler::handleGetEcoTemperature()
{
    return handleGet("ecoTemp", dataManager->getEcoTemp());
}

String APIHandler::handleSetEcoTemperature(const String& requestBody)
{
    return handleSet<float>(requestBody, "ecoTemp", &DataManager::setEcoTemp);
}



String APIHandler::handleGetMode()
{
    return handleGet("mode", dataManager->getMode());
}

String APIHandler::handleSetMode(const String& requestBody) 
{
    return handleSet<String>(requestBody, "mode", &DataManager::setMode);
}



String APIHandler::handleGetMaxTemperature() 
{
    return handleGet("maxTemp", dataManager->getMaxTemp());
}

String APIHandler::handleSetMaxTemperature(const String &requestBody) 
{
    return handleSet<float>(requestBody, "maxTemp", &DataManager::setMaxTemp);
}



String APIHandler::handleGetMinTemperature() 
{
    return handleGet("minTemp", dataManager->getMinTemp());
}

String APIHandler:: handleSetMinTemperature(const String &requestBody) 
{
    return handleSet<float>(requestBody, "minTemp", &DataManager::setMinTemp);
}