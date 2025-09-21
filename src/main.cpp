#include <Arduino.h>
#include <web.h>
#include <secrets.h>
#include <esp_task_wdt.h>

SimpleWebServer webServer(WIFI_SSID, WIFI_PASS);

void setup()
{
    Serial.begin(115200);
    Serial.println();
    
    // Configure watchdog
    esp_task_wdt_init(30, true);
    
    // Start web server
    if (!webServer.begin())
    {
        Serial.println("Failed to start web server!");
        while(1)
            delay(1000);
    }
    
    Serial.println("Webserver started successfully!");
}

void loop()
{
    // Feed watchdog to prevent timeout
    esp_task_wdt_reset();

    delay(100);
}