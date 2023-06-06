#ifndef LONGIFI_WEBSERVER_H
#define LONGIFI_WEBSERVER_H

#include "Longifi_pin_util.h"

#include "Longifi_wifi.h"

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

void initWebserver(){
  server.serveStatic("/", Storage, "/").setDefaultFile("index.html");

  Serial.println("Server initialized.");
  server.begin();
}

#endif