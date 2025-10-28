# ESP32 Hardware Hwallet

> Hardware wallet with persistent storage using ESP32 (Experimental)

A web-based tool to flash and manage persistent key-value storage on ESP32 devices via Web Serial API. This project enables your ESP32 to function as a secure storage device accessible directly from your browser.

## Features

- **Web-based Flashing**: Install firmware directly from your browser using ESP Web Tools
- **Persistent Storage**: Save, read, delete, and list key-value pairs stored in SPIFFS
- **Web Serial Interface**: Communicate with ESP32 without installing drivers
- **Real-time Console**: Monitor device communication with detailed logging
- **JSON Protocol**: Simple command structure for easy integration
- **ESP32-C6 Support**: Optimized for XIAO ESP32C6 and compatible boards

## Quick Start

1. **Visit the Web Interface**: Open `index.html` in Chrome, Edge, or Opera
2. **Flash Firmware**: 
   - Connect your ESP32-C6 via USB
   - Click "Install ESP32 Storage Firmware"
   - Select your device and wait for flashing to complete
3. **Test Storage**:
   - Switch to "Test Storage" tab
   - Click "Connect to ESP32"
   - Start saving and reading data!

## Requirements

- **Browser**: Chrome, Edge, or Opera (Web Serial API support required)
- **Hardware**: ESP32-C6 (XIAO ESP32C6) or compatible ESP32 board
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
└── README.md
```

## Building from Source

If you want to modify the firmware:

1. Open `firmware/firmware-source/HW_ESP32.ino` in Arduino IDE
2. Install required libraries:
   - SPIFFS
   - ArduinoJson
3. Select your ESP32 board and compile
4. Upload directly or export compiled binaries to `/firmware` folder

## Web Serial API

This project uses the [Web Serial API](https://developer.mozilla.org/en-US/docs/Web/API/Web_Serial_API) to communicate with ESP32 devices directly from the browser at 115200 baud rate.

## Security Note

This is an **experimental project** for educational and development purposes. While it implements persistent storage, it should not be used for storing highly sensitive data without additional security measures.

## License

MIT License - See LICENSE file for details

## Authors

Created by **Neurai Team**  
Website: [https://neurai.org](https://neurai.org)  
Repository: [https://github.com/NeuraiProject/ESP32-HW](https://github.com/NeuraiProject/ESP32-HW)

## Contributing

Contributions, issues, and feature requests are welcome! Feel free to check the issues page.

## Troubleshooting

**Device not appearing in port selection?**
- Make sure you're using a data cable (not charge-only)
- Try a different USB port
- Check if drivers are installed for your ESP32 board

**Connection fails?**
- Ensure no other program is using the serial port
- Reset your ESP32 and try again
- Check that the correct baud rate (115200) is being used

**Firmware flashing fails?**
- Hold the BOOT button on your ESP32 while clicking Install
- Try a different USB cable or port
- Verify your browser supports Web Serial API

---

by Neuraiproject
