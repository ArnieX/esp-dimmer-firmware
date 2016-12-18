#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

// Constants
const char* autoconf_ssid  = "ESP8266_LEDSTRIP";    //AP name for WiFi setup AP which your ESP will open when not able to connect to other WiFi
const char* autoconf_pwd   = "PASSWORD";            //AP password so noone else can connect to the ESP in case your router fails
const char* mqtt_server    = "test.mosquitto.org";  //MQTT Server IP, your home MQTT server eg Mosquitto on RPi, or some public MQTT
const int mqtt_port        = 1883;                  //MQTT Server PORT, default is 1883 but can be anything.

// MQTT Constants
const char* mqtt_devicestatus_set_topic              = "home/room/ledstrip/devicestatus";
const char* mqtt_dimlightstatus_set_topic            = "home/room/ledstrip/status";
const char* mqtt_dimlightbrightnessstatus_set_topic  = "home/room/ledstrip/brightness/status";
const char* mqtt_pingallresponse_set_topic           = "home/pingallresponse";
const char* mqtt_dimlight_get_topic                  = "home/room/ledstrip";
const char* mqtt_dimlightbrightness_get_topic        = "home/room/ledstrip/brightness";
const char* mqtt_pingall_get_topic                   = "home/pingall";

// Global
int current_brightness = 100;             // For brightness animation (last value)
int last_homebridge_brightness = 100;     // After turning OFF and ON this value will be set
byte state = 1;
long lastReconnectAttempt = 0;

WiFiClient espClient;
PubSubClient client(espClient);

void setup_ota() {

  // Set OTA Password, and change it in platformio.ini
  ArduinoOTA.setPassword("ESP8266_PASSWORD");
  ArduinoOTA.onStart([]() {});
  ArduinoOTA.onEnd([]() {});
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {});
  ArduinoOTA.onError([](ota_error_t error) {
    if (error == OTA_AUTH_ERROR);          // Auth failed
    else if (error == OTA_BEGIN_ERROR);    // Begin failed
    else if (error == OTA_CONNECT_ERROR);  // Connect failed
    else if (error == OTA_RECEIVE_ERROR);  // Receive failed
    else if (error == OTA_END_ERROR);      // End failed
  });
  ArduinoOTA.begin();

}

boolean reconnect() {

  // MQTT reconnection function - NON BLOCKING!!

    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);

    // Attempt to connect - and set last will for devicestatus to disconnected
    if (client.connect(clientId.c_str(),mqtt_devicestatus_set_topic,0,false,"disconnected")) {

      // Once connected, publish an announcement...
      client.publish(mqtt_devicestatus_set_topic, "connected");
      // ... and resubscribe
      client.subscribe(mqtt_dimlight_get_topic);
      client.subscribe(mqtt_dimlightbrightness_get_topic);
      client.subscribe(mqtt_pingall_get_topic);

    }

    return client.connected();

}

void callback(char* topic, byte* payload, unsigned int length) {

    char c_payload[length];
    memcpy(c_payload, payload, length);
    c_payload[length] = '\0';

    String s_topic = String(topic);
    String s_payload = String(c_payload);

    blink();

  // Handling incoming messages

    if ( s_topic == mqtt_dimlight_get_topic ) {

      if (s_payload == "1") {

        if (state != 1) {

          // Turn ON function will set last known brightness

          client.publish(mqtt_dimlightstatus_set_topic, "1");
          state = 1;

          setBrightness(last_homebridge_brightness);


        }

      } else if (s_payload == "0") {

        if (state != 0) {

          // Turn OFF function

          client.publish(mqtt_dimlightstatus_set_topic, "0");
          state = 0;

          setBrightness(-1); // -1 is FULL STOP of PWM


        }

      }

    } else if ( s_topic == mqtt_dimlightbrightness_get_topic ) {

      int brightness = s_payload.toInt();

      if (brightness <= 100) {

        last_homebridge_brightness = brightness;
        setBrightness(brightness);
        client.publish(mqtt_dimlightbrightnessstatus_set_topic,c_payload);

      }

    } else if ( s_topic == mqtt_pingall_get_topic ) {

      client.publish(mqtt_pingallresponse_set_topic, "{\"livingroom_ledstrip\":\"connected\"}");
      client.publish(mqtt_devicestatus_set_topic, "connected");

    }


}

void setBrightness(int newbrightness) {

  // This function will animate brightness change from last known brightness to the new one

  if (newbrightness > current_brightness) {

    for (int i=current_brightness; newbrightness>i; i++) {

      analogWrite(D7, i*10);
      delay(10);
      current_brightness = i;

    }

  } else if (newbrightness < current_brightness) {

    for (int i=current_brightness; newbrightness<i; i--) {

      analogWrite(D7, i*10);
      delay(10);
      current_brightness = i;

    }

  } else if (newbrightness == current_brightness) {

    analogWrite(D7, newbrightness*10);

  }

}

void blink() {

  //Blink will try to blink ESP8266-12E built-in blue LED (try because some ESP's reportedly do not blink programatically on GPIO2)
  digitalWrite(D4, LOW);
  delay(25);
  digitalWrite(D4, HIGH);

}

void setup() {

  pinMode(D4, OUTPUT);               //Setup pin for internal ESP LED
  digitalWrite(D4, LOW);
  pinMode(D7, OUTPUT);               //Setup pin for MOSFET
  analogWriteRange(1000);            //This should set PWM range not 1023 but 100 as is %
  analogWrite(D7,1000);              //Turn OFF by default
  Serial.begin(115200);
  WiFiManager wifiManager;
  wifiManager.autoConnect(autoconf_ssid,autoconf_pwd);
  setup_ota();
  MDNS.begin("esp-ledstrip");
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  lastReconnectAttempt = 0;
  digitalWrite(D4, HIGH);           //Turn of buil-in LED by default and signal end of SETUP phase

}

void loop() {

  // KEEP UP MQTT
  if (!client.connected()) {
    long now = millis();
    if (now - lastReconnectAttempt > 5000) {
      lastReconnectAttempt = now;
      // Attempt to reconnect
      if (reconnect()) {
        lastReconnectAttempt = 0;
      }
    }
  } else {
    // Client connected
    client.loop();
  }

  // KEEP UP OTA
  ArduinoOTA.handle();

}