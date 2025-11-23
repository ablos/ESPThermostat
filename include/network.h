#ifndef NETWORK_H
#define NETWORK_H

#include <WiFi.h>
#include <ESPmDNS.h>
#include <Arduino.h>
#include <secrets.h>

class NetworkManager
{
    private:
        enum class WiFiState
        {
            DISCONNECTED,
            CONNECTING,
            CONNECTED,
            FAILED
        };

        WiFiState state = WiFiState::DISCONNECTED;
        unsigned long connectionStartTime = 0;
        const unsigned long CONNECTION_TIMEOUT = 30000; // 30 seconds
        const unsigned long RETRY_DELAY = 30000; // 30 seconds between retries
        unsigned long lastRetryTime = 0;

        const char* ssid;
        const char* password;
        const char* mdnsName;

        bool initialized = false;

        NetworkManager();

    public:
        // Singleton accessor
        static NetworkManager& getInstance()
        {
            static NetworkManager instance;
            return instance;
        }

        // Delete copy and assignment operator
        NetworkManager(const NetworkManager &) = delete;
        NetworkManager &operator=(const NetworkManager &) = delete;

        void begin();
        void update();

        bool isConnected();
        bool isInitialized();
        String getIP();
        int getRSSI();
};

#endif