#ifndef LONGIFI_DATABASE_H
#define LONGIFI_DATABASE_H

#include "Longifi_storage.h"
#ifndef ArduinoJson
#include <ArduinoJson.h>
#endif

#define DATABASE_MAX_SIZE  50 * 1024 // 50kB
#define DATABASE_FILE_PATH  "/config.json"
// #define DATABASE_TMP_PATH  "/tmp.jsonl"

DynamicJsonDocument memoryDatabase(DATABASE_MAX_SIZE);
bool isDatabaseChanged = false;

void saveDatabase(){
  File file = Storage.open(DATABASE_FILE_PATH, FILE_WRITE);
  if(!file){
      Serial.println("- failed to open file for writing");
      return;
  }

  String str;
  serializeJson(memoryDatabase, str);
  if(!file.print(str.c_str()))
      Serial.println("- failed to save database");
}

void readDatabase(){
  
  File file = Storage.open(DATABASE_FILE_PATH);
  if(!file){
      Serial.println("- failed to open file for reading");
      return;
  }

  String str = "";
  while (file.available()) {
      char c = file.read();
      str += c;
  }

  DeserializationError error = deserializeJson(memoryDatabase, str);

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }
}

//void saveDatabasePeriodically(int periodInMillis){
//  static uint64_t lastSaved = 0;
//  if(isDatabaseChanged && millis() - lastSaved > periodInMillis){
//    saveDatabase();
//    lastSaved = millis();
//  }
//}

void initDatabase(){
  readDatabase();
}

void loopDatabase(){
//  saveDatabasePeriodically(DATABASE_SAVE_PERIOD);
}

bool isKeyArray(const String& key) {
  return memoryDatabase.containsKey(key) && memoryDatabase[key].is<JsonArray>();
}

bool saveInDatabase(const String& key, const String& value) {
  if (isKeyArray(key)) {
    Serial.println("Error: Key is an array, cannot save a single value");
    return false;
  }

  if (memoryDatabase[key] != value) {
    memoryDatabase[key] = value;
    isDatabaseChanged = true;
    saveDatabase();
    return true;
  }

  return false;
}

bool saveInDatabase(const String& key, const String& value1, const String& value2) {
  if (memoryDatabase.containsKey(key) && !isKeyArray(key)) {
    Serial.println("Error: Key is not an array, cannot save multiple values");
    return false;
  }

  if (memoryDatabase[key][0] != value1 || memoryDatabase[key][1] != value2) {
    memoryDatabase[key][0] = value1;
    memoryDatabase[key][1] = value2;
    isDatabaseChanged = true;
    return true;
  }

  return false;
}

String getFromDatabase(const String& key) {
  if (memoryDatabase.containsKey(key)) {
    if (isKeyArray(key)) {
      Serial.println("Error: Key is an array, cannot retrieve as a single value");
      return "";
    }

    return memoryDatabase[key].as<String>();
  }

  return "";
}

int getFromDatabase(const String& key, String* array, int maxLength) {
  if (memoryDatabase.containsKey(key)) {
    if (!isKeyArray(key)) {
      Serial.println("Error: Key is not an array, cannot retrieve as an array");
      return 0;
    }

    JsonArray arr = memoryDatabase[key].as<JsonArray>();
    int length = (arr.size() < maxLength)? arr.size(): maxLength;

    for (int i = 0; i < length; i++) {
      array[i] = arr[i].as<String>();
    }

    return length;
  }

  return 0;
}

bool addToArray(const String& key, const String& value) {
  if (!isKeyArray(key)) {
    Serial.println("Error: Key is not an array, cannot add element");
    return false;
  }

  JsonArray arr = memoryDatabase[key].as<JsonArray>();
  arr.add(value);
  isDatabaseChanged = true;
  return true;
}

bool removeFromArray(const String& key, int index = -1) {
  if (!isKeyArray(key)) {
    Serial.println("Error: Key is not an array, cannot remove element");
    return false;
  }

  JsonArray arr = memoryDatabase[key].as<JsonArray>();
  if(index < 0) index += arr.size();
  if (index >= 0 && index < arr.size()) {
    arr.remove(index);
    isDatabaseChanged = true;
    saveDatabase();
    return true;
  }

  return false;
}

#endif
