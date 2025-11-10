#include <display.h>

DisplayManager::DisplayManager(DataManager* dm, Thermostat* ts) : dataManager(dm), thermostat(ts) {}

void DisplayManager::begin() 
{
    Serial.println("Initializing display...");
    
    // Initialize SPI with custom pins
    SPI.begin(EPD_SCK, -1, EPD_MOSI, EPD_CS);
    
    // Initialize eInk display
    display = new GxEPD2_3C<GxEPD2_290_C90c, GxEPD2_290_C90c::HEIGHT>(GxEPD2_290_C90c(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY));
    display->init(115200);
    display->setRotation(1);
    
    // Initialize time
    waitForSync(5);
    timezone.setLocation(dataManager->getTimezone());

    if (timeStatus() == timeSet) 
    {
        Serial.println("Time synchronized");
    }
    else 
    {
        Serial.println("Time sync failed, will retry later.");
    }
    setInterval(3600);
    setDebug(ezDebugLevel_t::NONE);

    // Start update task on core 0
    xTaskCreatePinnedToCore([](void* param)
    {
        DisplayManager* dm = static_cast<DisplayManager*>(param);
        dm->updateTask();
    }, "DisplayUpdateTask", 8192, this, 1, NULL, 0);

    Serial.println("Display initialized!");
}

void DisplayManager::update() 
{
    // Update time when connected to WiFi
    if (WiFi.status() == WL_CONNECTED) 
    {
        events();
    }

    // Read out current status
    String mode = dataManager->getMode();
    float currentTemp = round(thermostat->getCurrentTemp() * 2) / 2;
    float targetTemp = mode == "eco" ? dataManager->getEcoTemp() : dataManager->getTargetTemp();
    float humidity = round(thermostat->getCurrentHumidity());
    bool heatingActive = thermostat->getStatus().heaterActive;

    // Refresh if heatingActive or targetTemp changed or mode changed
    if (heatingActive != lastHeatingActive || targetTemp != lastTargetTemp || mode != lastMode) 
    {
        refreshDisplay(currentTemp, targetTemp, humidity, mode, heatingActive);

        lastHeatingActive = heatingActive;
        lastMode = mode;
        return;
    }
    
    // Refresh after minimum interval length and if current temp or humidity changed enough
    if (millis() - lastRefresh >= (dataManager->getEpdRefreshRate() * 1000)
        && (abs(currentTemp - lastCurrentTemp) >= dataManager->getTempChangeThreshold()
            || abs(humidity - lastHumidity) >= dataManager->getHumidityChangeThreshold())) 
    {
        refreshDisplay(currentTemp, targetTemp, humidity, mode, heatingActive);
        lastRefresh = millis();
        return;
    }
}

void DisplayManager::updateTask() 
{
    // Update every 2 seconds
    while (true) 
    {
        update();
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

void DisplayManager::refreshDisplay(float currentTemp, float targetTemp, float humidity, String mode, bool heatingActive)
{
    display->setFullWindow();
    display->firstPage();

    do
    {
        // Clear display
        display->fillScreen(GxEPD_WHITE);

        // Draw different parts
        drawLines();
        drawCurrentTemperature(currentTemp, mode);
        drawHumidity(humidity);
        drawTargetTemperature(targetTemp, mode);
        drawDate();
        drawFireIcon(heatingActive);
        drawLeafIcon(mode);

    } while (display->nextPage());

    partialRefreshCount = 0;
    lastTargetTemp = targetTemp;
    lastCurrentTemp = currentTemp;
    lastHumidity = humidity;
}

void DisplayManager::drawLines() 
{
    // top left corner + width and height of rect
    display->fillRect(0, 94, display->width(), 2, GxEPD_BLACK);
    display->fillRect((display->width() * 2 / 5) - 1, 96, 2, 96, GxEPD_BLACK);
}

void DisplayManager::drawTargetTemperature(float temperature, String mode)
{
    if (mode != "on")
        return;

    // Format temperature string
    char buffer[10];
    if (temperature == (int)temperature)
    {
        sprintf(buffer, "%.0f", temperature);
    }
    else
    {
        sprintf(buffer, "%.1f", temperature);
    }

    // Set text parameters
    display->setTextColor(GxEPD_BLACK);
    setFontSemiBold();

    // Get width and height of string
    uint16_t w, h;
    getStringBounds(buffer, &w, &h);

    // Calculate position
    int x = (display->width() / 2) + (targetTempIconSize + iconSpacing - w) / 2;
    int y = 70 + h;

    // Print icon and text
    display->setCursor(x, y);
    display->print(buffer);
    display->drawBitmap(x - (targetTempIconSize + iconSpacing), 71, target_icon, targetTempIconSize, targetTempIconSize, GxEPD_BLACK);

    // Calculate position of degree circle
    int xc = x + w + (degreeRadiusSmall / 2) + 7;
    int yc = y - h + degreeRadiusSmall;

    display->fillCircle(xc, yc, degreeRadiusSmall, GxEPD_BLACK);
    display->fillCircle(xc, yc, degreeRadiusSmall / 2, GxEPD_WHITE);
}

void DisplayManager::drawCurrentTemperature(float temperature, String mode)
{
    // Format temperature string
    char buffer[10];
    if (temperature == (int)temperature)
    {
        sprintf(buffer, "%.0f", temperature);
    }
    else
    {
        sprintf(buffer, "%.1f", temperature);
    }

    // Set text parameters
    display->setTextColor(GxEPD_BLACK);
    setFontExtraBold();

    // Get width and height of string
    uint16_t w, h;
    getStringBounds(buffer, &w, &h);

    // Calculate position
    int x = (display->width() / 2) - (w / 2);
    int y = mode == "on" ? 35 + (h / 2) : (95 / 2) + (h / 2);

    // Print text
    display->setCursor(x, y);
    display->print(buffer);

    // Calculate position of degree circle
    int xc = x + w + (degreeRadiusBig / 2) + 10;
    int yc = y - h + degreeRadiusBig - 3;

    // Draw degrees circle
    display->fillCircle(xc, yc, degreeRadiusBig, GxEPD_BLACK);
    display->fillCircle(xc, yc, degreeRadiusBig / 2, GxEPD_WHITE);
}

void DisplayManager::drawHumidity(float humidity)
{
    // Format humidity string
    char buffer[10];
    sprintf(buffer, "%.0f%%", humidity);

    // Set text parameters
    display->setTextColor(GxEPD_BLACK);
    setFontSemiBold();

    // Get width and height of string
    uint16_t w, h;
    getStringBounds(buffer, &w, &h);

    // Calculate position
    int x = (display->width() / 5) + (humidityIconSize + iconSpacing - w) / 2;
    int y = (display->height() + h) / 2 + 50;

    // Print text and icon
    display->setCursor(x, y);
    display->print(buffer);
    display->drawBitmap(x - (humidityIconSize + iconSpacing), 101, humidity_icon, humidityIconSize, humidityIconSize, GxEPD_BLACK);
}

void DisplayManager::drawFireIcon(bool heatingActive) 
{
    if (heatingActive) 
    {
        display->drawBitmap(225, 17, fire_icon, 61, 61, GxEPD_RED);
    }
}

void DisplayManager::drawLeafIcon(String mode) 
{
    if (mode == "eco") 
    {
        display->drawBitmap(10, 17, leaf_icon, 61, 61, GxEPD_BLACK);
    }
}

void DisplayManager::drawDate() 
{
    String dateString = "";

    if (timeStatus() == timeSet) 
    {
        dateString += dataManager->getLanguagePack()->days[timezone.weekday()];
        dateString += ", ";
        dateString += timezone.day();
        dateString += " ";
        dateString += dataManager->getLanguagePack()->months[timezone.month() - 1];
    }
    else 
    {
        dateString = "?";
    }

    char *date = new char[dateString.length() + 1];
    dateString.toCharArray(date, dateString.length() + 1);

    setFontSemiBold();

    uint16_t w, h;
    getStringBounds(date, &w, &h);

    int x = (display->width() * 7) / 10 - w / 2;
    int y = (display->height() + 95) / 2 + h / 2 - 2;

    display->setCursor(x, y);
    display->print(date);

    delete[] date;
}

// Usage (don't forget to set font first!):
// uint16_t w, h;
// getStringBounds("Hello", &w, &h);
void DisplayManager::getStringBounds(const char *str, uint16_t *w, uint16_t *h)
{
    int16_t x1, y1;
    display->getTextBounds(str, 0, 0, &x1, &y1, w, h);
}

void DisplayManager::setFontExtraBold() 
{
    display->setFont(&Inter_28pt_ExtraBold27pt7b);
    display->setTextSize(1);
}

void DisplayManager::setFontSemiBold() 
{
    display->setFont(&Inter_18pt_SemiBold12pt7b);
    display->setTextSize(1);
}