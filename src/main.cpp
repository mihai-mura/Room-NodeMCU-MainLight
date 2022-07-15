#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include <sendRawHEX.cpp>
#include <env.cpp>

/*
  POWER: 1FEA05F
  NIGHT MODE: 1FE609F
  BRIGHTNESS UP: 1FE7887
  BRIGHTNESS DOWN: 1FE40BF
  SWITCH TEMP: 1FE807F
  TIMER: 1FEC03F
  COLD: 1FE48B7
  WARM: 1FE58A7
*/

int OutputPin = D1;

// WiFi config
const IPAddress ip(192, 168, 100, 54);
const IPAddress gateway(192, 168, 100, 1);
const IPAddress bcastAddr(192, 168, 100, 255);
const IPAddress subnet(255, 255, 255, 0);
const IPAddress dns(192, 168, 100, 1);

// WebSocket config
WebSocketsClient webSocket;

DynamicJsonDocument outDoc(1024);
DynamicJsonDocument inDoc(1024);

void sendWSData(String type, String data)
{
  outDoc["sender"] = "node-main-light";
  outDoc["type"] = type;
  outDoc["data"] = data;
  String json;
  serializeJson(outDoc, json);
  webSocket.sendTXT(json);
}

void webSocketEvent(WStype_t type, uint8_t *payload, size_t length)
{
  switch (type)
  {
  case WStype_DISCONNECTED:
    Serial.println("WebSocket Disconnected!");
    break;
  case WStype_CONNECTED:
    Serial.printf("WebSocket Connected");

    // send message to server when Connected
    Serial.println("WebSocket SENT: NodeMCU Connected");
    sendWSData("register-node-mcu", "");

    break;
  case WStype_TEXT:
    deserializeJson(inDoc, payload);
    const char *action = inDoc["action"];
    Serial.println("WebSocket Received: " + String(action));

    if (strcmp(action, "power") == 0)
    {
      sendRawHEX(OutputPin, 0x1FEA05F, 32);
    }
    else if (strcmp(action, "night-mode") == 0)
    {
      sendRawHEX(OutputPin, 0x1FE609F, 32);
    }
    else if (strcmp(action, "brightness-up") == 0)
    {
      sendRawHEX(OutputPin, 0x1FE7887, 32);
    }
    else if (strcmp(action, "brightness-down") == 0)
    {
      sendRawHEX(OutputPin, 0x1FE40BF, 32);
    }
    else if (strcmp(action, "switch-temp") == 0)
    {
      sendRawHEX(OutputPin, 0x1FE807F, 32);
    }
    else if (strcmp(action, "timer") == 0)
    {
      sendRawHEX(OutputPin, 0x1FEC03F, 32);
    }
    else if (strcmp(action, "cold") == 0)
    {
      sendRawHEX(OutputPin, 0x1FE48B7, 32);
    }
    else if (strcmp(action, "warm") == 0)
    {
      sendRawHEX(OutputPin, 0x1FE58A7, 32);
    }
    else
    {
      Serial.println("WebSocket Received: Unknown action");
    }
    break;
  }
}

void connectWiFi()
{
  WiFi.mode(WIFI_STA);
  WiFi.hostname("NodeMCU-MainLight");
  WiFi.config(ip, dns, gateway, subnet);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  int count = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(250);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
    digitalWrite(LED_BUILTIN, LOW);

    count++;
    if (count > 20)
    {
      delay(500);
      ESP.restart();
    }
  }

  Serial.println("Wifi connected, ip: " + WiFi.localIP().toString());
}

void connectCloud()
{
  webSocket.begin(SERVER_HOST, SERVER_PORT, "/");
  webSocket.onEvent(webSocketEvent);
}

void setup()
{
  Serial.begin(115200);
  pinMode(OutputPin, OUTPUT);
  digitalWrite(OutputPin, HIGH);

  connectWiFi();
  connectCloud();
}

void loop()
{
  webSocket.loop();

  // Reconnect WiFi
  if (WiFi.status() != WL_CONNECTED)
  {
    connectWiFi();
    return;
  }
}