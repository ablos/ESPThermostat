#include <Arduino.h>
#include <web.h>
#include <secrets.h>
#include <esp_task_wdt.h>
#include <data.h>
#include <thermostat.h>
#include <api.h>

DataManager* dataManager;
Thermostat* thermostat;
APIHandler* apiHandler;
SimpleWebServer* webServer;

void setup()
{
    Serial.begin(115200);
    Serial.println();

    // Initialize data manager first
    dataManager = new DataManager();
    dataManager->begin();

    // Initialize thermostat
    thermostat = new Thermostat(dataManager);
    thermostat->begin();
    
    // Initialize API handler
    apiHandler = new APIHandler(dataManager, thermostat);

    // Initialize Web Server
    webServer = new SimpleWebServer(WIFI_SSID, WIFI_PASS, apiHandler);
    
    if (!webServer->begin())
    {
        Serial.println("Failed to start web server!");
        while(1)
            delay(1000);
    }

    Serial.println("Webserver started successfully!");
}

void loop()
{
    delay(100);
}