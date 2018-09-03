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

#define CONFIG_SIZE 512
#define UPLOAD_SPEED 115200
#define REPORT_INTERVAL 60000*60 //Seconds
#define MESSAGE_ID 104 //Message defined in the platform
#define LED_PIN D4 // Led Indication pin of board
#define RELAY_PIN D2 // relay pin connected to board


/**
 * ***** PLEASE CHANGE THE BELOW SETTINGS MATCHING YOUR ENVIRONMENT *****
*/
#define DEF_WIFI_SSID "your_wifi_ssid" //Your WiFi SSID
#define DEF_WIFI_PSK "your_wifi_psk" //Your WiFi password
#define DEF_DOMAIN_KEY "your_domain_key" //your DOMAIN Key
#define DEF_API_KEY "your_api_key" //Your API Key
#define DEF_DEVICE_MODEL "BSKP_INBOARD_LIGHT_CONTROLLER" //Your device model
#define DEF_FIRMWARE_VER "1.0.0" //Your firmware version

BoodskapTransceiver Boodskap(UDP); //MQTT, UDP, HTTP

uint32_t lastReport = 0;
bool light; // define led as boolean type variable
bool status;
bool pStatus = false;
bool cStatus = false;

void sendReading();
void handleData(byte* data);
bool handleMessage(uint16_t messageId, JsonObject& header, JsonObject& data);
//bool prevDetect = LOW;

// the setup function runs once when you press reset or power the board
void setup() {
while(!Serial){
  Serial.begin(UPLOAD_SPEED);
}
  pinMode(LED_PIN, OUTPUT);          // Assign Pin Modes
  pinMode(RELAY_PIN, OUTPUT);
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
  config["dev_id"] = String("ESP8266-") + String(ESP.getChipId()); //Your unique device ID


  /**
    If you have setup your own Boodskap IoT Platform, then change the below settings matching your installation
    Leave it for default Boodskap IoT Cloud Instance
  */
  config["api_path"] = "https://api.boodskap.io"; //HTTP API Base Path Endpoint
  config["api_fp"] = "B9:01:85:CE:E3:48:5F:5E:E1:19:74:CC:47:A1:4A:63:26:B4:CB:32"; //In case of HTTPS enter your server fingerprint (https://www.grc.com/fingerprints.htm)
  config["udp_host"] = "udp.boodskap.io"; //UDP Server IP
  config["udp_port"] = 5555; //UDP Server Port
  config["mqtt_host"] = "mqtt.boodskap.io"; //MQTT Server IP
  config["mqtt_port"] = 1883; //MQTT Server Port
  config["heartbeat"] = 45; //seconds

  Boodskap.setup(config);

}

void loop() {

  Boodskap.loop();

  status = digitalRead(RELAY_PIN);

  cStatus = status;
  if (pStatus != cStatus) { // To Send Message to Boodskap Platform only If Current Status Change
    sendReading();
    Serial.println("Status Changed");
    lastReport = millis();
  }
  pStatus = cStatus;

  if ((millis() - lastReport) >= REPORT_INTERVAL) { // Periodicaly send Message to platform
    sendReading();
    lastReport = millis();
  }

  delay(200);
}


void sendReading() { // Send Message to Platform

  Serial.println("Welcome to BOODSKAP Light Controller");

  StaticJsonBuffer<128> buffer;
  JsonObject &data = buffer.createObject();

  data["light"] = light;
  data["cstatus"] = status;

  Boodskap.sendMessage(MESSAGE_ID, data);

}

void handleData(byte* data) {
  //handle raw data from the platform
}

bool handleMessage(uint16_t messageId, JsonObject& header, JsonObject& data)  // Handle Commands which is send by platform
{
  //handle JSON data from the platform
  Serial.printf("Message ID: %d \n", messageId);
  if ((!data["light"]) && status) {
    light = false;
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(RELAY_PIN, LOW);
    Serial.println("Status change to Light OFF");
  } else if ((data["light"]) && !status) {
    light = true;
    digitalWrite(LED_PIN, LOW);
    digitalWrite(RELAY_PIN, HIGH);
    Serial.println("Status change to Light ON");
  } else {
    Serial.println("No Status change");
  }
 
  return true; //return true if you have successfully handled the message
}
