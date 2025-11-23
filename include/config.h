#ifndef CONFIG_H
#define CONFIG_H

// AHT Sensor I2C pins
#define AHT_SDA 21
#define AHT_SCL 22

// TRANSISTOR PIN
#define TRANS_PIN 27

// eINK DISPLAY PINS
#define EPD_CS 4
#define EPD_DC 2
#define EPD_RST 15
#define EPD_BUSY 5
#define EPD_MOSI 23                     // Sometimes labeled SDA
#define EPD_SCK 18                      // Sometimes labeled SCL

// BUTTON PINS - Single mode switch pin
#define BTN_MODE 26
#define BTN_T_UP -1                     // Not used (mode switch only)
#define BTN_T_DOWN -1                  // Not used (mode switch only)
#define BTN_PROG -1                     // Not used

#endif