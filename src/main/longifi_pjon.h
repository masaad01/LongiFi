#ifndef LONGIFI_PJON_H
#define LONGIFI_PJON_H

#include "Longifi_pin_util.h"
#include "Longifi_pjon_utils.h"
#include "Longifi_storage.h"
#include "Longifi_screen.h"

#ifndef ArduinoJson
#include <ArduinoJson.h>
#endif
#ifndef Vector
#include <Vector.h>
#endif

#include <PJONThroughLora.h>

#define LORA_FREQUENCY				868000000UL
//#define LORA_BANDWIDTH				125E3 //Supported values are 7.8E3, 10.4E3, 15.6E3, 20.8E3, 31.25E3, 41.7E3, 62.5E3, 125E3, 250E3, 500E3
//#define LORA_SPREADING_FACTOR		7   //Supported values are between 6 and 12. If a spreading factor of 6 is set, implicit header mode must be used to transmit and receive packets
//#define LORA_TX_POWER         20
#define LORA_CODING_RATE			5     //Supported values are between 5 and 8, these correspond to coding rates of 4/5 and 4/8
#define LORA_SYNC_WORD				0x12


PJONThroughLora pjonLora(255);

struct PjonReceiveTopicListener{
  int id = 0;
  String topicName = "";
  void (*callback)(String *, int);
};

PjonReceiveTopicListener topicListenersArr[20];
Vector<PjonReceiveTopicListener> topicListeners(topicListenersArr);

uint64_t totalPacketsSent = 0;
uint64_t totalPacketsReceived = 0;
uint64_t lastConnectionTimestamp = 0;

uint64_t totalPings = 0;
uint64_t receivedPings = 0;
String pingRole = "";

void receiver_function(uint8_t *payload, uint16_t length, const PJON_Packet_Info &packet_info);
void error_handler(uint8_t code, uint16_t data, void *custom_pointer);
void initPjon();
void pingHandler(String *data, int length);
void loopPjon();
void sendPjonTopicMessage(String topicName, String data1, String data2 = "", String data3 = "");
int addPjonReceiveTopicListener(String topicName, void (*callback)(String *data, int length));
bool removePjonReceiveTopicListener(int id);
String getLoraRssi();
String getTotalPacketsSent();
String getTotalPacketsReceived();
String getTotalPacketsQueued();
String getLastConnectionTime();
void displayPjonStatus();
void sendPingPeriodically(int periodInMillis);



void receiver_function(uint8_t *payload, uint16_t length, const PJON_Packet_Info &packet_info) {
  displayPjonStatus();
  totalPacketsReceived++;
  lastConnectionTimestamp = millis();
  payload[length] = '\0'; // put a null at the end to terminate the string
  StaticJsonDocument<255> doc;
  String topicName = "";
  String data[] = {"", "", ""};
  int len = 1;
  DeserializationError error = deserializeJson(doc, (char*)payload);
  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }
  topicName = String(doc["e"].as<const char *>());
  data[0] = String(doc["d"].as<const char *>());

  if(doc.containsKey("d2")){
    data[1] = String(doc["d2"].as<const char *>());
    len++;
  }
  if(doc.containsKey("d3")){
    data[2] = String(doc["d3"].as<const char *>());
    len++;
  }
  

  for(PjonReceiveTopicListener topicListener : topicListeners){
    if(topicListener.topicName == topicName){
      topicListener.callback(data, len);
    }
  }
}

void error_handler(uint8_t code, uint16_t data, void *custom_pointer) {
	if (code == PJON_PACKETS_BUFFER_FULL) {
		Serial.print("Packet buffer is full, has now a length of ");
		Serial.println(data, DEC);
		Serial.println("Possible wrong pjonLora configuration!");
		Serial.println("higher PJON_MAX_PACKETS if necessary.");
	}
	if (code == PJON_CONTENT_TOO_LONG) {
		Serial.print("Content is too long, length: ");
		Serial.println(data);
	}
}

void initPjon() {
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, LOW);

  pjonLora.set_id(PJON_LOCAL_ADDRESS);

  SPI.begin(LORA_SCLK_PIN, LORA_MISO_PIN, LORA_MOSI_PIN);
  pjonLora.strategy.setPins(LORA_CS_PIN, LORA_RST_PIN, LORA_DIO0_PIN);

	pjonLora.set_receiver(receiver_function);
	pjonLora.set_error(error_handler);
	// Synchronous acknowledgement is not supported // This was written in the example
  pjonLora.set_acknowledge(false);             // I checked the source and it seems to be supported now, though not sure
	// Obligatory to initialize Radio with correct frequency
	pjonLora.strategy.setFrequency(LORA_FREQUENCY);
	// Optional
	pjonLora.strategy.setSignalBandwidth(getFromDatabase("LORA_BANDWIDTH").toInt());
	pjonLora.strategy.setSpreadingFactor(getFromDatabase("LORA_SPREADING_FACTOR").toInt());
  // pjonLora.strategy.setTxPower(getFromDatabase("LORA_TX_POWER").toInt());
	pjonLora.strategy.setCodingRate4(LORA_CODING_RATE);
	pjonLora.strategy.setSyncWord(LORA_SYNC_WORD);

	pjonLora.begin();

  addPjonReceiveTopicListener(LORA_PING_TOPIC, pingHandler);

  pingRole = getFromDatabase("ACTIVE_MODE") == "FIXED_STA"? "req" : "rep";    
}

void pingHandler(String *data, int length){
    String pingType = data[0];
    String id = data[1];
    if(pingType == "req"){
      sendPjonTopicMessage(LORA_PING_TOPIC, "rep", id);
    }
    receivedPings++;
    if(pingRole == "rep")
      totalPings = id.toInt();
}

void loopPjon() {
  sendPingPeriodically(10000);
	pjonLora.receive(20000); //receive for 20 ms
	pjonLora.update();
}

// sends a PJON packet with JSON payload {"e":"", "d1":"", "d2":"", "d3":""}
void sendPjonTopicMessage(String topicName, String data1, String data2, String data3){
  StaticJsonDocument<255> doc;
  doc["e"] = topicName;
  doc["d"] = data1;
  if(data2 != "")
    doc["d2"] = data2;
  if(data3 != "")
    doc["d3"] = data3;

  String jsonStr;
  serializeJson(doc, jsonStr);

  Serial.println("PJON sending: " + jsonStr);
  pjonLora.send(PJON_REMOTE_ADDRESS, jsonStr.c_str(), jsonStr.length());
  totalPacketsSent++;
  lastConnectionTimestamp = millis(); //TODO: make update on actual send instead of on the call
}

// returns the id of the topicListener
int addPjonReceiveTopicListener(String topicName, void (*callback)(String *data, int length)){
  PjonReceiveTopicListener topicListener;
  topicListener.topicName = topicName;
  topicListener.callback = callback;
  topicListener.id = random(1000000);

  topicListeners.push_back(topicListener);

  return topicListener.id;
}

// returns true if the topicListener is removed successfully
bool removePjonReceiveTopicListener(int id){
  for(int i = 0; i < topicListeners.size(); i++){
    if(topicListeners[i].id == id){
      topicListeners.remove(i);
      return true;
    }
  }
  return false;
}

// returns the number of removed topicListeners
int removePjonReceiveTopicListener(String topicName){
  int numRemoved = 0;
  for(int i = 0; i < topicListeners.size(); i++){
    if(topicListeners[i].topicName == topicName){
      topicListeners.remove(i--);
    }
  }
  return numRemoved;
}

String getLoraRssi(){
  return String(pjonLora.strategy.packetRssi());
}

String getTotalPacketsSent(){
  return String(totalPacketsSent);
}

String getTotalPacketsReceived(){
  return String(totalPacketsReceived);
}
//get_packets_count
String getTotalPacketsQueued(){
  return String(pjonLora.get_packets_count());
}

String getLastConnectionTime(){
  return String((millis() - lastConnectionTimestamp) / 1000);
}

void displayPjonStatus(){
  String s = "";
  s += String(pjonLora.strategy.packetRssi()) + "  ";
  s += String(receivedPings) + "/" + String(totalPings);
  displayOnLoraPart(s);
}

void sendPingPeriodically(int periodInMillis){
  static uint64_t lastSent = 0;
  if(pingRole == "req" && millis() - lastSent > periodInMillis){
    Serial.println("pinging...");
    sendPjonTopicMessage(LORA_PING_TOPIC, "req", String(totalPings));
    totalPings++;  
    lastSent = millis();
    displayPjonStatus();
  }
}

#endif
