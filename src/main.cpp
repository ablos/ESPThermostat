#include <Arduino.h>
#include <web.h>
#include <secrets.h>
#include <esp_task_wdt.h>
#include <data.h>
#include <thermostat.h>
#include <api.h>
#include <display.h>
#include <buttons.h>

void setup()
{
    Serial.begin(115200);
    Serial.println();

    // Initialize data manager
    DataManager::getInstance().begin();

    // Initialize thermostat
    while (!Thermostat::getInstance().begin()) 
    {
        Serial.println("Failed to initialized thermostat");
        delay(5000);
    }
    
    // Initialize buttons
    ButtonManager::getInstance().begin();

    // Initialize eInk display
    DisplayManager::getInstance().begin();

    // Initialize Web Server
    if (!SimpleWebServer::getInstance().begin())
    {
        Serial.println("Failed to start web server!");
        while(1)
            delay(1000);
    }

    Serial.println("Webserver started successfully!");
}

void loop()
{
    Thermostat::getInstance().update();
    ButtonManager::getInstance().update();

    // Small delay to limit processing power
    // And therefore leaking heat that could affect readings
    delay(10);
}