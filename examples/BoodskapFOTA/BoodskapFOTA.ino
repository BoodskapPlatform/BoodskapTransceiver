/**
  MIT License

  Copyright (c) 2017 Boodskap Inc

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/
#include <ArduinoJson.h>
#include <BoodskapCommunicator.h>

#define CONFIG_SIZE 1024
#define REPORT_INTERVAL 15000
#define MESSAGE_ID 110 //***** Message defined in the platform ****

/**
 * ***** PLEASE CHANGE THE BELOW SETTINGS MATCHING YOUR ENVIRONMENT *****
*/

#define DEF_WIFI_SSID "change-this"  //Your WiFi SSID
#define DEF_WIFI_PSK "change-this" //Your WiFi password
#define DEF_DOMAIN_KEY "change-this" //your DOMAIN Key
#define DEF_API_KEY "change-this" //Your API Key
#define DEF_DEVICE_MODEL "ESP" //Your device model (Change this)
const char* DEF_FIRMWARE_VER =  "1.0.0"; //Your firmware version (Change this)

BoodskapTransceiver Boodskap(UDP); //MQTT, UDP, HTTP

uint32_t lastReport = 0;

void sendReading();
void handleData(byte* data);
bool handleMessage(uint16_t messageId, JsonObject& header, JsonObject& data);

void setup() {

  Serial.begin(115200);
  Boodskap.setHandleData(&handleData);
  Boodskap.setHandleMessage(&handleMessage);

  StaticJsonBuffer<CONFIG_SIZE> buffer;
  JsonObject &config = buffer.createObject();

  config["ssid"] = DEF_WIFI_SSID;
  config["psk"] = DEF_WIFI_PSK;
  config["domain_key"] = DEF_DOMAIN_KEY;
  config["api_key"] = DEF_API_KEY;
  config["dev_model"] = DEF_DEVICE_MODEL;
  config["fw_ver"] = DEF_FIRMWARE_VER;
  config["dev_id"] = String("NODEMCU-") + String(ESP.getChipId()); //Your unique device ID

  /**
     If you have setup your own Boodskap IoT Platform, then change the below settings matching your installation
     Leave it for default Boodskap IoT Cloud Instance
  */
  config["api_path"] = "https://api.boodskap.io"; //HTTP API Base Path Endpoint
  config["ota_path"] = "http://ota.boodskap.io"; //HTTP OTA API Base Path Endpoint
  config["api_fp"] = "A8:72:EF:14:B4:59:FD:3B:88:98:07:3C:7F:7F:8B:E1:00:21:54:CA"; //In case of HTTPS enter your server fingerprint (https://www.grc.com/fingerprints.htm)
  config["udp_host"] = "udp.boodskap.io"; //UDP Server IP
  config["udp_port"] = 5555; //UDP Server Port
  config["mqtt_host"] = "mqtt.boodskap.io"; //MQTT Server IP
  config["mqtt_port"] = 1883; //MQTT Server Port
  config["heartbeat"] = 45; //seconds

  Boodskap.setup(config);
}

void loop() {

  Boodskap.loop();

  if ((millis() - lastReport) >= REPORT_INTERVAL) {
    sendReading();
    lastReport = millis();
  }
}

void sendReading() {

  StaticJsonBuffer<128> buffer;
  JsonObject &data = buffer.createObject();
  data["click"] = 1;
  data["battery"] = "1345mv";

  Boodskap.sendMessage(MESSAGE_ID, data);
}

void handleData(byte* data) {
  //handle raw data from the platform
}

bool handleMessage(uint16_t messageId, JsonObject& header, JsonObject& data){
   //handle JSON commands from the platform
  return false; //return true if you have successfully handled the message
}
