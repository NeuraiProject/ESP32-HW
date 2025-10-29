# ESP32 Hardware Wallet

> Universal hardware wallet with persistent storage using ESP32 NVS (Non-Volatile Storage)

A web-based tool to flash and manage persistent key-value storage on **any ESP32 device** via Web Serial API. This project enables your ESP32 to function as a secure storage device accessible directly from your browser.

## Features

- **Web-based Flashing**: Install firmware directly from your browser using ESP Web Tools
- **Persistent Storage**: Save, read, delete, and list key-value pairs using NVS (Non-Volatile Storage)
- **Web Serial Interface**: Communicate with ESP32 without installing drivers
- **Real-time Console**: Monitor device communication with detailed logging
- **JSON Protocol**: Simple command structure for easy integration
- **Universal Compatibility**: Works with **all ESP32 models** (ESP32, S2, S3, C3, C6, etc.)
- **No Partition Configuration**: Uses built-in NVS - no special partition setup needed
- **Automatic Chip Detection**: Identifies your ESP32 model automatically

## Quick Start

1. **Visit the Web Interface**: Open `index.html` in Chrome, Edge, or Opera
2. **Flash Firmware**: 
   - Connect your ESP32 via USB
   - Click "Install ESP32 Storage Firmware"
   - Select your device and wait for flashing to complete
3. **Test Storage**:
   - Switch to "Test Storage" tab
   - Click "Connect to ESP32"
   - Start saving and reading data!

## Requirements

- **Browser**: Chrome, Edge, or Opera (Web Serial API support required)
- **Hardware**: Any ESP32 board (ESP32, ESP32-S2, ESP32-S3, ESP32-C3, ESP32-C6, XIAO, etc.)
- **USB Cable**: Data cable for ESP32 connection

## Storage Operations

### Save Data
```json
{"action": "save", "key": "username", "value": "John"}
```

### Read Data
```json
{"action": "read", "key": "username"}
```

### List All Keys
```json
{"action": "list"}
```

### Delete Key
```json
{"action": "delete", "key": "username"}
```

### Clear All Storage
```json
{"action": "clear"}
```

### Get System Info
```json
{"action": "info"}
```

**Response:**
```json
{
  "status": "success",
  "storage": {
    "type": "NVS",
    "max_key_length": 15,
    "max_value_length": 4000,
    "namespace": "neuraistore"
  },
  "chip": {
    "model": "ESP32-S3",
    "revision": 0,
    "cores": 2,
    "frequency": 240
  }
}
```

## Project Structure

```
ESP32-HW/
├── index.html              # Main web interface
├── styles.css              # UI styles
├── script.js               # Web Serial logic
├── manifest.json           # ESP Web Tools firmware manifest
├── firmware/               # Compiled firmware binaries
│   ├── bootloader-XIAO_ESP32C6.bin
│   ├── partitions-XIAO_ESP32C6.bin
│   ├── app-XIAO_ESP32C6.bin
│   └── firmware-source/    # Arduino source code
│       ├── HW_ESP32_XIAO_ESP32C6/  # Original version for ESP32-C6
│       └── HW_ESP32_UNIVERSAL/     # NVS version (recommended - works on all ESP32)
└── README.md
```

## 🔧 Building from Source

If you want to modify the firmware:

1. Open `firmware/firmware-source/HW_ESP32_UNIVERSAL/HW_ESP32_LittleFS_AutoPartition.ino` in Arduino IDE
2. Install required libraries:
   - **Preferences** (built-in)
   - **ArduinoJson** (install from Library Manager)
3. Select your ESP32 board model
4. **For ESP32-S3**: Set "USB CDC On Boot: Enabled" in Tools menu
5. Compile and upload

### Firmware Versions:

- **HW_ESP32_UNIVERSAL** ✅ **Recommended** - Uses NVS, universal compatibility for all ESP32 models
- **HW_ESP32_XIAO_ESP32C6** - Original version optimized for XIAO ESP32-C6

## Storage Technology: NVS

This project uses **NVS (Non-Volatile Storage)**, the built-in persistent storage system in ESP32:

### Advantages:
- ✅ **Always available** - No partition configuration needed
- ✅ **Universal** - Works on all ESP32 models
- ✅ **Reliable** - Designed for key-value storage
- ✅ **Persistent** - Data survives power loss and resets
- ✅ **Built-in** - No external libraries needed

### Limitations:
- ⚠️ Max key length: 15 characters
- ⚠️ Max value length: 4000 bytes (4KB)
- 💡 Perfect for storing credentials, settings, tokens, etc.

## Web Serial API

This project uses the [Web Serial API](https://developer.mozilla.org/en-US/docs/Web/API/Web_Serial_API) to communicate with ESP32 devices directly from the browser at 115200 baud rate.

## Security Note

This is an **experimental project** for educational and development purposes. While it implements persistent storage, it should not be used for storing highly sensitive data without additional security measures (encryption, authentication, etc.).

## License

MIT License - See LICENSE file for details

## 👥 Authors

Created by **Neurai Team**  
Website: [https://neurai.org](https://neurai.org)  
Repository: [https://github.com/NeuraiProject/ESP32-HW](https://github.com/NeuraiProject/ESP32-HW)

## Contributing

Contributions, issues, and feature requests are welcome! Feel free to check the issues page.

## Troubleshooting

**Device not appearing in port selection?**
- Make sure you're using a data cable (not charge-only)
- Try a different USB port
- For ESP32-S3: Ensure "USB CDC On Boot: Enabled" in Arduino IDE

**Connection fails?**
- Ensure no other program is using the serial port
- Reset your ESP32 and try again
- Check that the correct baud rate (115200) is being used

**Firmware flashing fails?**
- Hold the BOOT button on your ESP32 while clicking Install
- Try a different USB cable or port
- Verify your browser supports Web Serial API

**Data doesn't persist after power cycle?**
- Check the "Boot count" in Serial Monitor - it should increment
- Try "Erase All Flash" in Arduino IDE before uploading
- Verify you're using the NVS version of the firmware

**"Key too long" error?**
- NVS keys are limited to 15 characters
- Use shorter key names (e.g., "user" instead of "username_data")

---

by Neurai Project
