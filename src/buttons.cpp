#include <buttons.h>

ButtonManager *ButtonManager::instance = nullptr;
ButtonManager::ButtonManager(DataManager *dm) : dataManager(dm) 
{
    instance = this;
}

void ButtonManager::begin() 
{
    // Set up MODE button
    modeBtn.setup(BTN_MODE, INPUT, true);
    modeBtn.attachClick(modeSingleClickEvent);
    modeBtn.attachLongPressStart(modeLongClickEvent);
    
    // Set up TEMP UP button
    tUpBtn.setup(BTN_T_UP, INPUT, true);
    tUpBtn.attachClick(tempUpClickEvent);
    tUpBtn.attachDoubleClick(tempUpClickEvent);
    tUpBtn.attachMultiClick(tempUpClickEvent);

    // Set up TEMP DOWN button
    tDownBtn.setup(BTN_T_DOWN, INPUT, true);
    tDownBtn.attachClick(tempDownClickEvent);
    tDownBtn.attachDoubleClick(tempDownClickEvent);
    tDownBtn.attachMultiClick(tempDownClickEvent);
    
    // Set up PROG button
    progBtn.setup(BTN_PROG, INPUT, true);
    progBtn.attachClick(progSingleClickEvent);
}

void ButtonManager::update() 
{
    modeBtn.tick();
    tUpBtn.tick();
    tDownBtn.tick();
    progBtn.tick();
}

void ButtonManager::modeSingleClickEvent() 
{
    if (instance) instance->handleModeSingleClick();
}

void ButtonManager::handleModeSingleClick() 
{
    Serial.println("Mode button clicked");

    String mode = dataManager->getMode();
    
    if (mode == "eco") 
    {
        dataManager->setMode("on");
    }
    
    if (mode == "on") 
    {
        dataManager->setMode("eco");
    }
}

void ButtonManager::modeLongClickEvent() 
{
    if (instance) instance->handleModeLongClick();
}

void ButtonManager::handleModeLongClick() 
{
    Serial.println("Mode button long press");

    String mode = dataManager->getMode();
    
    if (mode != "off") 
    {
        dataManager->setMode("off");
    }
    else 
    {
        dataManager->setMode("on");
    }
}

void ButtonManager::tempUpClickEvent() 
{
    if (instance) instance->handleTempUpClick();
}

void ButtonManager::handleTempUpClick() 
{
    Serial.printf("Temp up button pressed %d times", tUpBtn.getNumberClicks());
    Serial.println();

    if (dataManager->getMode() == "on") 
    {
        dataManager->setTargetTemp(dataManager->getTargetTemp() + (tUpBtn.getNumberClicks() * 0.5));
    }
}

void ButtonManager::tempDownClickEvent() 
{
    if (instance) instance->handleTempDownClick();
}

void ButtonManager::handleTempDownClick() 
{
    Serial.printf("Temp down button pressed %d times", tDownBtn.getNumberClicks());
    Serial.println();

    if (dataManager->getMode() == "on") 
    {
        dataManager->setTargetTemp(dataManager->getTargetTemp() - (tDownBtn.getNumberClicks() * 0.5));
    }
}

void ButtonManager::progSingleClickEvent() 
{
    if (instance) instance->handleProgSingleClick();
}

void ButtonManager::handleProgSingleClick() 
{
    Serial.println("Prog button clicked");
}