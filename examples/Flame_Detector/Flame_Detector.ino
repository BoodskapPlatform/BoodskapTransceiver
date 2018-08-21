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
#define SENSE_INTERTVAL 100
#define REPORT_INTERVAL 15000
#define MESSAGE_ID 103 //Message defined in the platform

// define the flame sensor interface

//#define DIGITAL_PIN D1
#define BUZZER_PIN D2
#define ANALOG_PIN A0
#define LED_PIN D4
#define THRESHOLD_VAL 990

/*
  * ***** PLEASE CHANGE THE BELOW SETTINGS MATCHING YOUR ENVIRONMENT *****
*/

#define DEF_WIFI_SSID "*****"  //Your WiFi SSID
#define DEF_WIFI_PSK "*****" //Your WiFi password
#define DEF_DOMAIN_KEY "*****" //your DOMAIN Key
#define DEF_API_KEY "*****" //Your API Key
#define DEF_DEVICE_MODEL "BSKP_FLAME_DETECTOR" //Your device model
#define DEF_FIRMWARE_VER "1.0.0" //Your firmware version

BoodskapTransceiver Boodskap(UDP); //MQTT, UDP, HTTP

bool current_state = LOW;
bool previous_state = LOW;

uint32_t lastReport = 0;
uint16_t SENSOR_VAL;
bool fire;

void sendReading();
void handleData(byte* data);
bool handleMessage(uint16_t messageId, JsonObject& header, JsonObject& data);

void setup() {

  Serial.begin(115200);
  pinMode (LED_PIN, OUTPUT); // define LED as output interface
  pinMode(BUZZER_PIN, OUTPUT); // define buzzer module as output interface
  //  pinMode (digitalPin, INPUT); // output interface defines flame sensor
  pinMode (ANALOG_PIN, INPUT); // output interface defines the flame sensor

  StaticJsonBuffer<CONFIG_SIZE> buffer;
  JsonObject &config = buffer.createObject();

  config["ssid"] = DEF_WIFI_SSID;
  config["psk"] = DEF_WIFI_PSK;
  config["domain_key"] = DEF_DOMAIN_KEY;
  config["api_key"] = DEF_API_KEY;
  config["dev_model"] = DEF_DEVICE_MODEL;
  config["fw_ver"] = DEF_FIRMWARE_VER;
  config["dev_id"] = String("ESP8266-") + String(ESP.getChipId()); //Your unique device ID

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

  if ((millis() - lastReport) >= REPORT_INTERVAL) {
    sendReading();
    lastReport = millis();
  }

  SENSOR_VAL = analogRead(ANALOG_PIN); // Read analog PIN
  Serial.print("Analog Value:");
  Serial.println(SENSOR_VAL);

  if (SENSOR_VAL <= THRESHOLD_VAL) // When the flame sensor detects a signal,LED flashes & buzzer activated
  {
    Serial.println("Flame Detected");
    digitalWrite (LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
    fire = true;
    current_state = HIGH;
  }

  else if (SENSOR_VAL > THRESHOLD_VAL) { // When the flame sensor not detecting a signal,LED turned off & buzzer deactivated

    digitalWrite (LED_PIN, HIGH);
    digitalWrite (BUZZER_PIN, HIGH);
    fire = false;
    current_state = LOW;
  }

  if (current_state != previous_state) {  // message will be sent to the platform only when state change occurs
    sendReading();
  }

  previous_state = current_state;        //Reset both states
  delay(300);
}

void sendReading() {                    // Send Messages to the platform

  StaticJsonBuffer<128> buffer;
  JsonObject &data = buffer.createObject();
  data["fire"] = fire;
  data["value"] = SENSOR_VAL;
  Boodskap.sendMessage(MESSAGE_ID, data);
}

void handleData(byte* data) {
  //handle raw data from the platform
}

bool handleMessage(uint16_t messageId, JsonObject& header, JsonObject& data)
{
  //handle JSON data from the platform
  Serial.print(messageId);
  if (messageId == "101") {
    Serial.println("Alart Passed...");
  }

  return true; // return true if you have successfully handled the message
}
