#ifndef CONFIG_H
#define CONFIG_H

// AHT Sensor I2C pins
#define AHT_SDA 21
#define AHT_SCL 22

// TRANSISTOR PIN
#define TRANS_PIN 25

// eINK DISPLAY PINS
#define EPD_CS 5
#define EPD_DC 17                       // Labeled TX2 on ESP32 Devkit V1
#define EPD_RST 16                      // Labeled RX2 on ESP32 Devkit V1
#define EPD_BUSY 4
#define EPD_MOSI 23                     // Sometimes labeled SDA
#define EPD_SCK 18                      // Sometimes labeled SCL

// BUTTON PINS
#define BTN_MODE 27
#define BTN_T_UP 13
#define BTN_T_DOWN 14

#endif