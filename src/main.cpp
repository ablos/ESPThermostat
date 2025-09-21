#include <Arduino.h>
#include <web.h>
#include <secrets.h>

SimpleWebServer webServer(WIFI_SSID, WIFI_PASS);

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