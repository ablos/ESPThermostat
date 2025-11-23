#include <time_manager.h>

TimeManager::TimeManager() {}

void TimeManager::begin()
{
    if (initialized)
        return;

    Serial.println("Starting time manager...");

    // Set NTP server
    setServer("pool.ntp.org");
    setInterval(3600); // Update every hour

    // Set timezone location from settings
    timezone.setLocation(dataManager.getTimezone());

    Serial.println("Time manager started (waiting for WiFi to sync)");

    initialized = true;
}

void TimeManager::update()
{
    if (!initialized)
        return;

    // Process ezTime events
    events();

    // Check for WiFi connection state changes
    bool isConnected = networkManager.isConnected();

    // Trigger sync when WiFi connects
    if (isConnected && !wasConnected)
    {
        Serial.println("WiFi connected - requesting time sync...");
        updateNTP();
        syncRequested = true;
        lastSyncAttempt = millis();
    }

    wasConnected = isConnected;

    // Check if time is synced
    if (syncRequested && !timeSynced)
    {
        if (timeStatus() == timeSet)
        {
            timeSynced = true;
            justSynced = true; // Set flag for first sync event
            Serial.println("Time synced successfully!");
            Serial.print("Current time: ");
            Serial.println(timezone.dateTime("Y-m-d H:i:s"));

            // Update timezone location in case it changed
            timezone.setLocation(dataManager.getTimezone());
        }
        else if (isConnected && millis() - lastSyncAttempt > SYNC_RETRY_INTERVAL)
        {
            // Retry sync if it failed
            Serial.println("Retrying time sync...");
            updateNTP();
            lastSyncAttempt = millis();
        }
    }

    // If we were synced but lost WiFi, mark as potentially needing resync
    if (!isConnected && timeSynced)
    {
        // Time will continue to run from internal clock, but mark that we should resync when WiFi returns
        syncRequested = false;
    }
}

Timezone& TimeManager::getTimezone()
{
    return timezone;
}

bool TimeManager::isSynced()
{
    return timeSynced;
}

bool TimeManager::isInitialized()
{
    return initialized;
}

bool TimeManager::checkAndClearJustSynced()
{
    if (justSynced)
    {
        justSynced = false;
        return true;
    }
    return false;
}
