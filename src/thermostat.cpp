#include <thermostat.h>
#include <Wire.h>

Thermostat::Thermostat(DataManager* dm) : dataManager(dm) {}

bool Thermostat::begin()
{
    pinMode(TRANS_PIN, OUTPUT);
    digitalWrite(TRANS_PIN, LOW);

    Serial.println("Initializing I2C...");
    Wire.begin(AHT_SDA, AHT_SCL);

    Serial.println("Initializing AHT sensor...");
    if (!aht.begin())
    {
        Serial.println("Could not find AHT sensor!");
        return false;
    }

    Serial.println("Thermostat initialized!");
    return true;
}

void Thermostat::update() 
{
    // Check how much time has passed since last sensor read and only read every 5 seconds
    if (millis() - lastSensorTime >= 5000) 
    {
        lastSensorTime = millis();
        updateSensor();
    }
    
    // Check how much time has passed since last heater control and update every 0.5 secondsP
    if (millis() - lastControlTime >= 500) 
    {
        lastControlTime = millis();
        controlHeater();
    }
}

void Thermostat::updateSensor() 
{
    sensors_event_t humidity, temp;
    
    // Read temperature and humidity from AHT sensor
    aht.getEvent(&humidity, &temp);
  
    // Store values
    status.currentTemp = temp.temperature;
    status.currentHumidity = humidity.relative_humidity;
}

void Thermostat::controlHeater()
{
    // Don't do anything when heating is off
    // Only turn off heating to be sure
    if (dataManager->getMode() == "off")
    {
        digitalWrite(TRANS_PIN, LOW);
        status.heaterActive = false;
        return;
    }

    // Determine target temperature based on mode
    float targetTemp;
    if (dataManager->getMode() == "eco")
        targetTemp = dataManager->getEcoTemp();
    else
        targetTemp = dataManager->getTargetTemp();

    // Turn heater on if temp is below target temp - hysteresis
    // Hysteresis is used to prevent excessive on/off switching
    if (status.currentTemp < (targetTemp - dataManager->getHysteresis()))
    {
        digitalWrite(TRANS_PIN, HIGH);
        status.heaterActive = true;
    }

    // Turn heater off if temp is above or equal to target temp
    if (status.currentTemp >= targetTemp)
    {
        digitalWrite(TRANS_PIN, LOW);
        status.heaterActive = false;
    }
}

ThermostatStatus Thermostat::getStatus() 
{
    return status;
}

float Thermostat::getCurrentTemp() 
{
    return status.currentTemp;
}

float Thermostat::getCurrentHumidity() 
{
    return status.currentHumidity;
}