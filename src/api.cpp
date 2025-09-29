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
        doc["enabled"] = dataManager->isEnabled();
        doc["awayMode"] = dataManager->isAwayMode();
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

String APIHandler::handleSetTargetTemperature(const String& requestBody) 
{
    DynamicJsonDocument doc(256);
    DeserializationError error = deserializeJson(doc, requestBody);
    
    if (error)
        return handleError("Invalid JSON");

    if (!doc.containsKey("targetTemp"))
        return handleError("Missing targetTemp field");

    float newTemp = doc["targetTemp"];
    
    if (dataManager && dataManager->setTargetTemp(newTemp)) 
        return handleStatus();
    else 
        return handleError("Failed to set temperature");
}

String APIHandler::handleGetTargetTemperature() 
{
    if (!(dataManager && dataManager->isInitialized()))
        return handleError("Data manager not initialized");

    return handleGet("targetTemp", dataManager->getTargetTemp());
}