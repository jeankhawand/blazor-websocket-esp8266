#include <DHT_U.h>
#include <DHT.h>

#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>

// Constants
const char *ssid = "<your-ssid>";
const char *password = "<your-pass>";
const int relay = 10;
const int dhtpin = 14;
const int mq2analogue = A0;
DHT dht(dhtpin, DHT11);
// JSON Definition
const size_t capacity = JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(3);
DynamicJsonDocument doc(capacity);
JsonObject pins = doc.createNestedObject("pins");

/*
pins["GPIO8"] = 0;
pins["GPIO11"] = 0;
pins["GPIO7"] = 0;
pins["GPIO6"] = 0;
pins["GPIO16"] = 0;
pins["GPIO5"] = 0;
pins["GPIO4"] = 0;
pins["GPIO0"] = 0;
pins["GPIO2"] = 0;
pins["GPIO14"] = 0;
pins["GPIO12"] = 0;
pins["GPIO13"] = 0;
pins["GPIO15"] = 0;
pins["GPIO3"] = 0;
pins["GPIO1"] = 0;*/

// Globals
WebSocketsServer webSocket = WebSocketsServer(80);

// Called when receiving any WebSocket message
void onWebSocketEvent(uint8_t num,
                      WStype_t type,
                      uint8_t *payload,
                      size_t length)
{

  // Figure out the type of WebSocket event
  switch (type)
  {

  // Client has disconnected
  case WStype_DISCONNECTED:
  {
    Serial.printf("[%u] Disconnected!\n", num);
    break;
  }

  // New client has connected
  case WStype_CONNECTED:
  {
    IPAddress ip = webSocket.remoteIP(num);
    Serial.printf("[%u] Connection from ", num);
    Serial.println(ip.toString());
    break;
  }

  // Echo text message back to client
  case WStype_TEXT:
  {
    Serial.printf("[%u] Text: %s\n", num, payload);
    if (strcmp((char *)payload, "init") == 0)
    {
      //sending initial state
      Serial.println("received init");
    }
    // else we are going to send new json
    // capture humidity sensor
    float myTemperature = dht.readTemperature();
    float myHumidity = dht.readHumidity();
    pins["A0"] = analogRead(mq2analogue); // MQ2 Sensor Data
    pins["GPIO10"] = myTemperature;       // Humidity Data
    pins["GPIO9"] = myHumidity;           // Temp Data
    // debugging mq2 values with temp
    Serial.println(analogRead(mq2analogue));
    Serial.println(myTemperature);
    Serial.println(myHumidity);
    String m;
    // serialize json and send it through websocket
    serializeJson(doc, m);
    webSocket.sendTXT(num, m);
    break;
  }

  // For everything else: do nothing
  //    case WStype_BIN:
  //    case WStype_ERROR:
  //    case WStype_FRAGMENT_TEXT_START:
  //    case WStype_FRAGMENT_BIN_START:
  //    case WStype_FRAGMENT:
  //    case WStype_FRAGMENT_FIN:
  default:
  {
    break;
  }
  }
}

void setup()
{

  // Start Serial port
  Serial.begin(115200);
  dht.begin();
  //
  pins["A0"] = 0;
  pins["GPIO10"] = 0;
  pins["GPIO9"] = 0;

  // Connect to access point
  Serial.println("Connecting");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  // Print our IP address
  Serial.println("Connected!");
  Serial.print("My IP address: ");
  Serial.println(WiFi.localIP());
  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);
}

void loop()
{
  webSocket.loop();
}
