#include <Arduino.h>
#include <config.h>
#include <data.h>
#include <thermostat.h>
#include <buttons.h>
#include <network.h>
#include <time_manager.h>
#include <mqtt.h>
#include <display.h>
#include <web.h>

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

    // Initialize network manager
    NetworkManager::getInstance().begin();

    // Initialize time manager
    TimeManager::getInstance().begin();

    // Initialize MQTT manager
    if (USE_MQTT)
        MQTTManager::getInstance().begin();

    // Initialize eInk display
    DisplayManager::getInstance().begin();

    // Initialize Web Server
    if (USE_WEB)
        SimpleWebServer::getInstance().begin();

    Serial.println("Setup complete!");
}

void loop()
{
    NetworkManager::getInstance().update();
    TimeManager::getInstance().update();
    MQTTManager::getInstance().update();
    Thermostat::getInstance().update();
    ButtonManager::getInstance().update();

    // Small delay to limit processing power
    // And therefore leaking heat that could affect readings
    delay(10);
}