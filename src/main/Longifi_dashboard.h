#ifndef LONGIFI_DASHBOARD_H
#define LONGIFI_DASHBOARD_H

#include "Longifi_webserver.h"
#include "Longifi_pjon.h"
#include "Longifi_wifi.h"
#include "Longifi_storage.h"


#define DASHBOARD_PJON_TOPIC   "Dashboard"

#ifdef HAS_SDCARD
#define DASHBOARD_STORAGE_PATH   "/dashboard/"
#define DASHBOARD_DEFAULT_FILE   "index.html"
#else
#define DASHBOARD_STORAGE_PATH   "/"
#define DASHBOARD_DEFAULT_FILE   "dashboard__index.html"
#endif

#define DASHBOARD_URL_PATH       "/dashboard"

String dashboardProcessor(const String& var){
  if(var == "ACTIVE_MODE")
    return getFromDatabase("ACTIVE_MODE");  
  if(var == "SSID")
    return getActiveSSID();
  if(var == "PASSWORD")
    return getActivePassword();
  if(var == "IP")
    return getServerIP();
  if(var == "LORA_RSSI")
    return getLoraRssi();
  if(var == "LORA_LAST")
    return getLastConnectionTime() + " seconds";
  if(var == "LORA_PACKETS_SENT")
    return getTotalPacketsSent();
  if(var == "LORA_PACKETS_RECEIVED")
    return getTotalPacketsReceived();
  if(var == "LORA_PACKETS_QUEUED")
    return getTotalPacketsQueued();
  
  return String();
}


void initDashboard(){
  
  server.serveStatic(DASHBOARD_URL_PATH, Storage, DASHBOARD_STORAGE_PATH);
  
  server.on("/dashboard", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(Storage, String(DASHBOARD_STORAGE_PATH)+ DASHBOARD_DEFAULT_FILE, String(), false, dashboardProcessor);
  });

  server.on("/dashboard/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(Storage, String(DASHBOARD_STORAGE_PATH)+ "style.css");
  });
  // addPjonReceiveTopicListener(CHAT_APP_PJON_TOPIC, handlePjonChatAppMessage);
  
  server.on("/dashboard/ap", HTTP_POST, [](AsyncWebServerRequest *request) {
    int params = request->params();
    String ssid;
    String password;
    for(int i=0;i<params;i++){
      AsyncWebParameter* p = request->getParam(i);
      if(p->isPost()){
        // HTTP POST ssid value
        if (p->name() == "ssid") {
          ssid = p->value().c_str();
          saveInDatabase("MOBILE_AP_SSID", ssid);
        }
        // HTTP POST pass value
        if (p->name() == "pass") {
          password = p->value().c_str();
          saveInDatabase("MOBILE_AP_PASS", password);
        }
        saveInDatabase("ACTIVE_MODE","MOBILE_AP");
        //Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
      }
    }
    request->redirect("/dashboard");
    changeAPCrdentials();
  });
  
  server.on("/dashboard/sta", HTTP_POST, [](AsyncWebServerRequest *request) {
    int params = request->params();
    String ssid;
    String password;
    for(int i=0;i<params;i++){
      AsyncWebParameter* p = request->getParam(i);
      if(p->isPost()){
        // HTTP POST ssid value
        if (p->name() == "ssid") {
          ssid = p->value().c_str();
          saveInDatabase("FIXED_STA_SSID", ssid);
        }
        // HTTP POST pass value
        if (p->name() == "pass") {
          password = p->value().c_str();
          saveInDatabase("FIXED_STA_PASS", password);
        }
        saveInDatabase("ACTIVE_MODE","FIXED_STA");
        //Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
      }
    }
    request->redirect("/dashboard");
    changeSTACrdentials();
  });
}

#endif
