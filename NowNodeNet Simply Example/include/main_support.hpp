
//-- Main Application Support -------------------------------------------------

#ifndef MAIN_SUPPORT_HPP
#define MAIN_SUPPORT_HPP

#define SERIAL_BAUD_RATE 115200 // IFF NO_SERIAL is not defined.
#define MAX_XFER_SIZE 128

#include <Arduino.h>
#if defined ARDUINO_ARCH_ESP8266 // non-WiFi
  #include <espnow.h>
  #include <ESP8266WiFi.h>
  #include <ESPAsyncTCP.h>
  #ifdef INC_FILES
    #define FILES LittleFS
    #include <Updater.h>
    #include "FS.h"
    #include "LittleFS.h"
  #endif ///INC_FILES
  #define display Serial
  #define WIFI_MODE_STA WIFI_STA
#elif defined ARDUINO_ARCH_ESP32
  #include <esp_now.h> //...from Arduino libraries.
  #include <WiFi.h>
  #include <esp_wifi.h>
  #ifdef INC_FILES
    #define  FILES SPIFFS
    #include "SPIFFS.h"
    #include <Update.h>
    #include "FS.h"
  #endif ///INC_FILES
  #ifndef GATEWAY
    #define display Serial
  #else
    #define display console
    #include <gateway_support.hpp>
  #endif ///GATEWAY
#else
  #error "Unsupported. Required: ARDUINO_ARCH_ESP8266 or ARDUINO_ARCH_ESP32"
#endif ///ARDUINO_ARCH_ESP8266
#include <vector>
#include <queue>

//-- Common Global Definitions
String espName;
unsigned long init_heap_size = 0;
static uint32_t heartbeats = 0;
uint8_t broadcastAddress[6] = {MAC_ADDRESS}; // Common ESP-NOW MAC Address

typedef struct struct_message { // Data transfer structure - sender/receiver.
  int but_status;
  char dest[10];   // Target ESP's name. (9 + '\0')
  char orig[10];   // This ESP's name. (9 + '\0')
  char cmnd[10];   // This ESP's name. (9 + '\0')
  char data[MAX_XFER_SIZE-36];
} struct_message;
struct_message outgoingData;  // Create a data struct_message for sending.
struct_message incomingData;  // Create a data struct_message for recieviong.

#ifdef GATEWAY
  #include <gateway_support.hpp>
#endif ///GATEWAY

//-- Support Methods ----------------------------------------------------------

#if defined ARDUINO_ARCH_ESP32  //-- Callback Methods

  esp_now_peer_info_t peerInfo; // Create peer interface, adding one per esp-node.

  void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    if (status != ESP_NOW_SEND_SUCCESS ) display.println("Last Packet Delivery Failed!");
  }
  void OnDataRecv(const uint8_t * mac, const uint8_t *recievedData, int len) {
    memcpy(&incomingData, recievedData, sizeof(incomingData));
    display.printf("%s << %s\n", incomingData.orig, incomingData.but_status?"on":"off");

    digitalWrite(LED_BUILTIN, incomingData.but_status);
  }

  void mainSetup() { // Initialize and setup core application components.
    Serial.begin(SERIAL_BAUD_RATE);
    Serial.setDebugOutput(true);

    WiFi.disconnect(true); //...just in case to avoid soft reboot hangup.
    WiFi.mode(WIFI_AP_STA);
    display.println("\n");

    #ifdef GATEWAY
      gatewaySetup();
    #endif ///GATEWAY

    esp_wifi_set_mac(WIFI_IF_STA, &broadcastAddress[0]); // Set ESP-NOW common MAC_Address.

    #ifdef FIXED_IP
      // Create a fixed-static WiFi IPAddress for this ESP-NOW NodeNet gateway/broker.
      IPAddress staticIP(FIXED_IP);       // see platformio.ini
      IPAddress gateway(192, 168, 2, 1);  //...required
      IPAddress subnet(255, 255, 0, 0);   //...required
      IPAddress primaryDNS(8, 8, 8, 8);   //...optional
      IPAddress secondaryDNS(8, 8, 4, 4); //...optional
      WiFi.config(staticIP,gateway,subnet,primaryDNS,secondaryDNS); // Set fixed, static IP.
    #endif ///FIXED_IP

    if(!(espName.length()))espName="esp"+String(WiFi.localIP()[3]);
    int n = snprintf(outgoingData.orig, 9, "%s", espName.c_str());
    outgoingData.orig[n] = '\0';

    if (esp_now_init() != ESP_OK) { // Initialize ESP-NOW
      display.println("ESP-NOW initialization error.");
      return; //...failed so abort. (perhaps, soft restart?)
    }

    esp_now_register_recv_cb(OnDataRecv); // Receive callback function.
    esp_now_register_send_cb(OnDataSent); // Send callback function.
    
    memcpy(peerInfo.peer_addr, broadcastAddress, 6); // Register peer
    esp_wifi_set_channel((uint8_t)(USE_CHANNEL),WIFI_SECOND_CHAN_NONE);
    peerInfo.channel = (uint8_t)(USE_CHANNEL);
    peerInfo.encrypt = false;

    // Add broadcast nodes as a group based on common MAC Address and WiFi channel.
    if (esp_now_add_peer(&peerInfo) != ESP_OK) { // All use message broadcasting.
      display.println("Failed to add peer"); // ...notify if failed.
      return; //...failed so abort. (perhaps, soft restart?)
    }
  }
#elif defined ARDUINO_ARCH_ESP8266

  void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) { // Callback, was data sent?
    if (!!sendStatus) display.println("Last Packet Delivery Failed!");
  }
  void OnDataRecv(uint8_t * mac, uint8_t *recievedData, uint8_t len) { // Callback, data was received.
    memcpy((uint8_t*)&incomingData, (uint8_t*)recievedData, sizeof(incomingData));
    display.printf("%s << %s\n", incomingData.orig, incomingData.but_status?"on":"off");

    #ifdef INVERTED // LED_BUILTIN inverted states. 
      digitalWrite(LED_BUILTIN, !incomingData.but_status);
    #else
      digitalWrite(LED_BUILTIN, incomingData.but_status);
    #endif ///INVERTED
  }

  void mainSetup() { // Initialize and setup core application components.
    Serial.begin(SERIAL_BAUD_RATE);
    Serial.setDebugOutput(true);

    WiFi.disconnect(true); //...just in case to avoid soft reboot hangup.
    WiFi.mode(WIFI_AP_STA);
    display.println("\n");

    wifi_set_channel((uint8_t)(USE_CHANNEL));
    wifi_set_macaddr(STATION_IF, &broadcastAddress[0]);

    #ifdef FIXED_IP //...set ESP's fixed/static IP Address.
      IPAddress staticIP(FIXED_IP);       // see platformio.ini
      IPAddress gateway(192, 168, 2, 1);  //...required
      IPAddress subnet(255, 255, 0, 0);   //...required
      IPAddress primaryDNS(8, 8, 8, 8);   //...optional
      IPAddress secondaryDNS(8, 8, 4, 4); //...optional
      WiFi.config(staticIP,gateway,subnet,primaryDNS,secondaryDNS); // Set fixed, static IP.
    #endif ///FIXED_IP

    if(!(espName.length()))espName="esp"+String(WiFi.localIP()[3]);
    int n = snprintf(outgoingData.orig, 9, "%s", espName.c_str());
    outgoingData.orig[n] = '\0';
    WiFi.disconnect();
    if (!!esp_now_init()) { // Initialize ESP-NOW protocol.
      display.println("Error initializing ESP-NOW");
      return;
    }
    esp_now_set_self_role(ESP_NOW_ROLE_COMBO); // Set ESP-NOW Role
    esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_COMBO, 7, NULL, 0); // Register one peer.
    esp_now_register_recv_cb(OnDataRecv); //...callback function for incoming data.
    esp_now_register_send_cb(OnDataSent);
  }
#endif ///ARDUINO_ARCH_ESP32 or ARDUINO_ARCH_ESP8266

void mainLoop(void) {  //...hide required loop functionality from user code.

  //-- Async HeartBeats - one-second 'heartbeat' timer loop. (optional)
  static uint64_t lastUpdated = 0;
  uint64_t currentMillis = millis();
  for (int delta=0;  // 1-second 'heartbeat' timer loop. (Optional.)
    (delta=(currentMillis-lastUpdated)) >= 1000;  //...caution, roll-over error.
    lastUpdated=(currentMillis-(delta%1000)),
    ++heartbeats)
  { /* intervals code goes here */ } ///HeartBeats

}///mainLoop

#endif ///MAIN_SUPPORT_HPP

//-----------------------------------------------------------------------------
