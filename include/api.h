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
        DataManager& dataManager = DataManager::getInstance();
        Thermostat& thermostat = Thermostat::getInstance();

        String handleError(const char *errorMessage);

        template<typename T>
        String handleGet(const char *key, T value) {
            if (!(dataManager.isInitialized()))
                return handleError("Data manager not initialized");

            DynamicJsonDocument doc(128);
            doc["status"] = "ok";
            doc[key] = value;
            String output;
            serializeJson(doc, output);
            return output;
        }

        template<typename T>
        String handleSet(const String& requestBody, const char *key, bool (DataManager::*setter)(T)) {
            if (!(dataManager.isInitialized()))
                return handleError("Data manager not initialized");

            DynamicJsonDocument doc(256);
            DeserializationError error = deserializeJson(doc, requestBody);

            if (error)
                return handleError("Invalid JSON");

            if (!doc.containsKey(key))
                return handleError("Missing field");

            T value = doc[key];

            if ((dataManager.*setter)(value))
                return handleStatus();
            else
                return handleError("Failed to set value");
        }
        
        APIHandler();

    public:
        // Singleton accessor
        static APIHandler& getInstance() 
        {
            static APIHandler instance;
            return instance;
        }
        
        // Delete copy constructor and assignment operator
        APIHandler(const APIHandler &) = delete;
        APIHandler &operator=(const APIHandler &) = delete;

        // API Endpoint Handlers
        String handleStatus();

        String handleGetCurrentTemperature();
        String handleGetCurrentHumidity();

        String handleGetTargetTemperature();
        String handleSetTargetTemperature(const String &requestBody);

        String handleGetEcoTemperature();
        String handleSetEcoTemperature(const String &requestBody);

        String handleGetMode();
        String handleSetMode(const String& requestBody);

        String handleGetMaxTemperature();
        String handleSetMaxTemperature(const String &requestBody);

        String handleGetMinTemperature();
        String handleSetMinTemperature(const String &requestBody);
};

#endif