# Arduino Nano 33 IoT Azure DPS with SAS tokens
 Sketch to connect an Arduino Nano 33 IoT to the Azure IoT Device Provisioning Service over WiFi with an SSL-secured connection, and authentication based on SAS token. This sketch uses direct MQTT connection to azure services.

## Just a few hints
- You need to distinguish between Enrollment Groups and Individual Enrollment: the former implies to use the primary or secondary key of the enrollment group to hash each device's derived device key (which will be encoded as a SAS token), while the latter implies that the enrollment key coincides with the device key, so it's sufficient to encode the key into a SAS token.
- The payload of the mqtt messages is described [here](https://docs.microsoft.com/en-us/azure/iot-dps/how-to-send-additional-data#device-sends-data-payload-to-dps)
- Remember to add the Baltimore CA-root certificate to your WiFiNINA module, otherwise your Arduino won't be able to properly identify the Azure servers. [Here](https://support.arduino.cc/hc/en-us/articles/360016119219-How-to-add-certificates-to-Wifi-Nina-Wifi-101-Modules-) is the official tutorial for that.
- You can find the sketch to authorize the same device with X509 in my repos.
- To generate SAS tokens, you can use my python script in the /SAS_token_generator folder.