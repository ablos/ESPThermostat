#include <network.h>

NetworkManager::NetworkManager()
    : ssid(WIFI_SSID), password(WIFI_PASS), mdnsName("thermostat")
{
}

void NetworkManager::begin()
{
    if (initialized)
        return;

    Serial.println("Starting network manager...");

    // Start WiFi connection
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    state = WiFiState::CONNECTING;
    connectionStartTime = millis();

    Serial.println("WiFi connection started");

    initialized = true;
}

void NetworkManager::update()
{
    if (!initialized)
        return;

    switch(state)
    {
        case WiFiState::CONNECTING:
            if (WiFi.status() == WL_CONNECTED)
            {
                state = WiFiState::CONNECTED;
                Serial.println("WiFi connected!");
                Serial.print("IP address: ");
                Serial.println(WiFi.localIP());

                // Setup mDNS
                if (MDNS.begin(mdnsName))
                {
                    Serial.println("mDNS responder started");
                    Serial.print("You can access via: http://");
                    Serial.print(mdnsName);
                    Serial.println(".local");
                }
                else
                {
                    Serial.println("Error setting up mDNS responder");
                }
            }
            else if (millis() - connectionStartTime > CONNECTION_TIMEOUT)
            {
                state = WiFiState::FAILED;
                lastRetryTime = millis();
                Serial.println("WiFi connection timeout!");
                Serial.print("Will retry in ");
                Serial.print(RETRY_DELAY / 1000);
                Serial.println(" seconds...");
            }
            break;

        case WiFiState::CONNECTED:
            // Monitor for disconnection
            if (WiFi.status() != WL_CONNECTED)
            {
                Serial.println("WiFi disconnected! Reconnecting...");
                WiFi.reconnect();
                state = WiFiState::CONNECTING;
                connectionStartTime = millis();
            }
            break;

        case WiFiState::FAILED:
            // Retry after delay
            if (millis() - lastRetryTime > RETRY_DELAY)
            {
                Serial.println("Retrying WiFi connection...");
                WiFi.begin(ssid, password);
                state = WiFiState::CONNECTING;
                connectionStartTime = millis();
            }
            break;

        case WiFiState::DISCONNECTED:
            // Do nothing, waiting for begin() to be called
            break;
    }
}

bool NetworkManager::isConnected()
{
    return state == WiFiState::CONNECTED;
}

bool NetworkManager::isInitialized()
{
    return initialized;
}

String NetworkManager::getIP()
{
    if (isConnected())
        return WiFi.localIP().toString();
    return "Not connected";
}

int NetworkManager::getRSSI()
{
    if (isConnected())
        return WiFi.RSSI();
    return 0;
}