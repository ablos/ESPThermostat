#include <mqtt.h>

MQTTManager::MQTTManager() : deviceId("thermostat"), deviceName("ESP Thermostat"), mqttClient(wifiClient)
{
    // Create topic strings
    baseTopic = String("homeassistant/climate/") + deviceId;
    stateTopic = baseTopic + "/state";
    commandTopic = baseTopic + "/set";
    availabilityTopic = baseTopic + "/availability";
}

void MQTTManager::begin() 
{
    if (initialized)
        return;

    Serial.println("Starting MQTT Manager...");
    
    // Configure MQTT Client
    mqttClient.setServer(MQTT_HOST, MQTT_PORT);
    mqttClient.setCallback(messageCallback);
    mqttClient.setKeepAlive(60);
    mqttClient.setSocketTimeout(15);

    Serial.println("MQTT Manager started");

    initialized = true;
}

void MQTTManager::update() 
{
    if (!initialized)
        return;
        
    // Handle connection states
    switch (state) 
    {
        case MQTTState::DISCONNECTED:
            // Only try to connect if WiFi is connected
            if (networkManager.isConnected() && millis() - lastConnectionAttempt > RECONNECT_INTERVAL)
            {
                Serial.println("Attempting MQTT connection...");
                state = MQTTState::CONNECTING;
                lastConnectionAttempt = millis();
            }
            break;
            
        case MQTTState::CONNECTING:
            if (attemptConnection()) 
            {
                state = MQTTState::CONNECTED;
                Serial.println("MQTT Connected");
                
                // Subscribe to command topics
                mqttClient.subscribe((commandTopic + "/#").c_str(), 1);
                
                // Publish availability
                publish(availabilityTopic.c_str(), "online", true);
                
                // Publish Home Assistant discovery
                publishDiscovery();
                
                // Publish initial state
                publishState();
            }
            else 
            {
                state = MQTTState::FAILED;
                Serial.print("MQTT connection failed, rc=");
                Serial.println(mqttClient.state());
            }
            break;
        
        case MQTTState::FAILED:
            // Retry after interval
            if (millis() - lastConnectionAttempt > RECONNECT_INTERVAL) 
            {
                state = MQTTState::DISCONNECTED;
            }
            break;
        
        case MQTTState::CONNECTED:
            handleConnectedState();
            break;
    }
}

bool MQTTManager::attemptConnection() 
{
    // Create a unique client id
    String clientId = String(deviceId) + "_" + String(WiFi.macAddress());
    
    // Attempt to connect with LWT
    return mqttClient.connect(
        clientId.c_str(),
        MQTT_USER,
        MQTT_PASS,
        availabilityTopic.c_str(),
        1,  // QoS 1
        true,  // retained
        "offline"  // LWT message
    );
}

void MQTTManager::handleConnectedState() 
{
    mqttClient.loop();
    
    // Check if still connected
    if (!mqttClient.connected()) 
    {
        Serial.println("MQTT Connection lost");
        state = MQTTState::DISCONNECTED;
        return;
    }
    
    // Poll thermostat every 2 seconds
    if (millis() - lastPollTime >= POLL_INTERVAL) 
    {
        float currentTemp = thermostat.getCurrentTemp();
        float humidity = thermostat.getCurrentHumidity();
        bool heatingActive = thermostat.isHeaterActive();

        String mode = dataManager.getMode();
        float targetTemp = mode == "eco" ? dataManager.getEcoTemp() : dataManager.getTargetTemp();

        if (forceNextPoll)
            currentTemp += 0.1;

        // Publish on any change
        if (
            currentTemp != lastPublishedCurrentTemp ||
            targetTemp != lastPublishedTargetTemp ||
            humidity != lastPublishedHumidity ||
            mode != lastPublishedMode ||
            heatingActive != lastPublishedHeatingActive ||
            forceNextPoll
        ) 
        {
            publishState();

            // Update last published values
            lastPublishedCurrentTemp = currentTemp;
            lastPublishedTargetTemp = targetTemp;
            lastPublishedHumidity = humidity;
            lastPublishedMode = mode;
            lastPublishedHeatingActive = heatingActive;
        }

        if (forceNextPoll) 
        {
            forceNextPoll = false;
        }
        else 
        {
            lastPollTime = millis();
        }
    }
}

bool MQTTManager::publish(const char* topic, const char* payload, bool retained) 
{
    if (state != MQTTState::CONNECTED)
        return false;

    return mqttClient.publish(topic, payload, retained);
}

void MQTTManager::publishState() 
{
    if (state != MQTTState::CONNECTED)
        return;
        
    // Create JSON state payload
    StaticJsonDocument<256> doc;
    
    // Round values to 1 decimal place
    float currentTemp = round(thermostat.getCurrentTemp() * 10.0) / 10.0;
    float humidity = round(thermostat.getCurrentHumidity() * 10.0) / 10.0;
    
    if (forceNextPoll)
        currentTemp += 0.1;

    // Map internal mode to HA mode / preset
    String internalMode = dataManager.getMode();
    
    // Get target temp based on mode
    float targetTemp = internalMode == "eco" ? dataManager.getEcoTemp() : dataManager.getTargetTemp();
    targetTemp = round(targetTemp * 10.0) / 10.0;

    String haMode;
    String preset;
    
    if (internalMode == "off") 
    {
        haMode = "off";
        preset = "comfort";
    }
    else if (internalMode == "eco") 
    {
        haMode = "heat";
        preset = "eco";
    }
    else 
    {
        haMode = "heat";
        preset = "comfort";
    }
    
    // Current state to JSON
    doc["current_temperature"] = serialized(String(currentTemp, 1));
    doc["temperature"] = serialized(String(targetTemp, 1));
    doc["mode"] = haMode;
    doc["preset"] = preset;
    
    // Add humidity as attribute
    doc["humidity"] = serialized(String(humidity, 1));
    doc["action"] = thermostat.isHeaterActive() ? "heating" : "idle";

    String output;
    serializeJson(doc, output);

    publish(stateTopic.c_str(), output.c_str(), true);
}

void MQTTManager::publishDiscovery() 
{
    Serial.println("Publishing Home Assistant discovery...");
    publishClimateDiscovery();
    publishSensorDiscovery();
}

void MQTTManager::publishClimateDiscovery() 
{
    String discoveryTopic = "homeassistant/climate/" + String(deviceId) + "/config";

    StaticJsonDocument<1024> doc;
    
    // Device info
    doc["name"] = deviceName;
    doc["unique_id"] = String(deviceId) + "_climate";

    JsonObject device = doc["device"].to<JsonObject>();
    device["identifiers"][0] = deviceId;
    device["name"] = deviceName;
    device["model"] = "ESP Thermostat";
    device["manufacturer"] = "ablos";
    
    // MQTT Topics
    doc["state_topic"] = stateTopic;
    doc["command_topic"] = commandTopic;
    doc["availability_topic"] = availabilityTopic;
    
    // Temperature config
    doc["current_temperature_topic"] = stateTopic;
    doc["current_temperature_template"] = "{{ value_json.current_temperature }}";
    doc["temperature_state_topic"] = stateTopic;
    doc["temperature_state_template"] = "{{ value_json.temperature }}";
    doc["temperature_command_topic"] = commandTopic + "/temperature";

    // Humidity config
    doc["current_humidity_topic"] = stateTopic;
    doc["current_humidity_template"] = "{{ value_json.humidity }}";

    doc["mode_state_topic"] = stateTopic;
    doc["mode_state_template"] = "{{ value_json.mode }}";
    doc["mode_command_topic"] = commandTopic + "/mode";
    
    // Modes
    JsonArray modes = doc["modes"].to<JsonArray>();
    modes.add("off");
    modes.add("heat");
    
    // Presets (only when on)
    doc["preset_mode_state_topic"] = stateTopic;
    doc["preset_mode_value_template"] = "{{ value_json.preset }}";
    doc["preset_mode_command_topic"] = commandTopic + "/preset";

    JsonArray presets = doc["preset_modes"].to<JsonArray>();
    presets.add("comfort");
    presets.add("eco");
    
    // Temperature settings
    doc["min_temp"] = dataManager.getMinTemp();
    doc["max_temp"] = dataManager.getMaxTemp();
    doc["temp_step"] = 0.5;
    doc["temperature_unit"] = "C";
    
    // Action
    doc["action_topic"] = stateTopic;
    doc["action_template"] = "{{ value_json.action }}";

    doc["optimistic"] = false;

    String output;
    serializeJson(doc, output);
    
    bool success = publish(discoveryTopic.c_str(), output.c_str(), true);
    Serial.print("Climate discovery publish: ");
    Serial.println(success ? "SUCCESS" : "FAILED");
}

void MQTTManager::publishSensorDiscovery() 
{
    // Humidity sensor discovery
    String humidityDiscoveryTopic = "homeassistant/sensor/" + String(deviceId) + "_humidity/config";

    StaticJsonDocument<512> doc;

    doc["name"] = String(deviceName) + " Humidity";
    doc["unique_id"] = String(deviceId) + "_humidity";
    doc["state_topic"] = stateTopic;
    doc["value_template"] = "{{ value_json.humidity }}";
    doc["unit_of_measurement"] = "%";
    doc["device_class"] = "humidity";

    JsonObject device = doc["device"].to<JsonObject>();
    device["identifiers"][0] = deviceId;

    String output;
    serializeJson(doc, output);
    
    bool success = publish(humidityDiscoveryTopic.c_str(), output.c_str(), true);
    Serial.print("Humidity sensor discovery publish: ");
    Serial.println(success ? "SUCCESS" : "FAILED");
    
    // Temperature sensor discovery
    String tempDiscoveryTopic = "homeassistant/sensor/" + String(deviceId) + "_temperature/config";

    StaticJsonDocument<512> doc2;

    doc2["name"] = String(deviceName) + " Temperature";
    doc2["unique_id"] = String(deviceId) + "_temperature";
    doc2["state_topic"] = stateTopic;
    doc2["value_template"] = "{{ value_json.current_temperature }}";
    doc2["unit_of_measurement"] = "°C";
    doc2["device_class"] = "temperature";

    JsonObject device2 = doc2["device"].to<JsonObject>();
    device2["identifiers"][0] = deviceId;

    String output2;
    serializeJson(doc2, output2);

    bool success2 = publish(tempDiscoveryTopic.c_str(), output2.c_str(), true);
    Serial.print("Temperature sensor discovery publish: ");
    Serial.println(success2 ? "SUCCESS" : "FAILED");
}

void MQTTManager::messageCallback(char* topic, byte* payload, unsigned int length) 
{
    // Convert payload to string
    String message;
    for (unsigned int i = 0; i < length; i++) 
    {
        message += (char)payload[i];
    }
    
    // Call instance method
    MQTTManager::getInstance().handleMessage(String(topic), message);
}

void MQTTManager::handleMessage(String topic, String payload) 
{
    Serial.print("MQTT message received on topic: ");
    Serial.print(topic);
    Serial.print(" - Payload: ");
    Serial.println(payload);
    
    // Handle temperature set command
    if (topic == commandTopic + "/temperature") 
    {
        // Don't allow temperature changes in eco mode
        if (dataManager.getMode() == "eco") 
        {
            Serial.println("Illegal update");
            forceNextPoll = true;
            lastPollTime -= POLL_INTERVAL;

            return;
        }

        // Set target temperature, validation is handled by data manager
        dataManager.setTargetTemp(payload.toFloat());
        
        // Make next poll instant to update values
        lastPollTime -= POLL_INTERVAL;
    }

    // Handle mode set command
    else if (topic == commandTopic + "/mode") 
    {
        payload.toLowerCase();
        
        // Translate HA modes to internal modes
        if (payload == "off") 
        {
            dataManager.setMode("off");
        }
        else if (payload == "heat") 
        {
            dataManager.setMode("on");
        }
        
        // Make next poll instant to update values
        lastPollTime -= POLL_INTERVAL;
    }
    
    // Handle preset set command
    else if (topic == commandTopic + "/preset") 
    {
        payload.toLowerCase();
        String currentMode = dataManager.getMode();
        
        // Translate HA presets to internal modes
        if (payload == "eco" && currentMode != "off") 
        {
            dataManager.setMode("eco");
        }
        else if (payload == "comfort" && currentMode != "off") 
        {
            dataManager.setMode("on");
        }
        else 
        {
            forceNextPoll = true;
        }

        // Make next poll instant to update values
        lastPollTime -= POLL_INTERVAL;
    }
}

bool MQTTManager::isConnected()
{
    return state == MQTTState::CONNECTED;
}

bool MQTTManager::isInitialized()
{
    return initialized;
}
