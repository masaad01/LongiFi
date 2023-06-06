#ifndef LONGIFI_CHAT_APP_H
#define LONGIFI_CHAT_APP_H

#include "Longifi_webserver.h"
#include "Longifi_pjon.h"
#include "Longifi_storage.h"

#ifndef ArduinoJson
#include <ArduinoJson.h>
#endif
#ifndef Vector
#include <Vector.h>
#endif

#define CHAT_APP_PJON_TOPIC   "ChatApp"

#ifdef HAS_SDCARD
#define CHAT_APP_STORAGE_PATH   "/chat_app/"
#define CHAT_APP_DEFAULT_FILE   "index.html"
#else
#define CHAT_APP_STORAGE_PATH   "/"
#define CHAT_APP_DEFAULT_FILE   "chatapp__index.html"
#endif

#define CHAT_APP_URL_PATH       "/chat"


AsyncWebSocket socket("/");


struct User{
  int id = 0;
  String name = "";
};
// User usersArray[100];
// Vector<User> onlineUsers(usersArray);
int usersNum = 0;

void parseJsonMessage(char* jsonStr, String &type, String &data1, String &data2){
  StaticJsonDocument<255> doc;
  DeserializationError error = deserializeJson(doc, jsonStr);
  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }
  //Json format: {"type":"", "data1":"", "data2":""}
  type = String(doc["type"].as<const char *>());
  data1 = String(doc["data1"].as<const char *>());
  if(doc.containsKey("data2"))
    data2 = String(doc["data2"].as<const char *>());

}

void webSocketSendAll(String type, String data1, String data2 = ""){
  StaticJsonDocument<255> doc;
  doc["type"] = type;
  doc["data1"] = data1;
  if(data2 != "")
    doc["data2"] = data2;

  String jsonStr;
  serializeJson(doc, jsonStr);
  socket.textAll(jsonStr);

  Serial.println("Websocket sending: " + jsonStr);
}

void chatAppSendAll(bool sendToPjon, String type, String data1, String data2 = ""){
  webSocketSendAll(type, data1, data2);
  if(sendToPjon)
    sendPjonTopicMessage(CHAT_APP_PJON_TOPIC, type, data1, data2);
}

void handleChatAppMessage(bool fromPjon, String type, String data1, String data2){
  
  if(type == "new-user"){
      chatAppSendAll(!fromPjon, "broadcast", "Online: " + String(usersNum));
      User usr;
      usr.id = 1;
      usr.name = data1;
      // onlineUsers.push_back(usr);
      chatAppSendAll(!fromPjon, "user-connected", data1);
  }
  else if(type == "new-message"){
    chatAppSendAll(!fromPjon, "new-message", data1, data2);
  }
  else if(type == "is-typing"){
    chatAppSendAll(false, "is-typing", data1); // do not send to pjon (to reduce traffic abd conserve bandwidth)
  }
  else{
    Serial.println("unknown chat message type: " + String(type));
  }
}

void handlePjonChatAppMessage(String *data, int length){
  Serial.println("received PJON Chat App message!");
  handleChatAppMessage(true, data[0], data[1], data[2]);
}

void webSocketHandleMessage(void *arg, uint8_t *data, size_t len) {
  // Serial.println("recieved msg");
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = '\0'; // put a null at the end to terminate the string

    String type = "";
    String data1 = "";
    String data2 = "";

    parseJsonMessage((char*)data, type, data1, data2);

    handleChatAppMessage(false, type, data1, data2);
  }
}

void webSocketOnEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      usersNum++;
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      usersNum--;
      break;
    case WS_EVT_DATA:
      webSocketHandleMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void initWebSocket() {
  socket.onEvent(webSocketOnEvent);
  server.addHandler(&socket);
}

void initChatApp(){
  server.serveStatic(CHAT_APP_URL_PATH, Storage, CHAT_APP_STORAGE_PATH).setDefaultFile(CHAT_APP_DEFAULT_FILE);
  initWebSocket();
  addPjonReceiveTopicListener(CHAT_APP_PJON_TOPIC, handlePjonChatAppMessage);
}

#endif