#include <Arduino.h>
#include <web.h>
#include <secrets.h>
#include <esp_task_wdt.h>
#include <data.h>
#include <thermostat.h>
#include <api.h>
#include <display.h>
#include <buttons.h>

DataManager* dataManager;
Thermostat* thermostat;
APIHandler* apiHandler;
SimpleWebServer* webServer;
DisplayManager* displayManager;
ButtonManager* buttonManager;

void setup()
{
    Serial.begin(115200);
    Serial.println();

    // Initialize data manager first
    dataManager = new DataManager();
    dataManager->begin();

    // Initialize thermostat
    thermostat = new Thermostat(dataManager);
    if (!thermostat->begin()) 
    {
        Serial.println("Failed to initialize thermostat!");
        while (1)
            delay(1000);
    }

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
    
    // Initialize buttons
    buttonManager = new ButtonManager(dataManager);
    buttonManager->begin();

    // Initialize eInk display
    displayManager = new DisplayManager(dataManager, thermostat);
    displayManager->begin();
}

void loop()
{
    thermostat->update();
    buttonManager->update();

    // Small delay to limit processing power
    // And therefore leaking heat that could affect readings
    delay(10);
}