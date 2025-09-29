#include <thermostat.h>

Thermostat::Thermostat(DataManager* dm) : dataManager(dm) {}

bool Thermostat::begin() 
{
    Serial.println("Thermostat initialized (mock)...");
    return true;
}

void Thermostat::update() 
{
    
}

ThermostatStatus Thermostat::getStatus() 
{
    return status;
}

float Thermostat::getCurrentTemp() 
{
    return status.currentTemp;
}