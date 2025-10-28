/*
 * ESP32 Serial Storage
 * Created for Neuraiproject
 * 
 * This firmware converts your ESP32 into a persistent storage device
 * accessible via Web Serial API from any Chrome browser.
 * 
 * Features:
 * - Save/Read/Delete key-value pairs
 * - Persistent storage using SPIFFS
 * - JSON-based communication protocol
 * - Compatible with all ESP32 models
 * 
 * Author: Neurai Team
 * License: MIT
 * Website: https://neurai.org
 */

#include <SPIFFS.h>
#include <ArduinoJson.h>

// Buffer to receive commands
String inputBuffer = "";
const int MAX_BUFFER = 2048;

// Storage file
const char* STORAGE_FILE = "/storage.json";

void setup() {
  // Initialize Serial (works with all ESP32 models)
  Serial.begin(115200);
  
  // Wait a bit for connection to establish
  delay(2000);
  
  // Startup messages
  Serial.println();
  Serial.println("========================================");
  Serial.println("ESP32 Serial Storage - Starting...");
  Serial.println("Created for Neurai Project");
  Serial.println("========================================");
  
  // Initialize SPIFFS
  if(!SPIFFS.begin(true)){
    Serial.println("{\"status\":\"error\",\"message\":\"SPIFFS mount failed\"}");
    return;
  }
  
  Serial.println("SPIFFS mounted successfully");
  
  // Show available space
  size_t totalBytes = SPIFFS.totalBytes();
  size_t usedBytes = SPIFFS.usedBytes();
  Serial.print("SPIFFS - Total: ");
  Serial.print(totalBytes);
  Serial.print(" bytes, Used: ");
  Serial.print(usedBytes);
  Serial.println(" bytes");
  
  // Send ready message
  Serial.println("========================================");
  sendResponse("ready", "ESP32 Serial Storage Ready");
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
  
  Serial.print(">>> Acción: ");
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
  
  // Read existing data
  StaticJsonDocument<4096> storage;
  File file = SPIFFS.open(STORAGE_FILE, "r");
  if (file) {
    DeserializationError error = deserializeJson(storage, file);
    file.close();
    
    if (error && error != DeserializationError::EmptyInput) {
      sendError("Error reading existing storage");
      return;
    }
  }
  
  // Add new data
  storage[key] = value;
  
  // Save to file
  file = SPIFFS.open(STORAGE_FILE, "w");
  if (!file) {
    sendError("Failed to open file for writing");
    return;
  }
  
  serializeJson(storage, file);
  file.close();
  
  sendResponse("saved", String("Data saved for key: ") + key);
}

void handleRead(JsonDocument& doc) {
  const char* key = doc["key"];
  
  if (!key) {
    sendError("Missing 'key' field");
    return;
  }
  
  File file = SPIFFS.open(STORAGE_FILE, "r");
  if (!file) {
    sendError("No data found - storage is empty");
    return;
  }
  
  StaticJsonDocument<4096> storage;
  DeserializationError error = deserializeJson(storage, file);
  file.close();
  
  if (error) {
    sendError("Error reading storage file");
    return;
  }
  
  if (storage.containsKey(key)) {
    StaticJsonDocument<512> response;
    response["status"] = "success";
    response["key"] = key;
    response["value"] = storage[key].as<String>();
    
    String output;
    serializeJson(response, output);
    Serial.println(output);
  } else {
    sendError(String("Key not found: ") + key);
  }
}

void handleDelete(JsonDocument& doc) {
  const char* key = doc["key"];
  
  if (!key) {
    sendError("Missing 'key' field");
    return;
  }
  
  File file = SPIFFS.open(STORAGE_FILE, "r");
  if (!file) {
    sendError("No data found - storage is empty");
    return;
  }
  
  StaticJsonDocument<4096> storage;
  DeserializationError error = deserializeJson(storage, file);
  file.close();
  
  if (error) {
    sendError("Error reading storage file");
    return;
  }
  
  if (storage.containsKey(key)) {
    storage.remove(key);
    
    file = SPIFFS.open(STORAGE_FILE, "w");
    if (!file) {
      sendError("Failed to open file for writing");
      return;
    }
    
    serializeJson(storage, file);
    file.close();
    
    sendResponse("deleted", String("Data deleted for key: ") + key);
  } else {
    sendError(String("Key not found: ") + key);
  }
}

void handleList() {
  File file = SPIFFS.open(STORAGE_FILE, "r");
  
  StaticJsonDocument<4096> storage;
  if (file) {
    DeserializationError error = deserializeJson(storage, file);
    file.close();
    
    if (error && error != DeserializationError::EmptyInput) {
      sendError("Error reading storage file");
      return;
    }
  }
  
  StaticJsonDocument<2048> response;
  response["status"] = "success";
  JsonArray keys = response.createNestedArray("keys");
  
  for (JsonPair kv : storage.as<JsonObject>()) {
    keys.add(kv.key().c_str());
  }
  
  String output;
  serializeJson(response, output);
  Serial.println(output);
}

void handleClear() {
  if (SPIFFS.remove(STORAGE_FILE)) {
    sendResponse("cleared", "All data cleared successfully");
  } else {
    sendError("Failed to clear data");
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