#ifndef MQTT_H
#define MQTT_H

#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <network.h>
#include <data.h>
#include <thermostat.h>
#include <secrets.h>

class MQTTManager
{
    private:
        enum class MQTTState
        {
            DISCONNECTED,
            CONNECTING,
            CONNECTED,
            FAILED
        };

        NetworkManager& networkManager = NetworkManager::getInstance();
        DataManager& dataManager = DataManager::getInstance();
        Thermostat& thermostat = Thermostat::getInstance();

        WiFiClient wifiClient;
        PubSubClient mqttClient;

        MQTTState state = MQTTState::DISCONNECTED;
        bool initialized = false;

        unsigned long lastConnectionAttempt = 0;
        const unsigned long RECONNECT_INTERVAL = 5000; // 5 seconds between reconnect attempts

        const char* deviceId;
        const char* deviceName;

        // Topic prefixes
        String baseTopic;
        String stateTopic;
        String commandTopic;
        String availabilityTopic;

        // Polling - track last published values
        float lastPublishedCurrentTemp = -999.0;
        float lastPublishedTargetTemp = -999.0;
        float lastPublishedHumidity = -999.0;
        String lastPublishedMode = "";
        bool lastPublishedHeatingActive = false;
        unsigned long lastPollTime = 0;
        const unsigned long POLL_INTERVAL = 2000; // Poll every 2 seconds
        const unsigned long COMMAND_COOLDOWN = 500; // Pause polling for 500ms after command
        bool forceNextPoll = false;

        // Home Assistant discovery
        void publishDiscovery();
        void publishClimateDiscovery();
        void publishSensorDiscovery();

        // Message handling
        static void messageCallback(char* topic, byte* payload, unsigned int length);
        void handleMessage(String topic, String payload);

        // Connection management
        bool attemptConnection();
        void handleConnectedState();

        MQTTManager();

    public:
        // Singleton accessor
        static MQTTManager& getInstance()
        {
            static MQTTManager instance;
            return instance;
        }

        // Delete copy constructor and assignment operator
        MQTTManager(const MQTTManager &) = delete;
        MQTTManager &operator=(const MQTTManager &) = delete;

        void begin();
        void update();

        // Publishing
        bool publish(const char* topic, const char* payload, bool retained = false);
        void publishState();

        // Status
        bool isConnected();
        bool isInitialized();
};

#endif