
//-- Main Application Support -------------------------------------------------

#ifndef GATEWAY_SUPPORT
#define GATEWAY_SUPPORT

#include <Arduino.h>
#include <queue>

  ///extern struct_message outgoingData;
  ///extern struct_message incomingData;
extern String espName;
extern std::queue<String> pending;  // main_support.hpp

#include <credentials.h>  // WLAN WIFI_SSID and WIFI_PASS as well as WebPage login info.
#include <ESPAsyncWebServer.h>
AsyncWebServer server(80);
AsyncEventSource events("/events");

///extern AsyncEventSource events;


//--------------------------------------------------------------------------------
//-- Console Class
// Simply mimics some Serial print functionality allowing direct printing
// to the WebPages. By default, messages are also echoed to the IDE's USB
// serial monitor, IFF enabled, attached and opened/running. (see NO_ECHO)
class Console {
  public:
    Console(){};
    void log(String msg) {
      #ifdef GATEWAY
        events.send(msg.c_str(), "log", millis());
        #ifdef ECHO
          Serial.print(msg);
        #endif
      #else
        Serial.print(msg);
      #endif ///GATEWAY
    }
    void print(String msg) {
      #ifdef GATEWAY
        events.send(msg.c_str(), "data", millis());
        #ifdef ECHO
          Serial.print(msg);
        #endif
      #else
        Serial.print(msg);
      #endif ///GATEWAY
    }
    void println(String msg) {print(msg+"\r\n");}
    void printf(const char * printf_formatting,...) {
      static char buff[128];
      size_t n = 0;
      size_t len = sizeof(buff)-1;
      va_list printf_argptr;
      va_start(printf_argptr, printf_formatting);
      n = vsnprintf(&buff[0], len, printf_formatting, printf_argptr);
      va_end(printf_argptr);
      buff[n] = '\0';
      print(buff);
    }
 };
 Console console; // Instantiation Console's interface.

//--------------------------------------------------------------------------------

//-- Console'ish WebPage Example

PROGMEM const char console_html[] = R"rawliteral(<!DOCTYPE html><html><head><title>ezConsole</title><meta name="monitor" content="width=device-width, initial-scale=1"><meta http-equiv = 'content-type' content = 'text/html; charset = UTF-8'>
<link rel="shortcut icon" href="data:image/x-icon;base64,AAABAAEAEBAAAAEAIABoBAAAFgAAACgAAAAQAAAAIAAAAAEAIAAAAAAAAAQAAAAAAAAAAAAAAAAAAAAAAAD///8BAABkPQAAZckAAGbtAABm6wAAZ70AAGcv////Af///wH///8BAABjHwAAZpMAAGbDAABmkQAAah3///8BAABlPwAAZvkAAGaPAABnJQAAZSsAAGWhAABm9wAAZzn///8BAABjHwAAZuMAAGbBAABlbwAAZsMAAGbrAABkKQAAZsUAAGWNAABmDwAAZaEAAGeVAABxCQAAZo8AAGbN////AQAAZpEAAGa7AABpEQAAZVEAAG0HAABmpwAAZrkAAGbrAABnLwAAZpsAAGW/AABmwQAAZq0AAGYZAABm7QAAaScAAGanAABniwAAaScAAGbXAABmkwAAZy8AAGbnAABm4QAAZVMAAGVRAABmZwAAZ0UAAGbfAABpEQAAZuUAAGZfAABlUQAAZvEAAGZfAABmuQAAZpsAAGYtAABm6QAAZn0AAGbvAABmQQAAZyUAAGbFAABnqQAAZBcAAGb7AABm1QAAXQsAAGdjAABm2QAAZqcAAHEJAABmmQAAZb////8BAABmhQAAZu0AAGbxAABltQAAXQsAAGaHAABm/wAAZv8AAGbRAABoOwAAaxMAAGgbAABnlQAAZvUAAGU1////Af///wEAAGYPAABhFf///wEAAGZ9AABm/wAAZq0AAGZ7AABmpwAAZusAAGbtAABm8QAAZs0AAGY3////Af///wH///8B////Af///wEAAGdjAABm9QAAZzkAAGYPAABlSQAAZCEAAHEJAABoMQAAZiMAAFUD////Af///wH///8B////Af///wEAAFUDAABm2wAAZmcAAGcvAABm7QAAZt8AAGb5AABmdf///wEAAAAAAAAAAAAAAAAAAAAA////Af///wH///8BAABkIQAAZusAAGMfAABmuwAAZof///8BAABlSQAAZvMAAGgxAAAAAAAAAAAAAAAAAAAAAP///wH///8B////AQAAah0AAGbrAABnJQAAZqMAAGa7AABmuwAAZhkAAGbPAABlZQAAAAAAAAAAAAAAAAAAAAD///8B////Af///wH///8BAABlyQAAZo8AAGINAABmnQAAZW8AAGQXAABm6wAAZUcAAAAAAAAAAAAAAAAAAAAA////Af///wH///8B////AQAAZD0AAGb3AABllwAAaScAAGg7AABm1QAAZrv///8BAAAAAAAAAAAAAAAAAAAAAP///wH///8B////Af///wH///8BAABmNwAAZsUAAGbvAABm6wAAZpsAAGYP////AQAAAAAAAAAAAAAAAAAAAAD///8B////Af///wH///8B////Af///wH///8B////Af///wH///8B////Af///wEAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA==" />
<style>*{box-sizing:border-box}:focus{outline:0}body{overflow-y:hidden}input{font-size:18px;width:calc(99vw - 6px);height:35px;padding:5px;align-self:center}textarea{resize:none;border:none;width:calc(99vw - 6px);height:calc(99vh - 46px);padding:10px;align-self:center}</style>
<script>
window.onload = function() {
  window.$ = document.querySelector.bind(document);
  $.input = $('input');
  $.textarea = $('textarea');
  $.textarea.innerHTML = '';
  $.stb = function() {
    setTimeout(function(){$.textarea.scrollTop=$.textarea.scrollHeight-50},0);
  }
  $.addMessage = function(str) {
    $.textarea.innerHTML += (str+'\n');
    $.stb();
  }
  $.input.onkeydown = function(n) {
    13 == n.keyCode&&('' == $.input.value
      ? $.textarea.innerHTML = ''
      : ($.addMessage('>> '+$.input.value),
            /* add massage formatting here */
            /* then send msg to ESP server */
                /* $.ws.send($.input.value), */
          $.input.value = '') );
  }
}
</script>
</head><body><label><input type = 'text'/></label><textarea onClick=''></textarea>
<script>
if (!!window.EventSource) {
 var source = new EventSource('/events');
 source.addEventListener('open', function(e) {
  console.log("Events Connected");
 }, false);
 source.addEventListener('error', function(e) {
  if (e.target.readyState != EventSource.OPEN) {
    console.log("Events Disconnected.");
  }
 }, false);
 source.addEventListener('log', function(e) {
  console.log(e.data);
 }, false);
 source.addEventListener('data', function(e) {
  $.addMessage(e.data);
 }, false);
}
</script>
</body></html>)rawliteral";


void gatewaySetup() {
  display.printf("\n\n  Connecting: [%s] ", WiFi.macAddress().c_str());
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  for (uint8_t wait=20; WiFi.status() != WL_CONNECTED; --wait) {
    if (!wait) {display.println("\nWiFi Connect Timeout Rebooting...\n\n"); ESP.restart();}
    Serial.print(".");
    delay(500);
  }
  display.printf("\n   Connected: %s\n", WIFI_SSID);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", console_html);
  });
 
  server.on("/info", HTTP_GET, [](AsyncWebServerRequest *request) {
    display.printf("%s/%s\n", espName.c_str(), (char*)(VERSION));
    request->send(200, "text/plain", VERSION);
  });
  server.on("/reboot", HTTP_GET, [](AsyncWebServerRequest *request) {
    display.printf("%s/Rebooting...\n\n", espName.c_str());
    request->send(200, "text/plain", "Rebooting...");
    delay(500);
    ESP.restart();
  });

  events.onConnect([](AsyncEventSourceClient *client){ // WebPage connection confirmation.
    if (client->lastId()){
      display.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    display.println("Welcome!");
  });

  server.addHandler(&events);
  server.begin();
  display.println(" WiFi Server: Available\n");

} ///gatewaySetup

//--------------------------------------------------------------------------------

#endif ///GATEWAY_SUPPORT

//--------------------------------------------------------------------------------

