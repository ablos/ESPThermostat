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

    Serial.println("Display initialized!");
}

void DisplayManager::update() 
{
    // Read out current status
    float currentTemp = round(thermostat->getCurrentTemp() * 2) / 2;
    float targetTemp = dataManager->getMode() == "eco" ? dataManager->getEcoTemp() : dataManager->getTargetTemp();
    float humidity = round(thermostat->getCurrentHumidity());
    bool heatingActive = thermostat->getStatus().heaterActive;

    // Refresh if heatingActive or targetTemp changed
    if (heatingActive != lastHeatingActive || targetTemp != lastTargetTemp) 
    {
        refreshDisplay(currentTemp, targetTemp, humidity);

        lastHeatingActive = heatingActive;
        return;
    }
    
    // Refresh after minimum interval length and if current temp or humidity changed enough
    if (millis() - lastRefresh >= (EPD_REFRESH_RATE * 1000)
        && (abs(currentTemp - lastCurrentTemp) >= TEMP_CHANGE_THRESHOLD
            || abs(humidity - lastHumidity) >= HUMIDITY_CHANGE_THRESHOLD)) 
    {
        refreshDisplay(currentTemp, targetTemp, humidity);
    }
}

void DisplayManager::refreshDisplay(float currentTemp, float targetTemp, float humidity)
{
    display->setFullWindow();
    display->firstPage();

    do
    {
        // Clear display
        display->fillScreen(GxEPD_WHITE);

        // Draw different parts
        drawLines();
        drawCurrentTemperature(currentTemp);
        drawHumidity(humidity);
        drawTargetTemperature(targetTemp);
        drawDate();
        drawFireIcon();

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

void DisplayManager::drawTargetTemperature(float temperature) 
{
    if (dataManager->getMode() == "off")
        return;

    char buffer[10];
    uint16_t x, y, w, h;
    getTargetTempRegion(temperature, buffer, &x, &y, &w, &h);

    // Print icon and text
    display->setCursor(x, y);
    display->print(buffer);
    display->drawBitmap(x - (targetTempIconSize + spacing), 71, target_icon, targetTempIconSize, targetTempIconSize, GxEPD_BLACK);
}

void DisplayManager::drawCurrentTemperature(float temperature) 
{
    // Make current temp string with single decimal point
    char buffer[10];
    uint16_t x, y, w, h;
    getCurrentTempRegion(temperature, buffer, &x, &y, &w, &h);

    // Print text
    display->setCursor(x, y);
    display->print(buffer);
    
    // Calculate position of degree circle
    int xc = x + w + (degreeRadius / 2) + 10;
    int yc = y - h + degreeRadius - 3;
    
    // Draw degrees circle
    display->fillCircle(xc, yc, degreeRadius, GxEPD_BLACK);
    display->fillCircle(xc, yc, degreeRadius / 2, GxEPD_WHITE);
}

void DisplayManager::drawHumidity(float humidity) 
{
    // Make humidity string without decimals
    char buffer[10];

    uint16_t x, y, w, h;
    getHumidityRegion(humidity, buffer, &x, &y, &w, &h);

    // Print text and icon
    display->setCursor(x, y);
    display->print(buffer);
    display->drawBitmap(x - (humidityIconSize + spacing), 101, humidity_icon, humidityIconSize, humidityIconSize, GxEPD_BLACK);
}

void DisplayManager::drawFireIcon() 
{
    if (!thermostat->getStatus().heaterActive)
        return;

    display->drawBitmap(225, 17, fire_icon, 61, 61, GxEPD_RED);
}

void DisplayManager::drawDate() 
{
    // TODO: IMPLEMENT DATE
}

// Usage (don't forget to set font first!):
// uint16_t w, h;
// getStringBounds("Hello", &w, &h);
void DisplayManager::getStringBounds(const char *str, uint16_t *w, uint16_t *h)
{
    int16_t x1, y1;
    display->getTextBounds(str, 0, 0, &x1, &y1, w, h);
}

void DisplayManager::getCurrentTempRegion(float temperature, char *buffer, uint16_t *x, uint16_t *y, uint16_t *w, uint16_t *h) 
{
    sprintf(buffer, "%.1f", temperature);
    
    // Set text parameters
    display->setTextColor(GxEPD_BLACK);
    setFontExtraBold();
    
    // Get width and height of string
    getStringBounds(buffer, w, h);
    
    // Calculate position
    *x = (display->width() / 2) - (*w / 2);
    *y = dataManager->getMode() == "off" ? (95 / 2) + (*h / 2) : 35 + (*h / 2);
}

void DisplayManager::getTargetTempRegion(float temperature, char *buffer, uint16_t *x, uint16_t *y, uint16_t *w, uint16_t *h) 
{
    // Make target temp string with single decimal point
    sprintf(buffer, "%.1f", temperature);

    // Set text parameters
    display->setTextColor(GxEPD_BLACK);
    setFontSemiBold();
    
    // Get width and height of string
    getStringBounds(buffer, w, h);

    // Calculate position
    *x = (display->width() / 2) + (targetTempIconSize + spacing - *w) / 2;
    *y = 70 + *h;
}

void DisplayManager::getHumidityRegion(float humidity, char *buffer, uint16_t *x, uint16_t *y, uint16_t *w, uint16_t *h) 
{
    sprintf(buffer, "%.0f%%", humidity);
    
    // Set text parameters
    display->setTextColor(GxEPD_BLACK);
    setFontSemiBold();
    
    // Get width and height of string
    getStringBounds(buffer, w, h);

    // Calculate position
    *x = (display->width() / 5) + (humidityIconSize + spacing - *w) / 2;
    *y = (display->height() + *h) / 2 + 50;
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