#ifndef LONGIFI_SD_H
#define LONGIFI_SD_H

#include "Longifi_pin_util.h"

#include <SD.h>
#include <SPIFFS.h>

#include <FS.h>
#include <SPI.h>

#ifdef HAS_SDCARD
#define Storage SD
#else
#define Storage SPIFFS
#endif

#define SD_SPI_SPEED 26 * 1000000UL
#define FORMAT_SPIFFS_IF_FAILED true

SPIClass SDSPI(HSPI);

void initSdcard(){
  
  SDSPI.begin(SDCARD_SCLK, SDCARD_MISO, SDCARD_MOSI, SDCARD_CS);
  if (!SD.begin(SDCARD_CS, SDSPI, SD_SPI_SPEED, "/sd", 20)) {
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
    Serial.println("No SD card attached");
    return;
  }

  Serial.print("SD Card Type: ");
  if(cardType == CARD_MMC){
    Serial.println("MMC");
  } else if(cardType == CARD_SD){
    Serial.println("SDSC");
  } else if(cardType == CARD_SDHC){
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);
  
}

void initFlashMemory(){
  
    if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)){
        Serial.println("SPIFFS Mount Failed");
        return;
    }
    uint64_t size = SPIFFS.totalBytes() / 1024;
    Serial.printf("Flash memory Card Size: %lluKB\n", size);
    return;
}

void initStorage(){
  #ifdef HAS_SDCARD
  initSdcard();
  #else
  initFlashMemory();
  #endif
}

#endif