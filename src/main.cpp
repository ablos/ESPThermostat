#include <Arduino.h>
#include <web.h>

// WiFi Configuration
const char* ssid = "Geraldine";
const char* password = "ripHarry2022";

SimpleWebServer webServer(ssid, password);

void setup()
{
  Serial.begin(115200);
  Serial.println();
  
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
    delay(100);
}