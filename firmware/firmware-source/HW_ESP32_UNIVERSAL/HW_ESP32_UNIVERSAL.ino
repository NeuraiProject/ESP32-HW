/*
 * ESP32 Serial Storage (NVS Edition - Universal)
 * Created for Neuraiproject
 * 
 * This firmware converts your ESP32 into a persistent storage device
 * accessible via Web Serial API from any Chrome browser.
 * 
 * Features:
 * - Save/Read/Delete key-value pairs
 * - Persistent storage using NVS (Non-Volatile Storage)
 * - JSON-based communication protocol
 * - Compatible with ALL ESP32 models (ESP32, S2, S3, C3, C6, etc.)
 * - Works with USB CDC on ESP32-S3 and C6
 * - No partition configuration needed - uses built-in NVS
 * 
 * Author: Neurai Team
 * License: MIT
 * Website: https://neurai.org
 * 
 * IMPORTANT FOR ESP32-S3:
 * - Set "USB CDC On Boot: Enabled" in Arduino IDE
 */

#include <Preferences.h>
#include <ArduinoJson.h>

// Preferences instance for NVS storage
Preferences preferences;

// Buffer to receive commands
String inputBuffer = "";
const int MAX_BUFFER = 2048;

// NVS namespace
const char* NVS_NAMESPACE = "neuraistore";

void setup() {
  // Universal Serial configuration for all ESP32 models
  Serial.setTxTimeoutMs(0);
  Serial.begin(115200);
  
  // Wait for Serial to be ready (works for both USB CDC and UART)
  unsigned long startTime = millis();
  while(!Serial && (millis() - startTime) < 5000) {
    delay(10);
  }
  
  // Wait a bit for connection to establish
  delay(2000);
  
  // Startup messages
  Serial.println();
  Serial.println("========================================");
  Serial.println("ESP32 Serial Storage - Starting...");
  Serial.print("Chip Model: ");
  Serial.println(ESP.getChipModel());
  Serial.println("Storage: NVS (Non-Volatile Storage)");
  Serial.println("Created for Neurai Project");
  Serial.println("========================================");
  
  // Initialize NVS - no partition configuration needed!
  Serial.println("Initializing NVS storage...");
  
  if(!preferences.begin(NVS_NAMESPACE, false)){
    Serial.println("{\"status\":\"error\",\"message\":\"NVS initialization failed\"}");
    Serial.println("CRITICAL ERROR: Cannot initialize NVS");
    return;
  }
  
  Serial.println("NVS storage initialized successfully!");
  
  // NVS doesn't have traditional space info, but we can show max key-value limits
  Serial.println("----------------------------------------");
  Serial.println("Storage Type: NVS (Non-Volatile Storage)");
  Serial.println("Max Key Length: 15 characters");
  Serial.println("Max Value Length: 4000 bytes");
  Serial.println("Storage is built-in and always available");
  Serial.println("----------------------------------------");
  
  // Test write capability
  Serial.println("Testing write capability...");
  if (preferences.putString("_test", "OK") > 0) {
    Serial.println("✓ Write test: PASSED");
    String testRead = preferences.getString("_test", "");
    if (testRead == "OK") {
      Serial.println("✓ Read verification: PASSED");
    } else {
      Serial.println("✗ Read verification: FAILED");
    }
    preferences.remove("_test");
  } else {
    Serial.println("✗ Write test: FAILED!");
    Serial.println("WARNING: Storage may not be writable");
  }
  
  // Check for boot counter to verify persistence
  int bootCount = preferences.getInt("_bootcount", 0);
  bootCount++;
  preferences.putInt("_bootcount", bootCount);
  Serial.print("Boot count: ");
  Serial.println(bootCount);
  Serial.println("(If this resets to 1 every boot, NVS is not persistent)");
  
  preferences.end();
  
  // Send ready message
  Serial.println("========================================");
  sendResponse("ready", "ESP32 Serial Storage Ready");
  Serial.println("Waiting for commands...");
}

void loop() {
  // Read data from Serial
  while (Serial.available()) {
    char c = Serial.read();
    
    // Ignore carriage return
    if (c == '\r') {
      continue;
    }
    
    // Process command when newline is received
    if (c == '\n') {
      if (inputBuffer.length() > 0) {
        processCommand(inputBuffer);
        inputBuffer = "";
      }
    } else if (inputBuffer.length() < MAX_BUFFER) {
      // Add character to buffer
      inputBuffer += c;
    } else {
      // Buffer overflow
      sendError("Buffer overflow - command too long");
      inputBuffer = "";
    }
  }
  
  delay(10);
}

void processCommand(String command) {
  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, command);
  
  if (error) {
    Serial.print(">>> Error parsing JSON: ");
    Serial.println(error.c_str());
    sendError("Invalid JSON");
    return;
  }
  
  const char* action = doc["action"];
  
  if (!action) {
    Serial.println(">>> Error: Missing action field");
    sendError("Missing action");
    return;
  }
  
  Serial.print(">>> Action: ");
  Serial.println(action);
  
  if (strcmp(action, "save") == 0) {
    handleSave(doc);
  } else if (strcmp(action, "read") == 0) {
    handleRead(doc);
  } else if (strcmp(action, "delete") == 0) {
    handleDelete(doc);
  } else if (strcmp(action, "list") == 0) {
    handleList();
  } else if (strcmp(action, "clear") == 0) {
    handleClear();
  } else if (strcmp(action, "info") == 0) {
    handleInfo();
  } else if (strcmp(action, "format") == 0) {
    handleFormat();
  } else {
    Serial.print(">>> Error: Unknown action: ");
    Serial.println(action);
    sendError("Unknown action");
  }
}

void handleSave(JsonDocument& doc) {
  const char* key = doc["key"];
  const char* value = doc["value"];
  
  if (!key || !value) {
    sendError("Missing 'key' or 'value' field");
    return;
  }
  
  // Check key length (NVS limit is 15 characters)
  if (strlen(key) > 15) {
    sendError("Key too long (max 15 characters)");
    return;
  }
  
  Serial.print(">>> Saving key: ");
  Serial.println(key);
  Serial.print(">>> Value: ");
  Serial.println(value);
  
  // Open preferences for writing (false = read-write mode)
  if (!preferences.begin(NVS_NAMESPACE, false)) {
    Serial.println(">>> ERROR: Failed to open NVS");
    sendError("Failed to open NVS for writing");
    return;
  }
  
  // Save the value
  Serial.println(">>> Writing value to NVS...");
  size_t bytesWritten = preferences.putString(key, value);
  
  if (bytesWritten == 0) {
    Serial.println(">>> ERROR: putString returned 0 bytes");
    preferences.end();
    sendError("Failed to write data to NVS");
    return;
  }
  
  Serial.print(">>> Bytes written: ");
  Serial.println(bytesWritten);
  
  // Verify the write by reading it back
  String readBack = preferences.getString(key, "");
  Serial.print(">>> Read back verification: ");
  Serial.println(readBack);
  
  if (readBack != String(value)) {
    Serial.println(">>> ERROR: Verification failed!");
    preferences.end();
    sendError("Data verification failed");
    return;
  }
  
  // Update key list for list() command
  Serial.println(">>> Updating key list...");
  String keyList = preferences.getString("_keylist", "");
  if (keyList.indexOf(String(key)) == -1) {
    if (keyList.length() > 0) {
      keyList += ",";
    }
    keyList += String(key);
    preferences.putString("_keylist", keyList);
  }
  
  // IMPORTANT: Commit changes before closing
  Serial.println(">>> Committing changes...");
  delay(100); // Give NVS time to write
  
  preferences.end();
  
  Serial.println(">>> Save successful and verified!");
  sendResponse("saved", String("Data saved for key: ") + key);
}

void handleRead(JsonDocument& doc) {
  const char* key = doc["key"];
  
  if (!key) {
    sendError("Missing 'key' field");
    return;
  }
  
  // Open preferences for reading
  if (!preferences.begin(NVS_NAMESPACE, true)) {
    sendError("Failed to open NVS for reading");
    return;
  }
  
  // Check if key exists
  if (preferences.isKey(key)) {
    String value = preferences.getString(key, "");
    preferences.end();
    
    StaticJsonDocument<512> response;
    response["status"] = "success";
    response["key"] = key;
    response["value"] = value;
    
    String output;
    serializeJson(response, output);
    Serial.println(output);
  } else {
    preferences.end();
    sendError(String("Key not found: ") + key);
  }
}

void handleDelete(JsonDocument& doc) {
  const char* key = doc["key"];
  
  if (!key) {
    sendError("Missing 'key' field");
    return;
  }
  
  // Open preferences for writing
  if (!preferences.begin(NVS_NAMESPACE, false)) {
    sendError("Failed to open NVS");
    return;
  }
  
  if (preferences.isKey(key)) {
    bool success = preferences.remove(key);
    
    // Update key list
    String keyList = preferences.getString("_keylist", "");
    keyList.replace(String(key) + ",", "");
    keyList.replace("," + String(key), "");
    keyList.replace(String(key), "");
    preferences.putString("_keylist", keyList);
    
    preferences.end();
    
    if (success) {
      sendResponse("deleted", String("Data deleted for key: ") + key);
    } else {
      sendError("Failed to delete key");
    }
  } else {
    preferences.end();
    sendError(String("Key not found: ") + key);
  }
}

void handleList() {
  // Open preferences for reading
  if (!preferences.begin(NVS_NAMESPACE, true)) {
    sendError("Failed to open NVS");
    return;
  }
  
  // NVS doesn't have a built-in list function, so we maintain a key list
  String keyList = preferences.getString("_keylist", "");
  preferences.end();
  
  StaticJsonDocument<2048> response;
  response["status"] = "success";
  JsonArray keys = response.createNestedArray("keys");
  
  if (keyList.length() > 0) {
    // Parse comma-separated key list
    int startPos = 0;
    int commaPos = keyList.indexOf(',');
    
    while (commaPos != -1) {
      String key = keyList.substring(startPos, commaPos);
      if (key.length() > 0 && key != "_keylist") {
        keys.add(key);
      }
      startPos = commaPos + 1;
      commaPos = keyList.indexOf(',', startPos);
    }
    
    // Add last key
    String lastKey = keyList.substring(startPos);
    if (lastKey.length() > 0 && lastKey != "_keylist") {
      keys.add(lastKey);
    }
  }
  
  String output;
  serializeJson(response, output);
  Serial.println(output);
}

void handleClear() {
  // Open preferences for writing
  if (!preferences.begin(NVS_NAMESPACE, false)) {
    sendError("Failed to open NVS");
    return;
  }
  
  bool success = preferences.clear();
  preferences.end();
  
  if (success) {
    sendResponse("cleared", "All data cleared successfully");
  } else {
    sendError("Failed to clear data");
  }
}

void handleInfo() {
  StaticJsonDocument<512> response;
  response["status"] = "success";
  
  // Storage info
  response["storage"]["type"] = "NVS";
  response["storage"]["max_key_length"] = 15;
  response["storage"]["max_value_length"] = 4000;
  response["storage"]["namespace"] = NVS_NAMESPACE;
  
  // Chip info - automatically detected
  response["chip"]["model"] = ESP.getChipModel();
  response["chip"]["revision"] = ESP.getChipRevision();
  response["chip"]["cores"] = ESP.getChipCores();
  response["chip"]["frequency"] = ESP.getCpuFreqMHz();
  
  String output;
  serializeJson(response, output);
  Serial.println(output);
}

void handleFormat() {
  Serial.println(">>> Clearing all NVS data...");
  sendResponse("formatting", "Clearing all storage, please wait...");
  
  if (!preferences.begin(NVS_NAMESPACE, false)) {
    sendError("Failed to open NVS");
    return;
  }
  
  bool success = preferences.clear();
  preferences.end();
  
  if (success) {
    Serial.println(">>> Clear successful!");
    sendResponse("formatted", "All storage cleared successfully");
  } else {
    Serial.println(">>> Clear failed!");
    sendError("Failed to clear storage");
  }
}

void sendResponse(const char* status, String message) {
  StaticJsonDocument<256> response;
  response["status"] = status;
  response["message"] = message;
  
  String output;
  serializeJson(response, output);
  Serial.println(output);
}

void sendError(const char* message) {
  StaticJsonDocument<256> response;
  response["status"] = "error";
  response["message"] = message;
  
  String output;
  serializeJson(response, output);
  Serial.println(output);
}

void sendError(String message) {
  sendError(message.c_str());
}
