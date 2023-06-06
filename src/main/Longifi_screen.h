#ifndef SCREEN_H
#define SCREEN_H

#if defined(HELTEC)

#include <heltec.h>
#include "Arduino.h"
#define Display (*(Heltec.display))


#elif defined(LILYGO)

#include <Wire.h>
#include <SSD1306Wire.h>

SSD1306Wire Display(0x3c, SDA, SCL);

#endif

void displayOnWIFIPart();
void displayCrdentials();
void displayOnLoRaPart(String status);

String wifiStrings[4];
String loraString = "LoRa Area";


void displayAll(){

  Display.clear();
  Display.setFont(ArialMT_Plain_10);
  Display.drawStringMaxWidth(0, 0, 128, wifiStrings[0] );
  Display.drawStringMaxWidth(0, 12, 128, wifiStrings[1] );
  Display.drawStringMaxWidth(0, 24, 128, wifiStrings[2] );
  Display.drawStringMaxWidth(0, 36, 128, wifiStrings[3] );
  Display.drawStringMaxWidth(0, 48, 128, loraString);
  Display.display();
  delay(1000);
}

void displayOnWIFIPart(String string1="",String string2="",String string3="",String string4=""){
  wifiStrings[0]=string1;
  wifiStrings[1]=string2;
  wifiStrings[2]=string3;
  wifiStrings[3]=string4;
  displayAll();
}
void initScreen()
{
	Serial.begin(115200);

  #if defined(HELTEC)
	Serial.println("Initializing Heltec screen...");
	Heltec.begin(true, false, true);
  Display.drawStringMaxWidth(0, 0, 128,"Initializing Heltec screen...");

  #elif defined(LILYGO)
	Serial.println("Initializing LilyGO screen...");
	Display.init();
	Display.flipScreenVertically();
  Display.drawStringMaxWidth(0, 0, 128,"Initializing lilyGO screen...");
  #endif

	delay(1000);
	Display.clear();

}


void displayOnLoRaPart(String status){
  loraString=status;
  displayAll();
}


void loopScreen()
{
  displayOnLoRaPart("LoRa Area");
}
#endif

