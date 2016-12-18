# ESP8266 LED Strip dimmer for TJCLEMENT's HW

This project is FIRMWARE for [TJCLEMENT's HW](https://github.com/tjclement/esp-dimmer-hardware) which I can very recommend, no need for serious soldering skillz as if I was able to build it, everyone can too. I hope that following this README will be as straightforward as possible. In case you have questions do not hesitate to contact me using [Issues](https://github.com/ArnieX/esp-dimmer-firmware/issues).

## Dependencies
- Some MQTT server (If you have Raspberry Pi use Mosquitto) - Source contains public broker as default good for testing.
- Optional is [Homebridge](https://github.com/nfarina/homebridge) with [MQTT Plugin](https://github.com/cflurin/homebridge-mqtt) to control the relay from iDevices
- Optional that will make your life easier with IoT is Node-RED, plus you can get decent dashboard with Node-RED Dashboard plugin
- [PlatformIO](https://github.com/platformio/platformio) best Arduino IDE available, hacked from ATOM text editor

- [WiFi Manager](https://github.com/tzapu/WiFiManager)
- [PubSubClient (MQTT)](https://github.com/knolleary/pubsubclient)
- ESP8266 WiFi library

Install using PlatformIO Library Manager

```
pio lib -g install WiFiManager
pio lib -g install PubSubClient
pio lib -g install ESP8266wifi
```

- Dimming module from [TJCLEMENT](https://github.com/tjclement/esp-dimmer-hardware) (Cheap ca. 5$)

## Getting started

Update main.ino with your custom preferences

[11] Set desired configuration AP name (This is used when ESP8266 isn't connected to your WiFi router to allow setup)
[12] Set password for configuration AP, so that noone else can access it in case your router is OFF
[13] Set your MQTT IP address
[14] Set your MQTT PORT

[17-23] Change MQTT topics (THIS IS OPTIONAL and I do not recommend to change it for first test of function)

[37] Set your OTA password, this will be used for secured OTA update using PlatformIO, change this respectively in platformio.ini too

Update platformio.ini with your custom preferences (Do not change unless you want to turn OTA ON)

[14 and 15] To enable OTA for next updates uncomment these lines and change values to reflect your environment

To turn OTA OFF any time, just comment these lines again with ;

## Usage

Send command through your MQTT server as such:

|TOPIC|DESCRIPTION|
|---|---|
|home/room/ledstrip|Turn LED Strip ON/OFF send 1/0|
|home/room/ledstrip/brightness|Set brightness 0% to 100% so send 0-100 number|
|home/pingall|Send whatever and you get response at topic home/pingallresponse|

Receive back from your device:

|TOPIC|DESCRIPTION|
|---|---|
|home/room/device_name/devicestatus|Will contain device status eg. connected/disconnected|
|home/room/ledstrip/status|Current ON/OFF status 1/0|
|home/room/ledstrip/brightness/status|Current brightness level, number 0-100|
|home/pingallresponse|This will contain status after you send pingall request and all devices should respond|

## The MODULE

![LED Strip MODULE from TJCLEMENT](https://github.com/ArnieX/esp-dimmer-firmware/blob/master/images/IMG_1468-2.JPG?raw=true)
