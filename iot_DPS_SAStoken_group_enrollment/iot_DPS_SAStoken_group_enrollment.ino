/*
   Azure IoT Device Provisioning Service registration tool
   This sketch securely connects to an Azure IoT DPS using MQTT over WiFi,
   secured by SSl.
   It uses a SAS token for authetication.

   It subscribes to a DPS topic to receive the response, and publishes a message
   to start the device enrollment, and another message to request the Hub info if
   the previous request was successful.

   Boards:
   - Arduino Nano 33 IoT

   Author: Nicola Elia
   GNU General Public License v3.0
*/

#include <ArduinoMqttClient.h>
#include <WiFiNINA.h>
#include <ArduinoJson.h>
#include "secrets.h"

// ================================== SETTINGS ===================================
String sasToken       = SECRET_SAS_TOKEN;
String registrationId = SECRET_REGISTRATION_ID;
String idScope        = SECRET_ID_SCOPE;
// ===============================================================================

const char ssid[]     = SECRET_SSID;
const char pass[]     = SECRET_PASS;
const char broker[]   = SECRET_BROKER;

WiFiClient        wifiClient;
WiFiSSLClient     wifiClientSSL;
MqttClient        mqttClient(wifiClientSSL);

unsigned long lastMillis = 0;
String requestId = (String)random(1, 1000);

void setup() {
  // Wait for serial
  Serial.begin(9600);
  while (!Serial);

  // ================ MQTT Client SETUP ================
  // Set the client id used for MQTT as the registrationId
  mqttClient.setId(registrationId);

  // Set the username to "<idScope>/registrations/<registrationId>/api-version=2019-03-31"
  String username = idScope + "/registrations/" + registrationId + "/api-version=2019-03-31";

  // Set the SAS token as password
  String password = sasToken;

  // Authenticate the MQTT Client
  mqttClient.setUsernamePassword(username, password);

  // Set the on message callback, called when the MQTT Client receives a message
  mqttClient.onMessage(onMessageReceived);
}

void loop() {
  // ================ LOOP FUNCTION ================
  // Select the MQTT topic to subscribe to. It is a default value for DPS.
  String sub_topic = "$dps/registrations/res/#";

  // Connect to WiFi
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  // Establish MQTT connection
  if (!mqttClient.connected()) {
    connectMQTT(sub_topic);
  }

  // Select the MQTT topic to publish to. It is a default value for DPS.
  String pub_topic = "$dps/registrations/PUT/iotdps-register/?$rid=" + requestId;

  // Select the MQTT payload to be published. It must be JSON formatted.
  // Example:
  //  {
  //    "registrationId": "mydevice",
  //    "tpm":
  //      {
  //        "endorsementKey": "stuff",
  //        "storageRootKey": "things"
  //      },
  //    "payload": "your additional data goes here. It can be nested JSON."
  //  }
  // (This is a rudimental JSON, you can use ArduinoJSON)
  String pub_msg = "{\"registrationId\":\"}" + registrationId + "\"}";

  // Publish the message for requesting the device registration
  publishMessage(pub_topic, pub_msg);

  // Hang on after the device registration
  hangHere();
}

unsigned long getTime() {
  // Get the current time from the WiFi module
  return WiFi.getTime();
}

void connectWiFi() {
  Serial.print("Attempting to connect to SSID: ");
  Serial.print(ssid);
  Serial.print(" ");

  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    // failed, retry
    Serial.print(".");
    delay(5000);
  }
  Serial.println();

  Serial.println("You're connected to the network");
  Serial.println();
}

void connectMQTT(String topic) {
  Serial.print("Attempting to connect to MQTT broker: ");
  Serial.print(broker);
  Serial.println(" ");

  while (!mqttClient.connect(broker, 8883)) {
    delay(5000);
    // Failed, retry
    Serial.print("connectError: ");
    Serial.println(mqttClient.connectError());
  }
  Serial.println();

  Serial.println("You're connected to the MQTT broker");
  Serial.println();

  // Subscribe to the given topic
  mqttClient.subscribe(topic);
}

void publishMessage(String topic, String payload) {
  Serial.println("Publishing message");
  // Use the Print interface to send the message contents
  mqttClient.beginMessage(topic);
  mqttClient.print(payload);
  mqttClient.endMessage();
}

void onMessageReceived(int messageSize) {
  // Message received, print the topic and the message
  Serial.print("Received a message with topic '");
  Serial.print(mqttClient.messageTopic());
  Serial.println("'");

  // Save the message in a string object
  String msg_str = mqttClient.readString();

  // Create a JSON object to store the msg
  // 4096 is an exaggerated value, just to be sure that the whole message is parsed
  StaticJsonDocument<4096> msg_json;
  deserializeJson(msg_json, msg_str);

  // Print the message in prettyJson on the Serial output
  serializeJsonPretty(msg_json, Serial);

  // If the DPS is assigning the device, wait 3secs for the assignment and then
  // ask for the details (i.e. ask for the assigned hub properties)
  // This delay is contained in the name of the topic to which the DPS publishes
  // its message; it would be better if you pick the delay value from it!
  if (msg_json.containsKey("operationId") &&
      msg_json.containsKey("status"))
  {
    if (msg_json["status"] == "assigning") {
      delay(3000);
      String operationId = msg_json["operationId"];
      // Set the GET topic, to aks for assigned Hub info
      String pubTopic = "";
      pubTopic = "$dps/registrations/GET/iotdps-get-operationstatus/";
      pubTopic += "?$rid=" + requestId;
      pubTopic += "&operationId=" + operationId;

      // Again, you could jsonize this message with ArduinoJson lib
      String pub_msg = "{\"registrationId\":\"}" + registrationId + "\"}";

      // Send the message to ask for assigned Hub info
      publishMessage(pubTopic, pub_msg);
    }
  }
}

void hangHere() {
  while (true) {
    // Poll for new MQTT messages and send keep alives
    mqttClient.poll();
  }
}
