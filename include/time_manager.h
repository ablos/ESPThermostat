#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include <Arduino.h>
#include <ezTime.h>
#include <data.h>
#include <network.h>

class TimeManager
{
    private:
        DataManager& dataManager = DataManager::getInstance();
        NetworkManager& networkManager = NetworkManager::getInstance();

        Timezone timezone;
        bool initialized = false;
        bool timeSynced = false;
        bool syncRequested = false;
        bool wasConnected = false;
        bool justSynced = false; // Flag for first sync event

        unsigned long lastSyncAttempt = 0;
        const unsigned long SYNC_RETRY_INTERVAL = 60000; // Retry every 60 seconds if sync fails

        TimeManager();

    public:
        // Singleton accessor
        static TimeManager& getInstance()
        {
            static TimeManager instance;
            return instance;
        }

        // Delete copy constructor and assignment operator
        TimeManager(const TimeManager &) = delete;
        TimeManager &operator=(const TimeManager &) = delete;

        void begin();
        void update();

        Timezone& getTimezone();
        bool isSynced();
        bool isInitialized();
        bool checkAndClearJustSynced(); // Returns true once after first sync, then false
};

#endif
