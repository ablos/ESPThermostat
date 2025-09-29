#ifndef API_H
#define API_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <data.h>
#include <thermostat.h>
#include <WiFi.h>

class APIHandler 
{
    private:
        DataManager *dataManager;
        Thermostat *thermostat;

        String handleError(const char *errorMessage);
        
        template<typename T>
        String handleGet(const char *key, T value) {
            DynamicJsonDocument doc(128);
            doc["status"] = "ok";
            doc[key] = value;
            String output;
            serializeJson(doc, output);
            return output;
        }

    public:
        APIHandler(DataManager *dm, Thermostat *thermo);
        
        // API Endpoint Handlers
        String handleStatus();
        String handleSetTargetTemperature(const String& requestBody);
        String handleGetTargetTemperature();
};

#endif