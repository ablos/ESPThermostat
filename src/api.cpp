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
        doc["mode"] = dataManager->getMode();
    }
    
    // Thermostat data
    if (thermostat) 
    {
        doc["currentTemp"] = thermostat->getCurrentTemp();
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



String APIHandler::handleGetAwayTemperature() 
{
    return handleGet("awayTemp", dataManager->getAwayTemp());
}

String APIHandler::handleSetAwayTemperature(const String& requestBody) 
{
    return handleSet<float>(requestBody, "awayTemp", &DataManager::setAwayTemp);
}



String APIHandler::handleGetMode()
{
    return handleGet("mode", dataManager->getMode());
}

String APIHandler::handleSetMode(const String& requestBody) 
{
    return handleSet<String>(requestBody, "mode", &DataManager::setMode);
}