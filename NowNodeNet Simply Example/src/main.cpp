//-----------------------------------------------------------------------------

//-- Used as Master-Template for all testing ESP32's and/or ESP8266's

//-----------------------------------------------------------------------------

#include <Arduino.h>

#include <main_support.hpp>

//-----------------------------------------------------------------------------
//-- Arduino's Core Methods - Setup and Loop

//-- User Setup Method
void setup() {
  mainSetup(); // ESP-Now specific setup requirements.

  pinMode(LED_BUILTIN, OUTPUT); // read/write access for LED_BUILTIN pin - demo only.

  //...for testing only - display abbreviated build summary.
  display.printf("  IP address: %s\n", WiFi.localIP().toString().c_str());
  display.printf("WiFi Channel: %d\n", WiFi.channel());
  display.printf(" MAC Address: %s\n", WiFi.macAddress().c_str());
  display.printf("     Version: %s\n", VERSION);
  #ifdef GATEWAY
    display.printf("    ESP Name: %s is WiFi-Gateway\n", espName.c_str());
    Serial.printf("\n Web Console: CMD-Click http://%s \n\n", WiFi.localIP().toString().c_str());
  #else
    display.printf("    ESP Name: %s\n\n", espName.c_str());
  #endif

  randomSeed((int)(micros()%100)); // µs until these machine instructions execute.

  display.printf("Setup complete for node %s\n", espName.c_str());
  display.println("\n");
}


//-- User Loop Method
void loop() {
  mainLoop();  // Declutter by moving application specific requirements.

  //-- One-second Async HeartBeats timers. (Optional, but helpful.)
  static uint32_t last_heartbeat = 0; //...for limiter.
  if (last_heartbeat != heartbeats) { // Limit to one call per second.

    if(heartbeats && !(heartbeats%10)) { //...optional, testing on/off for all, every 10-seconds.
      outgoingData.but_status = (int)random(0,2); // LED_BUILTIN status - 0 or 1 (off/on)
      if (esp_now_send( broadcastAddress,
                  (uint8_t*)&outgoingData,
                  sizeof(outgoingData)) != 0) { // Checked sending results - failure.
        display.log("Error: send failure.");
      } else { //...sent successfully.
        #ifdef INVERTED // LED_BUILTIN with inverted states. 
          digitalWrite(LED_BUILTIN, !outgoingData.but_status);
        #else
          digitalWrite(LED_BUILTIN, outgoingData.but_status);
        #endif ///INVERTED

        display.printf("%s >> %s\n", outgoingData.orig, outgoingData.but_status?"on":"off");
      }
    }
    last_heartbeat = heartbeats;
  } ///HeartBeats - Ignore all but first loop within a second.

} ///Loop

//-----------------------------------------------------------------------------
