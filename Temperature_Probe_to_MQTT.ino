#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include <OneWire.h> 
#include <DallasTemperature.h>

#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#include "config.h"

int ledPin = LED_BUILTIN;       // choose the pin for the LED
int val = 0;                    // variable for reading the pin status

#ifdef DATA_INPUT_PIN
  int inputPin = DATA_INPUT_PIN;  // choose the input pin (for temperature probe)
#endif
#ifdef VOLTAGE_INPUT_PIN
  int voltagePin = VOLTAGE_INPUT_PIN;  // choose the input pin (for battery voltage)
#endif

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, MQTT_SERVERPORT, MQTT_USERNAME, MQTT_KEY);

// Setup MQTT feeds for publishing.
#ifdef VOLTAGE_INPUT_PIN
  Adafruit_MQTT_Publish mqtt_client_voltage = Adafruit_MQTT_Publish(&mqtt, MQTT_CHANNEL_VOLTAGE);
#endif

#ifdef DATA_INPUT_PIN
  Adafruit_MQTT_Publish mqtt_client_data = Adafruit_MQTT_Publish(&mqtt, MQTT_CHANNEL_DATA);
// Setup a oneWire instance to communicate with any OneWire devices  
  // (not just Maxim/Dallas temperature ICs) 
  OneWire oneWire(inputPin); 
  /********************************************************************/
  // Pass our oneWire reference to Dallas Temperature. 
  DallasTemperature sensors(&oneWire);
  /********************************************************************/ 
#endif

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         Serial.println("Couldn't connect, resetting.");
         ESP.restart();
       }
  }
  Serial.println("MQTT Connected!");
}

/*
void setupOTA() {
  ArduinoOTA.setHostname(OTA_HOSTNAME);
  ArduinoOTA.setPort(OTA_PORT);
  ArduinoOTA.setHostname(OTA_HOSTNAME);
  ArduinoOTA.setPassword(OTA_PASSWD);

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
}
*/

void setup() {
  pinMode(ledPin, OUTPUT);      // declare LED as output

  #ifdef DATA_INPUT_PIN
    pinMode(inputPin, INPUT);     // declare sensor as input
  #endif
  #ifdef VOLTAGE_INPUT_PIN
    pinMode(voltagePin, INPUT);   // declare battery voltage as input
  #endif

  Serial.begin(115200);
  Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWD);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  MQTT_connect();

  #ifdef VOLTAGE_INPUT_PIN
    //Read battery voltage
    float voltage = ( analogRead(VOLTAGE_INPUT_PIN) * (VOLTAGE_SLOPE) + (VOLTAGE_INTERCEPT) );
    Serial.print("Voltage read: ");
    Serial.println(voltage);
    if (!mqtt_client_voltage.publish(voltage)) {
      Serial.println(F("Failed publishing MQTT message"));
    } else {
      Serial.println(F("Success publishing MQTT message!"));
    }
  #endif
  delay(500);
  #ifdef DATA_INPUT_PIN
    sensors.begin(); 
    sensors.requestTemperatures(); // Send the command to get temperature readings
    float tempC = sensors.getTempCByIndex(0);
    Serial.print("Temperature read: ");
    Serial.println(tempC);
  
    if (!mqtt_client_data.publish(tempC)) {
      Serial.println(F("Failed publishing MQTT message"));
    } else {
      Serial.println(F("Success publishing MQTT message!"));
    }
  #endif

/*
  setupOTA();
  uint8_t timeout_OTA = 600;
  while (timeout_OTA > 0) {
     //Serial.println(F("Checking for OTA..."));
     ArduinoOTA.handle();
     delay(100);
     timeout_OTA--;
  }
*/
  Serial.println(F("Going to sleep"));

  delay(1000);
  ESP.deepSleep(SLEEP_TIME * 60 * 10e5);//10e5 = 1 second

  //ESP.deepSleep(60 * 10e5);//10e5 = 1 second
  //delay(SLEEP_TIME * 10E3);
  //ESP.restart();
}

void loop() {
   //UNUSED
}
