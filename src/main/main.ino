
// please choose board before uploading
#define LILYGO
// #define HELTEC

#define FIXED_STA
//#define MOBILE_AP


// modifying PJON parameters
#define PJON_MAX_PACKETS 50
#define PJON_PACKET_MAX_LENGTH 200

#include "Longifi_pin_util.h"
#include "Longifi_storage.h"
#include "Longifi_screen.h"
#include "Longifi_database.h"
#include "Longifi_wifi.h"
#include "Longifi_pjon.h"
#include "Longifi_webserver.h"
#include "Longifi_chat_app.h"
#include "Longifi_dashboard.h"
#include "Longifi_search.h"
#include <SPIFFS_Shell.h>

void pjonOnTopicTest(String *data, int len){
  Serial.println("Received a message from PJON");
  for(int i = 0; i < len; i++)
    Serial.println(data[i]);
}

void setup() {
  Serial.begin(115200);
  initStorage();
  initDatabase();
  initScreen();
  initPjon();

  initWifi();
  initChatApp();
  initDashboard();
  initSearch();
  initWebserver();

  addPjonReceiveTopicListener("test", pjonOnTopicTest);
  // send pjon packet with event "test"
  sendPjonTopicMessage("test", "hello PJON!");

}

void loop() {
  loopPjon();
  loopDatabase();
}