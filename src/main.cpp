#include <Arduino.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>
#include <ESPmDNS.h>
#include <config.h>

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
Preferences preferences;

// ANY VARIBLE CHANGES, MODIFY THE CONFIG.H FILE
// Remeber to build and upload all SPIFFS files!!!

enum LightState
{
  RED,
  GREEN,
  YELLOW,
  OFF
};

LightState currentLightState = OFF;
LightState previousLightState = OFF;
unsigned long previousMillis = 0;
unsigned long currentDelay = 0;
unsigned long LED_red_delay = 0;
unsigned long LED_yellow_delay = 0;
unsigned long LED_green_delay = 0;

bool blinkMode = false;
bool catMode = false;
bool blinkState = false;
bool blinkAllColors = true; // Default to blinking all colors
int blinkPin = -1;
unsigned long lastBlinkMillis = 0;

void set_traffic_light(boolean LED_red_state, boolean LED_yellow_state, boolean LED_green_state)
{
  Serial.println("Changing traffic light color: "
                 "Red: " +
                 String(LED_red_state ? "ON" : "OFF") +
                 " Yellow: " + String(LED_yellow_state ? "ON" : "OFF") +
                 " Green: " + String(LED_green_state ? "ON" : "OFF"));

  // output and invert the logic here for relays
  digitalWrite(LED_red_pin, !LED_red_state);
  digitalWrite(LED_yellow_pin, !LED_yellow_state);
  digitalWrite(LED_green_pin, !LED_green_state);

  String state = "all_off";
  if (LED_red_state)
    state = "red";
  else if (LED_yellow_state)
    state = "yellow";
  else if (LED_green_state)
    state = "green";

  // Send the state to the client nomalt mode/cat mode
  if (catMode)
  {
    Serial.println("Cat Mode1: " + state);
    String jsonResponse = "{\"state\":\"" + state + "_cat\"}";
    ws.textAll(jsonResponse);
  }
  else
  {
    Serial.println("Normal Mode1: " + state);
    String jsonResponse = "{\"state\":\"" + state + "\"}";
    ws.textAll(jsonResponse);
  }
}

void cycleLights()
{
  unsigned long currentMillis = millis();
  if (blinkMode)
  {
    if (currentMillis - lastBlinkMillis >= blinkInterval)
    {
      lastBlinkMillis = currentMillis;
      blinkState = !blinkState;
      if (blinkAllColors)
      {
        digitalWrite(LED_red_pin, !blinkState); // Invert all the output state
        digitalWrite(LED_yellow_pin, !blinkState);
        digitalWrite(LED_green_pin, !blinkState);
      }
      else
      {
        digitalWrite(blinkPin, !blinkState); // Invert the output state
      }

      // Update the traffic light image on the webpage
      String state = "all_off";
      if (blinkState)
      {
        if (blinkAllColors)
          state = "all_on";
        else if (blinkPin == LED_red_pin)
          state = "red";
        else if (blinkPin == LED_yellow_pin)
          state = "yellow";
        else if (blinkPin == LED_green_pin)
          state = "green";
      }

      if (catMode) // return either the cat light or nomal traffic light
      {
        String jsonResponse = "{\"state\":\"" + state + "_cat\"}";
        ws.textAll(jsonResponse);
      }
      else
      {
        String jsonResponse = "{\"state\":\"" + state + "\"}";
        ws.textAll(jsonResponse);
      }
    }
    return;
  }

  if (currentMillis - previousMillis >= currentDelay) // only change the light stated after a timmed delay
  {
    previousMillis = currentMillis;
    switch (currentLightState)
    {
    case RED:
      currentLightState = GREEN;
      currentDelay = LED_green_delay;
      break;
    case GREEN:
      currentLightState = YELLOW;
      currentDelay = LED_yellow_delay;
      break;
    case YELLOW:
      currentLightState = RED;
      currentDelay = LED_red_delay;
      break;
    case OFF:
      currentLightState = RED;
      currentDelay = LED_red_delay;
      break;
    }
  }

  if (currentLightState != previousLightState) // if the light state has changed, update the light output
  {
    switch (currentLightState)
    {
    case RED:
      set_traffic_light(0, 0, 1);
      break;
    case GREEN:
      set_traffic_light(0, 1, 0);
      break;
    case YELLOW:
      set_traffic_light(1, 0, 0);
      break;
    case OFF:
      set_traffic_light(1, 1, 1);
      break;
    }
    previousLightState = currentLightState;
  }
}

void handleRoot(AsyncWebServerRequest *request)
{
  Serial.println("Got root request");
  if (SPIFFS.exists("/index.html"))
  {
    request->send(SPIFFS, "/index.html", "text/html");
  }
  else
  {
    Serial.println("index.html not found");
    request->send(404, "/index_page_not_found.html", "text/html");
  }
}

void handleSetDelay(AsyncWebServerRequest *request)
{

  Serial.println("Setting new delays");
  Serial.println("Current delays: R: " + String(LED_red_delay) + " Y: " + String(LED_yellow_delay) + " G: " + String(LED_green_delay));

  if (request->hasParam("red_delay"))
  {
    LED_red_delay = request->getParam("red_delay")->value().toInt() * 1000;
    preferences.putULong("red_delay", LED_red_delay);
  }
  if (request->hasParam("yellow_delay"))
  {
    LED_yellow_delay = request->getParam("yellow_delay")->value().toInt() * 1000;
    preferences.putULong("yellow_delay", LED_yellow_delay);
  }
  if (request->hasParam("green_delay"))
  {
    LED_green_delay = request->getParam("green_delay")->value().toInt() * 1000;
    preferences.putULong("green_delay", LED_green_delay);
  }

  Serial.println("Updated delays: R: " + String(LED_red_delay) + " Y: " + String(LED_yellow_delay) + " G: " + String(LED_green_delay));

  request->send(200, "text/plain", "Delays Updated");
  currentLightState = RED;
  previousMillis = millis();
  currentDelay = LED_red_delay;
}

void handleGetDelays(AsyncWebServerRequest *request)
{
  String jsonResponse = "{\"red\":" + String(LED_red_delay) +
                        ",\"yellow\":" + String(LED_yellow_delay) +
                        ",\"green\":" + String(LED_green_delay) + "}";
  request->send(200, "application/json", jsonResponse);
}

void handleBlinkMode(AsyncWebServerRequest *request)
{
  if (request->hasParam("color"))
  {
    String color = request->getParam("color")->value();
    if (color == "red")
    {
      blinkPin = LED_red_pin;
      blinkAllColors = false;
    }
    else if (color == "yellow")
    {
      blinkPin = LED_yellow_pin;
      blinkAllColors = false;
    }
    else if (color == "green")
    {
      blinkPin = LED_green_pin;
      blinkAllColors = false;
    }
    else if (color == "all")
    {
      blinkAllColors = true;
    }
    else
    {
      request->send(400, "text/plain", "Invalid Color 1");
      return;
    }

    blinkMode = true;
    blinkState = false;
    lastBlinkMillis = millis();
    request->send(200, "text/plain", "Blink Mode Set");
  }
  else
  {
    request->send(400, "text/plain", "Invalid Color 2");
  }
}

void handleToggleMode(AsyncWebServerRequest *request)
{

  blinkMode = !blinkMode;
  if (!blinkMode)
  {
    digitalWrite(LED_red_pin, LOW); // Turn off the blinking LEDs
    digitalWrite(LED_yellow_pin, LOW);
    digitalWrite(LED_green_pin, LOW);
    blinkPin = -1;
  }
  else
  {
    blinkAllColors = true; // Default to blinking all colors
  }

  Serial.println("Toggling " + String(blinkMode ? "Blink Mode" : "Cycle Mode"));

  // Send update to all clients
  String jsonResponse = "{\"mode\":\"" + String(blinkMode ? "blink" : "cycle") + "\"}";
  ws.textAll(jsonResponse);

  request->send(200, "text/plain", blinkMode ? "Blink Mode Enabled" : "Cycle Mode Enabled");
}

void handleToggleCatMode(AsyncWebServerRequest *request)
{
  catMode = !catMode;

  if (request->hasParam("color"))
  {
    String color = request->getParam("color")->value();
  }

  Serial.println("Toggling " + String(blinkMode ? "Normal Mode" : "Cat Mode"));
  request->send(200, "text/plain", blinkMode ? "Normal Mode" : "Cat Mode");
}

void listSPIFFSFiles()
{
  Serial.println("Listing SPIFFS files:");
  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  while (file)
  {
    Serial.print("FILE: ");
    Serial.print(file.name());
    Serial.print("\tSIZE: ");
    Serial.println(file.size());
    file = root.openNextFile();
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Starting");

  pinMode(LED_red_pin, OUTPUT);
  pinMode(LED_yellow_pin, OUTPUT);
  pinMode(LED_green_pin, OUTPUT);

  set_traffic_light(0, 0, 0);

  if (use_wifi)
  {
    // WIFI
    WiFi.begin(ssid, password);
    Serial.println("Connecting to WiFi...");
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
      Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
  else
  {
    // AP
    WiFi.softAP(AP_ssid); // Set up the AP without a password
    Serial.println("Access Point Started");
    Serial.print("IP address: ");
    Serial.println(WiFi.softAPIP());
  }

  if (!MDNS.begin("trafficlights"))
  {
    Serial.println("Error setting up MDNS responder!");
  }

  if (!SPIFFS.begin(true))
  {
    Serial.println("Failed to mount file system");
    return;
  }

  preferences.begin("traffic-light", false);

  if (!preferences.isKey("red_delay"))
  {
    preferences.putULong("red_delay", 5000);
  }

  if (!preferences.isKey("yellow_delay"))
  {
    preferences.putULong("yellow_delay", 3000);
  }

  if (!preferences.isKey("green_delay"))
  {
    preferences.putULong("green_delay", 6000);
  }

  LED_red_delay = preferences.getULong("red_delay", 5000);
  LED_yellow_delay = preferences.getULong("yellow_delay", 3000);
  LED_green_delay = preferences.getULong("green_delay", 6000);

  listSPIFFSFiles();

  server.on("/", HTTP_GET, handleRoot);
  server.on("/set_delay", HTTP_GET, handleSetDelay);
  server.on("/get_delays", HTTP_GET, handleGetDelays);
  server.on("/blink_mode", HTTP_GET, handleBlinkMode);
  server.on("/toggle_mode", HTTP_GET, handleToggleMode);
  server.on("/toggle_cat_mode", HTTP_GET, handleToggleCatMode);
  server.serveStatic("/images", SPIFFS, "/images");
  server.onNotFound([](AsyncWebServerRequest *request)
                    { request->send(302, "text/plain", "What ever you where looking for, was not found..."); });

  ws.onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
             {
              if (type == WS_EVT_CONNECT) {
                Serial.println("WebSocket client connected");
                // Send the current state to the client
                String state = "all_off";
                if (currentLightState == RED) state = "red";
                else if (currentLightState == YELLOW) state = "yellow";
                else if (currentLightState == GREEN) state = "green";
                
                if(catMode){
                  String jsonResponse = "{\"state\":\"" + state + "_cat\"}";
                  client->text(jsonResponse);
                }else{
                  String jsonResponse = "{\"state\":\"" + state + "\"}";
                  client->text(jsonResponse);
                }
                
              } else if (type == WS_EVT_DISCONNECT) {
                Serial.println("WebSocket client disconnected");
              } });

  server.addHandler(&ws);

  server.begin();
  Serial.println("HTTP server started");
}

void loop()
{
  cycleLights();
}