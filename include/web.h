#ifndef WEB_H
#define WEB_H

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <LittleFS.h>

class SimpleWebServer {
private:
    AsyncWebServer server;
    const char* ssid;
    const char* password;
    
    // Route handlers
    void handleStatus(AsyncWebServerRequest *request);
    void handleNotFound(AsyncWebServerRequest *request);
    
public:
    SimpleWebServer(const char* wifi_ssid, const char* wifi_password);
    
    bool begin();
    bool isConnected();
    String getIP();
};

#endif