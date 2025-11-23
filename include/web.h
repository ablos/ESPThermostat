#ifndef WEB_H
#define WEB_H

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <api.h>

class SimpleWebServer
{
    private:
        APIHandler &apiHandler = APIHandler::getInstance();
        bool initialized = false;

        AsyncWebServer server;

        // Route handlers
        void handleStatus(AsyncWebServerRequest *request);
        void handleNotFound(AsyncWebServerRequest *request);

        SimpleWebServer();

    public:
        // Singleton accessor
        static SimpleWebServer& getInstance()
        {
            static SimpleWebServer instance;
            return instance;
        }

        // Delete copy constructor and assignment operator
        SimpleWebServer(const SimpleWebServer &) = delete;
        SimpleWebServer &operator=(const SimpleWebServer &) = delete;

        bool begin();
        bool isInitialized();
};

#endif