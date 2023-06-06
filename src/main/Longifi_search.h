#ifndef LONGIFI_SEARCH_H
#define LONGIFI_SEARCH_H

#include "Longifi_webserver.h"
#include "Longifi_pjon.h"
#include "Longifi_wifi.h"
#include "Longifi_database.h"

#ifndef ArduinoJson
#include <ArduinoJson.h>
#endif
#include <HTTPClient.h>

#define SEARCH_PJON_TOPIC "SEARCH"

#ifdef HAS_SDCARD
#define SEARCH_STORAGE_PATH "/search/"
#define SEARCH_DEFAULT_FILE "index.html"
#else
#define SEARCH_STORAGE_PATH "/"
#define SEARCH_DEFAULT_FILE "search__index.html"
#endif

#define SEARCH_URL_PATH "/search"

#define MAX_JSON_RESPONSE_SIZE 10 * 1024  // 10KB

#define GOOGLE_CUSTOME_SEARCH_API_ID    "0425e195bc3894140"
#define GOOGLE_CUSTOME_SEARCH_API_KEY   "AIzaSyC6P6cSAol9WtXiYD1hfwwmbyzLrOgDPoY"

// data flow: searchQuery -> sendQueryToPjon -> sendQueryToWeb -> searchResults -> filterJsonResponse -> sendResultsToPjon -> getFormatedSearchResults -> searchProcessor -> CONTENT

// function declarations
String urlencode(String str);
String sendQueryToWeb(String query);  // function to send query to google custom search api
String filterJsonResponse(String json);
String getFormatedSearchResults(String json);
void handlePjonSearchQuery(String *data, int length);
String searchProcessor(const String &var);
void initSearch();
void waitForSearchResults(int timeout = 10000);
String formatSearchResult(String, String, String);

// global variables
String searchQuery = "";
String searchResults = "";

// function definitions

void initSearch() {

  server.serveStatic(SEARCH_URL_PATH, Storage, SEARCH_STORAGE_PATH);

  server.on(SEARCH_URL_PATH, HTTP_GET, [](AsyncWebServerRequest *request) {
    searchQuery = "";
    searchResults = "";
    request->send(Storage, String(SEARCH_STORAGE_PATH) + SEARCH_DEFAULT_FILE, String(), false, searchProcessor);
  });

  addPjonReceiveTopicListener(SEARCH_PJON_TOPIC, handlePjonSearchQuery);

  server.on("/search", HTTP_POST, [](AsyncWebServerRequest *request) {
    int params = request->params();
    for (int i = 0; i < params; i++) {
      AsyncWebParameter *p = request->getParam(i);
      if (p->isPost()) {
        // HTTP POST query value
        if (p->name() == "query") {
          searchQuery = p->value().c_str();
        }
      }
    }

    searchResults = "";
    if (getActiveMode() == "FIXED_STA") {
      searchResults = sendQueryToWeb(searchQuery);
      searchResults = filterJsonResponse(searchResults);
    } else {
      sendPjonTopicMessage(SEARCH_PJON_TOPIC, "req", searchQuery);
      waitForSearchResults();
    }
    if (searchResults != "")
      searchResults = getFormatedSearchResults(searchResults);
    else
      searchResults = "No results found";
    request->send(Storage, String(SEARCH_STORAGE_PATH) + SEARCH_DEFAULT_FILE, String(), false, searchProcessor);
  });
}

String searchProcessor(const String &var) {
  // Serial.println(var);
  // Serial.println(searchQuery);
  // Serial.println(searchResults);
  if (searchQuery == "")
    return String();
  if (var == "RESULTS")
    return searchResults;
  if (var == "QUERY")
    return searchQuery;
  return String();
}

void handlePjonSearchQuery(String *data, int length) {
  if (length != 2) {
    Serial.println("Invalid PJON message");
    return;
  }
  if (data[0] == "req") {
    String res = sendQueryToWeb(data[1]);
    res = filterJsonResponse(res);
    sendPjonTopicMessage(SEARCH_PJON_TOPIC, "res", res);
  } else if (data[0] == "res")
    searchResults = data[1];
  else
    Serial.println("Invalid PJON message");
}

String urlencode(String str) {
  String encodedString = "";
  char c;
  char code0;
  char code1;
  for (int i = 0; i < str.length(); i++) {
    c = str.charAt(i);
    if (c == ' ') {
      encodedString += '+';
    } else if (isalnum(c)) {
      encodedString += c;
    } else {
      code1 = (c & 0xf) + '0';
      if ((c & 0xf) > 9) {
        code1 = (c & 0xf) - 10 + 'A';
      }
      c = (c >> 4) & 0xf;
      code0 = c + '0';
      if (c > 9) {
        code0 = c - 10 + 'A';
      }
      encodedString += '%';
      encodedString += code0;
      encodedString += code1;
    }
    yield();
  }
  return encodedString;
}

String sendQueryToWeb(String query) {
  String cx = GOOGLE_CUSTOME_SEARCH_API_ID;
  String apiKey = GOOGLE_CUSTOME_SEARCH_API_KEY;
  String url = "https://customsearch.googleapis.com/customsearch/v1?key=" + apiKey + "&cx=" + cx + "&q=";

  HTTPClient http;
  http.useHTTP10(true);
  http.begin(url + urlencode(query));
  int httpCode = http.GET();
  Serial.println(httpCode);
  return http.getString();
}

String filterJsonResponse(String json) {
  StaticJsonDocument<100> filter;
  filter["items"][0]["htmlTitle"] = true;
  filter["items"][0]["link"] = true;
  filter["items"][0]["htmlSnippet"] = true;

  StaticJsonDocument<MAX_JSON_RESPONSE_SIZE> doc;
  DeserializationError error = deserializeJson(doc, json, DeserializationOption::Filter(filter));
  if (error) {
    Serial.print(F("filtering - deserializeJson() failed: "));
    Serial.println(error.c_str());
    return "";
  }

  String filteredJson;
  serializeJson(doc, filteredJson);
  return filteredJson;
}

String getFormatedSearchResults(String json) {
  StaticJsonDocument<MAX_JSON_RESPONSE_SIZE> doc;
  DeserializationError error = deserializeJson(doc, json);
  if (error) {
    Serial.print(F("Formatting - deserializeJson() failed: "));
    Serial.println(error.c_str());
    return "";
  }

  String res = "";
  JsonArray items = doc["items"];
  for (JsonObject item : items) {
    String title = item["htmlTitle"];
    String url = item["link"];
    String snippet = item["htmlSnippet"];
    res += formatSearchResult(title, url, snippet);
  }
  return res;
}

String formatSearchResult(String title, String url, String snippet) {
  String res = "";
  res += "<div class=\"search-result\">";
  res += "<a href=\"" + url + "\">";
  res += title;
  res += "</a>";
  res += "<p>" + snippet + "</p>";
  res += "</div>";
  return res;
}

void waitForSearchResults(int timeout) {
  int time = 0;
  while (searchResults == "" && time < timeout) {
    delay(100);
    time += 100;
  }
}
#endif