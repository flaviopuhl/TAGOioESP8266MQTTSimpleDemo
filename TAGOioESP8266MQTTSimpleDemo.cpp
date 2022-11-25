/*
  Simple MQTT communication to TAGO.io

  Sends basic ESP8266 data and dummy variables to a Tago.io MQTT broker.
  See instructions manual for the code implementation details, as well as
  Tago.io dashboard & device & bucket configuration tricks.

  created 2022
  by Flavio Puhl <flavio_puhl@hotmail.com>
  
  This example code is in the public domain.

*/


/*+--------------------------------------------------------------------------------------+
 *| Libraries                                                                            |
 *+--------------------------------------------------------------------------------------+ */

#include <Arduino.h>

#include <ESP8266WiFi.h>

#include <PubSubClient.h>                // MQTT
#include <ArduinoJson.h>                 // MQTT

/*+--------------------------------------------------------------------------------------+
 *| Constants declaration                                                                |
 *+--------------------------------------------------------------------------------------+ */
 
 // Insert here the wifi network credentials
const char *ssid                              = "xxxxxxxxxxxxxxx";                       // name of your WiFi network
const char *password                          = "xxxxxxxx";                              // password of the WiFi network

const char *ID                                = "ThisIsMyDeviceID";                      // Name of our device, must be unique
const char* BROKER_MQTT                       = "mqtt.tago.io";                          // MQTT Cloud Broker URL
unsigned int PORT                             = 8883;
const char *TOKEN                             ="xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";

const char *USER                              = "MQTTTuser";


// Insert here topics that the device will publish to broker
const char *TopicsToPublish[]                 = { 
                                                "data",
                                                "info"
                                              };

String DeviceName                             = "TAGOioESP8266MQTT";
String FirmWareVersion                        = "TAGOioESP8266MQTT_001";

unsigned long previousMillis = 0; 


/*+--------------------------------------------------------------------------------------+
 *| Objects                                                                              |
 *+--------------------------------------------------------------------------------------+ */


BearSSL::WiFiClientSecure wClient;
PubSubClient MQTTclient(wClient);                           // Setup MQTT client



/*+--------------------------------------------------------------------------------------+
 *| Connect to WiFi network                                                              |
 *+--------------------------------------------------------------------------------------+ */

void setup_wifi() {
  Serial.print("\nConnecting to ");
  Serial.println(ssid);
    WiFi.mode(WIFI_STA);                              // Setup ESP in client mode
    //WiFi.setSleepMode(WIFI_NONE_SLEEP);
    WiFi.begin(ssid, password);                       // Connect to network

    int wait_passes = 0;
    while (WiFi.status() != WL_CONNECTED) {           // Wait for connection
      delay(500);
      Serial.print(".");
      if (++wait_passes >= 20) { ESP.restart(); }     // Restart in case of no wifi connection   
    }

  Serial.print("\nWiFi connected");
  Serial.print("\nIP address: ");
    Serial.println(WiFi.localIP());

  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);

}

/*+--------------------------------------------------------------------------------------+
 *| Verify and Manage WiFi network                                                       |
 *+--------------------------------------------------------------------------------------+ */

void VerifyWifi() {

  if (WiFi.status() != WL_CONNECTED || WiFi.localIP() == IPAddress(0, 0, 0, 0)){          // Check for network health
      
    Serial.printf("error: WiFi not connected, reconnecting \n");
            
      WiFi.disconnect();
      setup_wifi();             

  } 

}


/*+--------------------------------------------------------------------------------------+
 *| Reconnect to MQTT client                                                             |
 *+--------------------------------------------------------------------------------------+ */
 
void MQTTconnect() {

  if(!MQTTclient.connected()) {                               // Check if MQTT client is connected
  
  Serial.println();
  Serial.println("MQTT Client   : [ not connected ]");

  MQTTclient.setServer(BROKER_MQTT, PORT);                    // MQTT broker info
  MQTTclient.setBufferSize(1024);
                      
    
    Serial.println("MQTT Client   : [ trying connection ]");
    
    if (MQTTclient.connect(ID,USER,TOKEN)) {
      Serial.println("MQTT Client   : [ broker connected ]");

      for(int i=0; i<=((sizeof(TopicsToPublish) / sizeof(TopicsToPublish[0]))-1); i++){

        Serial.print("MQTT Client   : [ publishing to ");
        Serial.print(TopicsToPublish[i]);
        Serial.println(" ]");
        
      }
    } else {
      Serial.print("MQTT Client   : [ failed, rc= ");
      Serial.print(MQTTclient.state());
      Serial.println(" ]");

      delay(5000);
      setup_wifi();
    }
  }
}

/*+--------------------------------------------------------------------------------------+
 *| Serialize JSON and publish MQTT                                                      |
 *+--------------------------------------------------------------------------------------+ */

void SerializeAndPublish() {

  if (!MQTTclient.connected())                            // Reconnect if connection to MQTT is lost 
  {    MQTTconnect();      }

  char buffer[1024];                                      // JSON serialization 
   
  StaticJsonDocument<512> doc;

    JsonObject doc_0 = doc.createNestedObject();
    doc_0["variable"] = "DeviceName";
    doc_0["value"] = DeviceName;
    doc_0["unit"] = "";

    JsonObject doc_1 = doc.createNestedObject();
    doc_1["variable"] = "FirmWareVersion";
    doc_1["value"] = FirmWareVersion;
    doc_1["unit"] = "";

    JsonObject doc_2 = doc.createNestedObject();
    doc_2["variable"] = "WiFiRSSI";
    doc_2["value"] = WiFi.RSSI();
    doc_2["unit"] = "dB";

    JsonObject doc_3 = doc.createNestedObject();
    doc_3["variable"] = "IP";
    doc_3["value"] = WiFi.localIP();
    doc_3["unit"] = "";

    JsonObject doc_4 = doc.createNestedObject();
    doc_4["variable"] = "temperature";
    doc_4["value"] = random(300);
    doc_4["unit"] = "C";

    JsonObject doc_5 = doc.createNestedObject();
    doc_5["variable"] = "pressure";
    doc_5["value"] = random(3000);
    doc_5["unit"] = "Bar";

    serializeJson(doc, buffer);

    Serial.printf("\nJSON Payload:");
      Serial.printf("\n");
    
    serializeJsonPretty(doc, Serial);                 // Print JSON payload on Serial port        
      Serial.printf("\n");
      Serial.println("MQTT Client   : [ Sending message to MQTT topic ]"); 
      Serial.println("");         

    MQTTclient.publish(TopicsToPublish[0], buffer);    // Publish data to MQTT Broker 
       
}


/*+--------------------------------------------------------------------------------------+
 *| Setup                                                                                |
 *+--------------------------------------------------------------------------------------+ */
 
void setup() {

  Serial.begin(115200);                                                                     // Start serial communication at 115200 baud
  delay(1000);

  Serial.println();
  Serial.println(FirmWareVersion);
  Serial.println();

  setup_wifi();         // Start wifi

  wClient.setInsecure();

  MQTTconnect();        // Connect to MQTT Broker

  Serial.println("");
  Serial.println("Setup         : [ finished ]");
  Serial.println("");
  
}


/*+--------------------------------------------------------------------------------------+
 *| main loop                                                                            |
 *+--------------------------------------------------------------------------------------+ */
 
void loop() {

  MQTTclient.loop();        // Needs to be in the loop to keep client connection alive

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= 10*1000) {
    
    previousMillis = currentMillis;

    SerializeAndPublish();

  }

}
