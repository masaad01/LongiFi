#ifndef LONGIFI_WIFI_H
#define LONGIFI_WIFI_H

#include "Longifi_screen.h"
#include "Longifi_pin_util.h"
#include "Longifi_storage.h"
#include <WiFi.h>

String WiFissid = "AA";
String WiFipass = "123456789";
String APssid     = "AP";
String APpass = "123456789";

void workAsFixedSTA(String ssid,String password);
void workAsMobileAP(String ssid,String password);
void changeSTACrdentials();
void changeAPCrdentials();

String getActiveMode(){
  return getFromDatabase("ACTIVE_MODE");
}

void initWifi() {
  APssid = getFromDatabase("MOBILE_AP_SSID");
  APpass = getFromDatabase("MOBILE_AP_PASS");
  WiFissid = getFromDatabase("FIXED_STA_SSID");
  WiFipass = getFromDatabase("FIXED_STA_PASS");
  if( getActiveMode() == "FIXED_STA")
  workAsFixedSTA(WiFissid,WiFipass);
  
  else 
    workAsMobileAP(APssid,APpass);
  
  
}
void workAsFixedSTA(String ssid,String password) {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }

  displayCrdentials();
}
void workAsMobileAP(String ssid,String password){
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);
    displayCrdentials(); 
}
void changeSTACrdentials(){
  WiFissid = getFromDatabase("FIXED_STA_SSID");
  WiFipass = getFromDatabase("FIXED_STA_PASS");
  workAsFixedSTA(WiFissid,WiFipass);
}
void changeAPCrdentials()
{ 
  APssid = getFromDatabase("MOBILE_AP_SSID");
  APpass = getFromDatabase("MOBILE_AP_PASS");
  workAsMobileAP(APssid,APpass);
}
void displayCrdentials()
{

  if( getActiveMode() == "FIXED_STA"){
    displayOnWIFIPart("Connect Succeed!");
    displayOnWIFIPart("STA_IP:" + WiFi.localIP().toString());
  }
  else {
    //APssid = getFromDatabase("MOBILE_AP_SSID");
    //APpass = getFromDatabase("MOBILE_AP_PASS");
    displayOnWIFIPart("Creating AP...");
    displayOnWIFIPart("AP created...");
    displayOnWIFIPart(("SSID: " + APssid),("PASS: " + APpass),"IP: "+ WiFi.softAPIP().toString());
    
  }
}

String getActiveSSID(){
  if( getActiveMode() == "FIXED_STA")
    return getFromDatabase("FIXED_STA_SSID");
  return getFromDatabase("MOBILE_AP_SSID");
}
String getActivePassword(){
  if(getActiveMode() == "FIXED_STA")
    return getFromDatabase("FIXED_STA_PASS");
  return getFromDatabase("MOBILE_AP_PASS");
}
String getServerIP(){
  if(getActiveMode() == "FIXED_STA")
    return (WiFi.localIP().toString());
  return WiFi.softAPIP().toString();
}

#endif