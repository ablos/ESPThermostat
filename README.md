# ESP32 Thermostat

A smart thermostat project using ESP32 with AHT21B temperature/humidity sensor.

## Hardware Setup

- **ESP32 DOIT DevKit V1**
- **AHT21B Sensor:**
  - SDA → GPIO 21
  - SCL → GPIO 22
- **Heater Control (C1815 transistor):**
  - Base resistor: 1kΩ - 2.2kΩ from GPIO (defined in config.h)
  - Protection diode: 1N4148 (cathode to collector, anode to emitter)
  - Switches CV ketel line (23V 21mA)

## PlatformIO Commands

### Basic Commands

```bash
# Upload code only
pio run --target upload

# Upload filesystem (data folder with web app)
pio run --target uploadfs

# Monitor serial output
pio run --target monitor

# Build without uploading
pio run
```

### Full Deploy Sequence

```bash
# 1. Erase ESP32 flash (optional, for clean install)
pio run --target erase

# 2. Upload filesystem (web app files from data folder)
pio run --target uploadfs

# 3. Upload code
pio run --target upload

# 4. Monitor serial output
pio run --target monitor
```

### One-Line Deploy

```bash
# Upload both filesystem and code
pio run --target uploadfs && pio run --target upload
```

## VS Code Tasks

Use the **Terminal → Run Task** menu or press `Ctrl+Shift+P` and search for "Run Task" to access:

- **ESP32: Upload Code** - Upload the program
- **ESP32: Upload Filesystem** - Upload web app files
- **ESP32: Upload All** - Upload both filesystem and code
- **ESP32: Erase Flash** - Wipe the ESP32
- **ESP32: Monitor** - Open serial monitor

## Configuration

Edit `include/config.h` to configure:
- I2C pins for AHT sensor
- Transistor control pin
- Other hardware settings

Edit `include/secrets.h` to configure:
- WiFi SSID
- WiFi Password

## API Endpoints

- `GET /api/status` - Get all status info
- `GET /api/target` - Get target temperature
- `POST /api/target/set` - Set target temperature
- `GET /api/mode` - Get mode (off/eco/on)
- `POST /api/mode/set` - Set mode
- `GET /api/current` - Get current temperature
- `GET /api/humidity` - Get current humidity

## Troubleshooting

### AHT Sensor Not Found
- Check I2C connections (SDA=21, SCL=22)
- Verify pull-up resistors are present
- Run I2C scanner (built into thermostat.begin())
- AHT21B should appear at address 0x38

### Serial Monitor Shows Garbled Text
- Check baud rate is set to 115200
- PlatformIO monitor should auto-configure

### Web Interface Not Loading
- Ensure filesystem was uploaded with `uploadfs`
- Check serial monitor for IP address
- Verify WiFi credentials in secrets.h
